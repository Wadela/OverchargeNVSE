#pragma once
#include "MainHeader.hpp"
#include "Overcharge.hpp"

#include "NiRTTI.hpp"
#include "NiObject.hpp"
#include "NiParticleSystem.hpp"
#include "NiAVObject.hpp"
#include <BSTempEffectParticle.hpp>
#include <BSValueNode.hpp>
#include <BSParticleSystemManager.h>

#include <filesystem>
#include <numbers>
#include <print>
#include <set>
#include <unordered_map>

#include "BGSAddonNode.hpp"
#include "BSMasterParticleSystem.hpp"
#include "BSPSysMultiTargetEmitterCtlr.hpp"
#include "BSStream.hpp"
#include "Defines.h"
#include "NiCloningProcess.hpp"
#include "NiGeometryData.hpp"
#include "NiNode.hpp"
#include "SafeWrite.hpp"
#include "TESMain.hpp"
#include "NiGeometry.hpp"
#include "NiStream.hpp"
#include "NiLight.hpp"
#include "NiPointLight.hpp"
#include "TESDataHandler.hpp"
#include <BSPSysSimpleColorModifier.hpp>
#include "NiUpdateData.hpp"
#include "TimeGlobal.hpp"
#include <TESObjectWEAP.hpp>
#include "PluginAPI.hpp"
#include "MuzzleFlash.hpp"
#include <NiBillboardNode.hpp>
#include "BGSImpactData.hpp" 
#include "TESModel.hpp"
#include <NifOverride.hpp>
#include "WeaponDataMap.hpp"
#include "NiPSysModifier.hpp"
#include "NiParticleInfo.hpp"
#include <NiPSysAgeDeathModifier.hpp>
#include <NiTransformInterpolator.hpp>
#include <NiPSysBoxEmitter.hpp>
//#include <tracy/Tracy.hpp>

namespace Overcharge
{
	std::unordered_map<UInt32, HeatData>							weaponDataMap;
	std::unordered_map<UInt64, std::shared_ptr<HeatData>>			activeWeapons;
	std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>		activeInstances;

	std::unordered_set<BSPSysSimpleColorModifier*> colorModifiers; 

	inline UInt64 MakeHashKey(UInt32 actorID, UInt32 weaponID) 
	{
		return ((UInt64)actorID << 32) | weaponID;
	}

	static void SetEmissiveColor(NiAVObjectPtr obj, const NiColor& color)
	{
		NiGeometryPtr geom = obj ? obj->NiDynamicCast<NiGeometry>() : nullptr;

		if (!geom) return;
		NiMaterialPropertyPtr matProp = geom->m_kProperties.m_spMaterialProperty;

		if (!matProp) return;
		matProp->m_emit = color;
	}

	static void CreateEmissiveColor(NiAVObjectPtr obj, const NiColor& color)
	{
		NiGeometryPtr geom = obj ? obj->NiDynamicCast<NiGeometry>() : nullptr;

		if (!geom) return;

		NiMaterialPropertyPtr newMat = NiMaterialProperty::CreateObject();
		newMat->m_emit = color;
		geom->m_kProperties.m_spMaterialProperty = newMat;
	}

	static void TraverseNiNode(const NiNodePtr obj, NiColor& color)
	{
		for (int i = 0; i < obj->m_kChildren.m_usSize; i++)
		{
			NiAVObject* const child = obj->m_kChildren[i].m_pObject;
			if (child)
			{
				//Checks if RTTI comparison is valid before static casting to avoid dynamic casting every single time
				if (child->IsNiType<NiParticleSystem>())
				{
					NiParticleSystemPtr childPsys = static_cast<NiParticleSystem*>(child);
					SetEmissiveColor(childPsys.m_pObject, color); 
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNodePtr childNode = static_cast<NiNode*>(child);
					TraverseNiNode(childNode.m_pObject, color);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometryPtr childGeom = static_cast<NiGeometry*>(child);
					SetEmissiveColor(childGeom.m_pObject, color);
				}
			}
		}
	}

	// Collect all BSValue nodes (game will 'attach' emitters to these)
	void FindValueNodes(const NiNode* apNode, std::vector<BSValueNode*>& result) {
		for (int i = 0; i < apNode->m_kChildren.m_usSize; i++) {
			const auto& child = apNode->m_kChildren.m_pBase[i];

			if (const auto pValueNode = child->NiDynamicCast<BSValueNode>()) {
				result.emplace_back(pValueNode);
			}
			else if (const auto pNiNode = child->NiDynamicCast<NiNode>()) {
				FindValueNodes(pNiNode, result);
			}
		}
	}

