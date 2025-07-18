#pragma once
#include "MainHeader.hpp"
#include "Overcharge.hpp"
#include "OverchargeConfig.hpp"
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
#include "NiPSysModifier.hpp"
#include "NiParticleInfo.hpp"
#include <NiPSysAgeDeathModifier.hpp>
#include <NiTransformInterpolator.hpp>
#include <NiPSysBoxEmitter.hpp>
#include "Animation.hpp"
#include "ExtraDataList.hpp"
#include "TESAmmoEffect.hpp"
#include "BSAnimGroupSequence.hpp"
#include "BSShaderPPLightingProperty.hpp"
#include "TESEffectShader.hpp"
#include "AnimationStuff.hpp"
#include <ParticleShaderProperty.hpp>
#include "MagicShaderHitEffect.hpp"
#include "MagicCaster.hpp"
#include "MagicTarget.hpp"
#include "NonActorMagicCaster.hpp"
#include "ExtraAshPileRef.hpp"
#include <TESObjectACTI.hpp>
#include <BGSPrimitive.hpp>
//#include <tracy/Tracy.hpp>

namespace Overcharge
{
	std::unordered_map<UInt64, HeatConfiguration>					weaponDataMap;
	std::unordered_map<UInt64, std::shared_ptr<HeatData>>			activeWeapons;
	std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>		activeInstances;

	std::unordered_set<BSPSysSimpleColorModifier*> colorModifiers; 

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

			instance->state.fHeatVal -= frameTime * instance->state.fCooldownRate;
			if (instance->state.fHeatVal <= 0.0f)
			{
				it = activeWeapons.erase(it);
				continue;
			}

			instance->fx.currCol = instance->fx.SmoothShift(instance->state.fHeatVal);
			for (const auto& node : instance->fx.targetBlocks)
			{
				if (node && node.m_pObject)
				{
					SetEmissiveColor(node.m_pObject, instance->fx.currCol, instance->fx.matProp);
				}
			}
			++it;
		}
	}

	//check if you can edit weap values, call function then restore original values to save hooks.
	inline void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, Actor* rActor)
	{
		UInt32 sourceRef = rActor->uiFormID;
		UInt32 sourceWeap = rWeap->uiFormID;
		NiNodePtr sourceNode = rActor->Get3D();
		UInt64 key = MakeHashKey(sourceRef, sourceWeap);

		TESAmmo* equippedAmmo = rWeap->GetEquippedAmmo(rActor);
		UInt32 ammoForm = equippedAmmo->uiFormID;
		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end())
		{
			it->second->state.HeatOnFire();
			if (it->second->state.fHeatVal >= 100.0f)
			{
				//PlayAnimPath("meshes\\characters\\_1stperson\\fuck.kf", rActor);
			}
		}
		else
		{
			UInt64 dataKey = MakeHashKey(sourceWeap, ammoForm); 
			auto newIt = weaponDataMap.find(dataKey);
			if (newIt != weaponDataMap.end())
			{
				auto [insertIt, inserted] = activeWeapons.try_emplace(
					key, std::make_shared<HeatData>(MakeHeatFromConfig(newIt->second, sourceNode.m_pObject))
				);

				std::shared_ptr<HeatData> newHeatPtr = insertIt->second;
				newHeatPtr->state.HeatOnFire();
				newHeatPtr->fx.currCol = newHeatPtr->fx.SmoothShift(newHeatPtr->state.fHeatVal);

				newHeatPtr->fx.matProp = NiMaterialProperty::CreateObject();

				for (NiAVObjectPtr& obj : newHeatPtr->fx.targetBlocks)
				{
					SetEmissiveColor(obj, newHeatPtr->fx.currCol, newHeatPtr->fx.matProp);
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
			if (const auto& psys = projNode->NiDynamicCast<NiParticleSystem>())
			{
				activeInstances[psys] = it->second;
			}

			float heatRatio = it->second->state.fHeatVal / 100.0f;
			heatRatio = std::clamp(heatRatio, 0.0f, 1.0f);
			projNode->m_kLocal.m_fScale *= (0.8f + heatRatio);
			proj->fSpeedMult *= (1.2f - heatRatio);
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

	BSTempEffectParticle* __cdecl ImpactActorWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B0);
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

	MagicShaderHitEffect* __fastcall MSHEInit(MagicShaderHitEffect* thisPtr, void* edx, TESObjectREFR* target, TESEffectShader* a3, float duration)
	{
		Actor* targetActor = reinterpret_cast<Actor*>(target);
		if (!targetActor) return ThisStdCall<MagicShaderHitEffect*>(0x81F580, thisPtr, target, a3, duration);

		Actor* killer = targetActor->pKiller;
		UInt32 killerID = killer->uiFormID;
		UInt32 killerWeapID = killer->GetCurrentWeaponID();
		UInt64 key = MakeHashKey(killerID, killerWeapID);

		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end())
		{
			auto& fx = it->second->fx;
			UInt32 col = fx.RGBtoUInt32(fx.currCol);

			a3->Data.colorKey1RGB = col;
			a3->Data.colorKey2RGB = col;
			a3->Data.colorKey3RGB = col;
			a3->Data.uiEdgeColor = col;
			a3->Data.edgeColorRGB = col;
			a3->Data.uiFillColor1 = col;
		}
		return ThisStdCall<MagicShaderHitEffect*>(0x81F580, thisPtr, target, a3, duration);
	}

	TESObjectREFR* __fastcall CreateRefAtLocation(TESDataHandler* thisPtr, void* edx, TESBoundObject* pObject, NiPoint3* apLocation, NiPoint3* apDirection, TESObjectCELL* pInterior, TESWorldSpace* pWorld, TESObjectREFR* pReference, BGSPrimitive* pAddPrimitive, void* pAdditionalData)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Actor* targetActor = *reinterpret_cast<Actor**>(ebp + 0x20);
		TESObjectREFR* expl = ThisStdCall<TESObjectREFR*>(0x4698A0, thisPtr, pObject, apLocation, apDirection, pInterior, pWorld, pReference, pAddPrimitive, pAdditionalData);

		Actor* killer = targetActor->pKiller;
		UInt32 killerID = killer->uiFormID;
		UInt32 killerWeapID = killer->GetCurrentWeaponID();
		UInt64 key = MakeHashKey(killer->uiFormID, killerWeapID);

		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end())
		{
			NiNodePtr node = pObject->LoadGraphics(expl);
			TraverseNiNode(node, it->second->fx.currCol);
		}
		return expl;
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

	void __fastcall SetAttackSpeedHook(Animation* thisPtr, void* edx, float animSpeed) 
	{
		UInt32 sourceRef = thisPtr->pActor->uiFormID;
		UInt32 sourceWeap = thisPtr->pActor->GetCurrentWeaponID();
		UInt64 key = MakeHashKey(sourceRef, sourceWeap);

		auto it = activeWeapons.find(key);
		if (it != activeWeapons.end())
		{
			float& currInfo = it->second->state.fHeatVal;

			float heatRatio = currInfo / 100.0f;

			heatRatio = std::clamp(heatRatio, 0.0f, 1.0f);

			animSpeed *= (1.0f + heatRatio);
		}

		ThisStdCall(0x4C0C90, thisPtr, animSpeed);
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;			//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;			//0x447168 Checks loaded NIF file
		UInt32 createProjectile = 0x9BD518;  
		UInt32 spawnImpact = 0x9C2058;			//Projectile::SpawnCollisionEffects @ Projectile::ProcessImpacts 
		UInt32 spawnImpactEffects = 0x9C2AC3;  
		UInt32 spawnActorImpactEffects = 0x88F245;
		UInt32 muzzleFlashEnable = 0x9BB7CD; 
		UInt32 initParticle = 0xC2237A;
		UInt32 colorParticle = 0xC220E5;
		UInt32 GetAttackSpeedMult = 0x645D73;
		UInt32 GetWeaponAnimMult = 0x9CBE83;
		UInt32 SetAttackSpeed = 0x894337;
		UInt32 MPSAddon = 0x9BE07A;
		UInt32 initialPosVelocity = 0xC31CF0;
		UInt32 updateSCM = 0xC602E0;
		UInt32 clearUnrefEmitters = 0xC5E164;
		UInt32 colorModUpdate = 0xC602E0;
		UInt32 impactBodyPart = 0x8B5D9B;
		UInt32 DismemberBodyPart = 0x8B5135; 
		UInt32 DisintegrateEffectShader = 0x8214E8;
		UInt32 StartParticleShader = 0x82021B;
		UInt32 SetupParticleShader = 0x5077ED;
		UInt32 MagicShaderHitEffectApply = 0x81FC70;
		UInt32 MagicShaderAddTempEffect = 0x5D1CD1;
		UInt32 InitMagicShaderCmd = 0x5D1C90;
		UInt32 AshPilePersists = 0x5DBD8E;
		UInt32 GetAshXData = 0x5DBD9D;
		UInt32 SetAshXData = 0x5DBDBA;
		UInt32 CreateRefAtLoc = 0x5DBD56;

		// Hooks

		WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		WriteRelCall(spawnActorImpactEffects, &ImpactActorWrapper);
		WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
		WriteRelJump(initialPosVelocity, &VertexColorModifier);
		WriteRelJump(clearUnrefEmitters, &ClearUnrefEmittersHook);
		WriteRelJump(colorModUpdate, &ColorModifierUpdate);
		WriteRelCall(SetAttackSpeed, &SetAttackSpeedHook);
		WriteRelCall(InitMagicShaderCmd, &MSHEInit);
		WriteRelCall(CreateRefAtLoc, &CreateRefAtLocation);
	}
}