	std::vector<BSValueNode*> FindValueNodes(const NiNode* apNode) {
		std::vector<BSValueNode*> valueNodes{};
		FindValueNodes(apNode, valueNodes);
		return valueNodes;
	}

	void WeaponCooldown()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;
		for (auto it = activeWeapons.begin(); it != activeWeapons.end();)
		{
			const auto& instance = it->second;
			if (!instance) 
			{
				it = activeWeapons.erase(it);
				continue;
			}

			instance->heat.heatVal -= frameTime * instance->heat.cooldownRate;
			if (instance->heat.heatVal <= instance->heat.baseHeatVal) 
			{
				it = activeWeapons.erase(it);
				continue;
			}

			instance->fx.currCol = instance->fx.SmoothShift(instance->heat.heatVal, instance->heat.maxHeat);
			for (const auto& node : instance->targetBlocks)
			{
				if (node && node.m_pObject)
				{
					SetEmissiveColor(node.m_pObject, instance->fx.currCol);
				}
			}
			++it;
		}
	}
	inline void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		UInt32 sourceRef = rActor->uiFormID;
		UInt32 sourceWeap = rWeap->uiFormID;
		NiNodePtr sourceNode = rActor->Get3D();

		UInt64 key = MakeHashKey(sourceRef, sourceWeap);

		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end())
		{
			it->second->heat.HeatOnFire();
		}
		else
		{
			auto newIt = weaponDataMap.find(sourceWeap);
			if (newIt != weaponDataMap.end())
			{
				auto [insertIt, inserted] = activeWeapons.try_emplace(
					key, std::make_shared<HeatData>(MakeHeatFromTemplate(newIt->second, sourceNode.m_pObject, sourceRef, sourceWeap))
				);

				std::shared_ptr<HeatData>& newHeatPtr = insertIt->second;
				newHeatPtr->heat.HeatOnFire();
				newHeatPtr->fx.currCol = newHeatPtr->fx.SmoothShift(newHeatPtr->heat.heatVal, newHeatPtr->heat.maxHeat);

				for (NiAVObjectPtr& obj : newHeatPtr->targetBlocks)
				{
					CreateEmissiveColor(obj, newHeatPtr->fx.currCol);
				}
			}
		}
		ThisStdCall(0x523150, rWeap, rActor);
	}

	inline void __fastcall MuzzleFlashEnable(MuzzleFlash* flash)
	{
		UInt64 key = MakeHashKey(flash->pSourceActor->uiFormID, flash->pSourceWeapon->uiFormID);
		NiNodePtr muzzleNode = flash->spNode;

		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end())
		{
			TraverseNiNode(muzzleNode.m_pObject, it->second->fx.currCol);
			flash->spLight->SetDiffuseColor(it->second->fx.currCol);
		}
		ThisStdCall(0x9BB690, flash);
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj)
	{
		UInt64 key = MakeHashKey(proj->pSourceRef->uiFormID, proj->pSourceWeapon->uiFormID);
		NiNodePtr projNode = proj->Get3D();

		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end())
		{
			for (const auto& valueNode : FindValueNodes(projNode)) 
			{
				activeInstances[valueNode] = it->second;
			}
			TraverseNiNode(projNode, it->second->fx.currCol);
		}
		ThisStdCall(0x9A52F0, a1, proj);
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B8);
		UInt64 key = MakeHashKey(pProjectile->pSourceRef->uiFormID, pProjectile->pSourceWeapon->uiFormID);

		BSTempEffectParticlePtr impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);
		if (!impact) return nullptr;

		NiNodePtr impactNode = impact->spParticleObject->IsNiNode();

		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end() || !impactNode)
		{
			for (const auto& valueNode : FindValueNodes(impactNode))
			{
				activeInstances[valueNode] = it->second;
			}
			TraverseNiNode(impactNode, it->second->fx.currCol);
		}
		return impact;
	}

	__declspec(naked) void __fastcall ComputeInitPosVelocity(NiPSysVolumeEmitter* thisPtr, void* edx, NiPoint3* arg0, NiPoint3* arg4)
	{
		__asm
		{
			sub		esp, 0xD0
			push	0xC31CF6
			retn
		}
	}

	void __fastcall VertexColorModifier(NiPSysVolumeEmitter* thisPtr, void* edx, NiPoint3* arg0, NiPoint3* arg4)
	{
		if (!thisPtr || !thisPtr->m_pkEmitterObj || !thisPtr->m_pkTarget)
		{
			ComputeInitPosVelocity(thisPtr, edx, arg0, arg4);
			return;
		}

		// Look up the heat data for this emitter object
		if (auto it = activeInstances.find(thisPtr->m_pkEmitterObj); it != activeInstances.end())
		{
			const auto& heatInfo = it->second;

			thisPtr->m_kInitialColor = heatInfo->fx.currCol;

			for (const auto& modifier : thisPtr->m_pkTarget->m_kModifierList)
			{
				if (const auto scm = modifier->NiDynamicCast<BSPSysSimpleColorModifier>())
				{
					colorModifiers.emplace(scm);
				}
			}
			if (auto matProp = thisPtr->m_pkTarget->GetMaterialProperty())
			{
				matProp->m_emit = NiColor(1, 1, 1);
			}
		}

		ComputeInitPosVelocity(thisPtr, edx, arg0, arg4);
	}

	__declspec(naked) void __fastcall OriginalColorModifierUpdate(BSPSysSimpleColorModifier* apThis, void* edx, float afTime, NiPSysData* apData)
	{
		__asm
		{
			push	ebp
			mov		ebp, esp
			sub		esp, 0x34
			push	0xC602E6
			retn
		}
	}

	inline void __fastcall ColorModifierUpdate(BSPSysSimpleColorModifier* apThis, void* edx, float afTime, NiPSysData* apData) 
	{
		if (!apData->m_pkColor || !colorModifiers.contains(apThis)) 
		{
			OriginalColorModifierUpdate(apThis, edx, afTime, apData);
			return;
		}
		for (int i = 0; i < apData->m_usActiveVertices; ++i) 
		{
			NiParticleInfo* pInfo = &apData->m_pkParticleInfo[i];
			float lifePercent = pInfo->m_fAge / pInfo->m_fLifeSpan;

			if (apThis->fFadeOut != 0.0f && lifePercent > apThis->fFadeOut) {
				float fadeOutPercent = (lifePercent - apThis->fFadeOut) / (1.0f - apThis->fFadeOut);
				apData->m_pkColor[i].a = (apThis->kColor3.a - apThis->kColor2.a) * fadeOutPercent + apThis->kColor2.a;
			}
			else if (apThis->fFadeIn == 0.0f || lifePercent >= apThis->fFadeIn) {
				apData->m_pkColor[i].a = apThis->kColor2.a;
			}
			else {
				float fadeInPercent = lifePercent / apThis->fFadeIn;
				apData->m_pkColor[i].a = (apThis->kColor2.a - apThis->kColor1.a) * fadeInPercent + apThis->kColor1.a;
			}
		}
	}
	void __stdcall ClearTrackedEmitters(NiAVObject* toDelete)
	{
		activeInstances.erase(toDelete);

		std::erase_if(colorModifiers, [](BSPSysSimpleColorModifier* mod) {
			return mod == nullptr;
			});
	}

	__declspec(naked) void ClearUnrefEmittersHook()
	{
		static const UInt32 returnAddr = 0xC5E169;

		__asm
		{
			mov        eax, [ebp - 0x20]
			push	   eax
			call	   ClearTrackedEmitters

			mov        ecx, [ebp - 0x20]
			mov        edx, [ecx]
			jmp        returnAddr
		}
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;			//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;			//0x447168 Checks loaded NIF file
		UInt32 createProjectile = 0x9BD518;  
		UInt32 spawnImpact = 0x9C2058;			//Projectile::SpawnCollisionEffects @ Projectile::ProcessImpacts 
		UInt32 spawnImpactEffects = 0x9C2AC3;  
		UInt32 muzzleFlashEnable = 0x9BB7CD; 
		UInt32 initParticle = 0xC2237A;
		UInt32 colorParticle = 0xC220E5;

		UInt32 MPSAddon = 0x9BE07A;
		UInt32 initialPosVelocity = 0xC31CF0;
		UInt32 updateSCM = 0xC602E0;
		UInt32 clearUnrefEmitters = 0xC5E164;
		UInt32 colorModUpdate = 0xC602E0;

		// Hooks

		WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
		WriteRelJump(initialPosVelocity, &VertexColorModifier);
		WriteRelJump(clearUnrefEmitters, &ClearUnrefEmittersHook);
		WriteRelJump(colorModUpdate, &ColorModifierUpdate);
	}

}