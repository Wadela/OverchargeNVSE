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
	NiMaterialPropertyPtr g_customPlayerMatProp = NiMaterialProperty::CreateObject();
	static const NiColorA col = (1, 0, 0, 1);
	std::vector<HeatInstance> activeWeapons;				//Vector containing all weapons that are currently heating up
	std::vector<Projectile*> activeProjectiles;
	std::unordered_map<UInt32, HeatInstance> weaponDataMap;
	int colorCount = 0;
	std::vector<ParticleInstance> newActiveParticles;

	struct ProjectileHeatValue {
		NiColorA kColor;
		UInt32 uiRefId;
	};

	std::unordered_map<NiAVObject*, std::shared_ptr<ProjectileHeatValue>> trackedNodes{};

	// Lookups for utility, weak_ptr makes it a little annoying though
	std::unordered_map<UInt32, std::weak_ptr<ProjectileHeatValue>> projectileHeat{};
	std::unordered_map<BSPSysSimpleColorModifier*, std::vector<std::weak_ptr<ProjectileHeatValue>>> colorModifiers{};

	std::shared_ptr<ProjectileHeatValue> curHeat{};

	// Collect all BSValue nodes (game will 'attach' emitters to these)
	void FindValueNodes(const NiNode* apNode, std::vector<BSValueNode*>& result) {
		for (int i = 0; i < apNode->m_kChildren.GetSize(); i++) {
			const auto child = apNode->m_kChildren.GetAt(i);

			if (const auto pValueNode = child->NiDynamicCast<BSValueNode>()) {
				result.push_back(pValueNode);
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

	char __cdecl MPSAddonWrapper(BSValueNode* apNode)
	{
		//This is where you're going to mark the addonNode for later use

		NiNode* projNode = apNode;

		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x8);

		const auto heatValue = std::make_shared<ProjectileHeatValue>(
			NiColorA(
				(rand() % 256) / 255.0,
				(rand() % 256) / 255.0,
				(rand() % 256) / 255.0,
				1.0
			),
			pProjectile->uiFormID
		);

		// Mark value nodes to track (for emission later)
		for (const auto& valueNode : FindValueNodes(projNode)) {
			trackedNodes[valueNode] = heatValue;
		}

		// Track initial heat for this projectile (later used for impact creation)
		projectileHeat[pProjectile->uiFormID] = heatValue;

		char result = CdeclCall<char>(0x578060, apNode);
		return result;
	}

	void VertexColorModifier(NiPSysVolumeEmitter* thisPtr, NiPoint3* arg0, NiPoint3* arg4)
	{
		if (trackedNodes.contains(thisPtr->m_pkEmitterObj)) {
			const auto& heatInfo = trackedNodes[thisPtr->m_pkEmitterObj];

			// Game uses this inside of NiPSysEmitter::EmitParticles (0xC220C0) to set the vertex color of the next spawned particle
			thisPtr->m_kInitialColor = heatInfo->kColor;

			// Track (+ hook) BSPSysSimpleColorModifier since it overwrites all vertex colors upon init
			for (const auto modifier : thisPtr->m_pkTarget->m_kModifierList) {
				if (const auto scm = modifier->NiDynamicCast<BSPSysSimpleColorModifier>()) {
					colorModifiers[scm].push_back(heatInfo);
				}
			}

			// Set emission to white for vertex colors to work
			//if (const auto matProp = thisPtr->m_pkTarget->GetMaterialProperty()) {
				//matProp->m_emit = NiColor(0.8f, 0.8f, 0.8f);
			//}
		}
	}

	__declspec(naked) void __stdcall InitialPosVelocityHook()
	{
		static const UInt32 returnAddr = 0xC31CF6;

		__asm 
		{
			sub		esp, 0xD0
			push	eax
			push	edx
			push	ebx

			mov		eax, ecx				// this (NiPSysVolumeEmitter*)
			push	ecx

			lea     ebx, [esp + 0xDC]		// arg0
			push    ebx
			lea     edx, [esp + 0xD8]		// arg4
			push    edx              
			push	eax
			call    VertexColorModifier

			add     esp, 0xC

			pop		ecx
			pop		ebx
			pop		edx
			pop		eax
			jmp		returnAddr
		}
	}

	void __stdcall ClearTrackedEmitters(NiObject* obj, void* edx)
	{
		// Stop tracking emitters when master particle system drops them
		if (NiAVObject* emitterObj = obj->NiDynamicCast<NiAVObject>())
		trackedNodes.erase(emitterObj);

		// Clean up weak pointers
		// Yes this is nasty
		{
			std::erase_if(projectileHeat, [](auto& elem) {
				const auto& [_, value] = elem;
				return value.expired();
				});

			std::erase_if(colorModifiers, [](auto& elem) {
				auto& [_, value] = elem;

				// Remove from inner vector
				std::erase_if(value, [](auto& item) {
					return item.expired();
					});

				// Remove from map if vector empty
				return value.empty();
				});
		}
	}

	__declspec(naked) void ClearUnrefEmittersHook()
	{
		static const UInt32 returnAddr = 0xC5E169;

		__asm
		{
			mov		eax, [ebp - 0x20]
			push	eax
			push	edx
			call	ClearTrackedEmitters

			mov		ecx, [ebp - 0x20]
			mov		edx, [ecx]
			jmp		returnAddr
		}
	}

	NiPoint3* GetRoundedPosition(NiPoint3* pos)
	{
		float roundX = std::round(pos->x);
		float roundY = std::round(pos->y);
		float roundZ = std::round(pos->z);

		return new NiPoint3(roundX, roundY, roundZ);
	}

	inline bool AreFloatsNearlyEqual(float a, float b, float epsilon = 1e-5f)
	{
		return fabsf(a - b) < epsilon;
	}

	inline bool AreNiPoint3NearlyEqual(const NiPoint3& a, const NiPoint3& b, float epsilon = 1e-5f)
	{
		return AreFloatsNearlyEqual(a.x, b.x, epsilon) &&
			AreFloatsNearlyEqual(a.y, b.y, epsilon) &&
			AreFloatsNearlyEqual(a.z, b.z, epsilon);
	}

	inline bool AreNiMatrix3NearlyEqual(const NiMatrix3& a, const NiMatrix3& b, float epsilon = 1e-5f)
	{
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				if (!AreFloatsNearlyEqual(a.m_pEntry[row][col], b.m_pEntry[row][col], epsilon))
					return false;
			}
		}
		return true;
	}

	inline bool AreNiTransformsNearlyEqual(const NiTransform& a, const NiTransform& b, float epsilon = 1e-5f)
	{
		return AreFloatsNearlyEqual(a.m_fScale, b.m_fScale, epsilon) &&
			AreNiPoint3NearlyEqual(a.m_Translate, b.m_Translate, epsilon) &&
			AreNiMatrix3NearlyEqual(a.m_Rotate, b.m_Rotate, epsilon);
	}

	static void SetEmissiveByName(NiAVObjectPtr obj, NiMaterialProperty* matProp, NiColor& blendedColor, const char* blockName = nullptr)
	{
		if (!obj) return;
		NiAVObjectPtr target = blockName ? obj->GetObjectByName(blockName) : obj.m_pObject;

		if (!target) return;
		if (NiGeometry* geom = target->NiDynamicCast<NiGeometry>())
		{
			if (!geom->m_kProperties.m_spMaterialProperty) return;
			geom->m_kProperties.m_spMaterialProperty = matProp;
			geom->m_kProperties.m_spMaterialProperty->m_emit = blendedColor;
		}
	}

	static void SetEmissiveColor(NiAVObjectPtr obj, const NiColor& blendedColor)
	{
		NiGeometryPtr geom = obj ? obj->NiDynamicCast<NiGeometry>() : nullptr;

		if (!geom) return;
		NiMaterialPropertyPtr matProp = geom->m_kProperties.m_spMaterialProperty;

		if (!matProp) return;
		matProp->m_emit = blendedColor;
	}

	static void SetEmissiveClone(NiAVObjectPtr obj, const NiColor& blendedColor)
	{
		NiGeometryPtr geom = obj ? obj->NiDynamicCast<NiGeometry>() : nullptr;

		if (!geom) return;
		NiMaterialPropertyPtr originalMat = geom->m_kProperties.m_spMaterialProperty;

		if (!originalMat) return;
		//NiMaterialPropertyPtr clonedMat = originalMat->Clone()->NiDynamicCast<NiMaterialProperty>();
		NiMaterialPropertyPtr clonedMat = NiMaterialProperty::CreateObject();

		if (!clonedMat) return;
		clonedMat->m_emit = blendedColor;
		geom->m_kProperties.m_spMaterialProperty = clonedMat;
	}

	//Update Child Particles to all be prepared for emissive color control
	static void ChangeParticleColor(const NiAVObject* obj, NiColor& blendedColor)
	{
		if (BSValueNode* const valueNode = obj->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->uiIndex)
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					for (int i = 0; i < mps->kChildParticles.m_usSize; i++)
					{
						SetEmissiveColor(mps->kChildParticles[i], blendedColor);
					}
				}
			}
		}
	}


	//NOTE TO SELF: YOU NEED TO MAKE THIS FUNCTION TRAVERSE THE MPS AND GET ALL CHILDPARTICLESYSTEMS OTHERWISE IT WILL ONLY GRAB THE FIRST ONE AS YOU SAW WITH THE IMPACT. YOU NEED TO MAKE IT LOOK SIMILAR TO THE FUNCTION IN NIFOVERRIDE.


	//Update Child Particles to all be prepared for emissive color control
	static void CloneParticle(const NiAVObject* obj, NiColor& blendedColor)
	{
		if (BSValueNode* const valueNode = obj->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->uiIndex)
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					NiNodePtr mpsNode = mps->GetAt(0)->NiDynamicCast<NiNode>();
					if (!mpsNode) return; 
					if (NiNode* newNode = mpsNode->Clone()->NiDynamicCast<NiNode>())
					{
						std::pair<NiNode*, NiNode*> activeNodes = { newNode, valueNode };

						for (int i = 0; i < newNode->m_kChildren.m_usSize; i++)
						{
							if (const auto& child2 = newNode->m_kChildren.m_pBase[i])
							{
								if (NiParticleSystem* psys = child2->NiDynamicCast<NiParticleSystem>())
								{
									if (auto bsp = psys->GetController<BSPSysMultiTargetEmitterCtlr>())
									{
										NiCloningProcess cloner{};
										if (NiPSysEmitterCtlrPtr newEmit = ThisStdCall<NiPSysEmitterCtlr*>(0xC1C570, bsp, &cloner))
										{
											newEmit->SetTarget(psys);
											psys->RemoveController(bsp);
											psys->m_fLastTime = 0;
										}
									}
									SetEmissiveClone(psys, blendedColor);
									newActiveParticles.emplace_back(ParticleInstance(psys, activeNodes));
									mps->FindRootNode()->AttachChild(newNode, false);
								}
							}
						}
						valueNode->SetAppCulled(true);
					}
				}
			}
		}
	}

	static void ProcessNiNode2(const NiNode* obj, NiColor& blendedColor)
	{
		for (int i = 0; i < obj->m_kChildren.m_usSize; i++)
		{
			NiAVObject* const child = obj->m_kChildren[i].m_pObject;
			if (child)
			{
				//Checks if RTTI comparison is valid before static casting to avoid dynamic casting every single time
				if (child->IsNiType<NiParticleSystem>())
				{
					NiParticleSystem* childPsys = static_cast<NiParticleSystem*>(child); 
					SetEmissiveColor(childPsys, blendedColor);
				}
				else if (child->IsNiType<BSValueNode>())
				{
					BSValueNode* childValNode = static_cast<BSValueNode*>(child);
					CloneParticle(childValNode, blendedColor);
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNode* childNode = static_cast<NiNode*>(child);
					ProcessNiNode2(childNode, blendedColor);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					SetEmissiveColor(childGeom, blendedColor); 
				} 
			}
		}
	}

	static void ProcessNiNodeTest(const NiNode* obj, NiColor& blendedColor)
	{
		for (int i = 0; i < obj->m_kChildren.m_usSize; i++)
		{
			NiAVObject* const child = obj->m_kChildren[i].m_pObject;
			if (child)
			{
				//Checks if RTTI comparison is valid before static casting to avoid dynamic casting every single time
				if (child->IsNiType<NiParticleSystem>())
				{
					NiParticleSystem* childPsys = static_cast<NiParticleSystem*>(child);
					SetEmissiveClone(childPsys, blendedColor);
				}
				else if (child->IsNiType<BSValueNode>())
				{
					BSValueNode* childValNode = static_cast<BSValueNode*>(child);
					ChangeParticleColor(childValNode, blendedColor);
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNode* childNode = static_cast<NiNode*>(child);
					ProcessNiNodeTest(childNode, blendedColor);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					SetEmissiveClone(childGeom, blendedColor);
				}
			}
		}
	}


	void WeaponCooldown()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;

		activeWeapons.erase(
			std::remove_if(activeWeapons.begin(), activeWeapons.end(),
				[frameTime](Overcharge::HeatInstance& instance)
				{
					instance.heat.heatVal -= frameTime * instance.heat.cooldownRate;
					if (instance.heat.heatVal <= instance.heat.baseHeatVal)
						return true;

					instance.fx.currCol = instance.fx.SmoothShift(instance.heat.heatVal, instance.heat.maxHeat);
					for (const auto& node : instance.targetBlocks)
					{
						if (node && node.m_pObject)
						{
							SetEmissiveColor(node.m_pObject, instance.fx.currCol);
						}
					}
					return false;
				}),
			activeWeapons.end());
	}

	void ParticleUpdater()
	{
		const float frameTime = TimeGlobal::GetSingleton()->fDelta;

		newActiveParticles.erase(
			std::remove_if(newActiveParticles.begin(), newActiveParticles.end(),
				[&](ParticleInstance& instance)
				{
					NiParticleSystem* psys = instance.particle;

					//psys->m_fLastTime += frameTime;

					if (!psys || !psys->m_spControllers) return true;

					const float hi = psys->m_spControllers->m_fHiKeyTime;
					const float lingerThreshold = hi + 4.0f ; 
					const bool inLingerWindow = psys->m_fLastTime <= hi;

					NiUpdateData updateData{ frameTime, 1, 0 };
					instance.nodePair.first->m_kLocal.m_Translate = (instance.nodePair.second->m_kWorld.m_Translate - instance.nodePair.first->m_pkParent->m_kWorld.m_Translate);
					instance.nodePair.first->Update(updateData);

					if (auto* sysData = psys->m_spModelData->NiDynamicCast<NiPSysData>())
					{
						for (NiPSysModifier* mod : psys->m_kModifierList)
						{
							//if (mod) mod->Update(frameTime, sysData);
						}
					}

					return false;
				}),
			newActiveParticles.end());
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj)
	{
		NiNode* projNode = proj->Get3D();
		UInt32 sourceRef = proj->pSourceRef->uiFormID;
		UInt32 sourceWeap = proj->pSourceWeapon->uiFormID;

		activeProjectiles.emplace_back(proj);
		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap) == true)
			{
				const auto heatValue = std::make_shared<ProjectileHeatValue>(NiColorA(it.fx.currCol, 1.0), proj->uiFormID);

				// Mark value nodes to track (for emission later)
				for (const auto& valueNode : FindValueNodes(projNode)) 
				{
					trackedNodes[valueNode] = heatValue;
				}

				// Track initial heat for this projectile (later used for impact creation)
				projectileHeat[proj->uiFormID] = heatValue;

			}
		}
		ThisStdCall(0x9A52F0, a1, proj);
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress());
		Projectile* pProjectile = *reinterpret_cast<Projectile**>(ebp - 0x2B8);

		BSTempEffectParticle* impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);

		// BSTempEffectParticle::Create
		if (!impact) {
			return nullptr;
		}

		NiNodePtr impactNode = impact->spParticleObject->IsNiNode();

		// Harden this
		if (!impactNode || !projectileHeat.contains(pProjectile->uiFormID)) {
			return impact;
		}

		// Attempt to lock the weak ptr / track new value nodes
		if (const auto shared = projectileHeat[pProjectile->uiFormID].lock()) {
			for (const auto& valueNode : FindValueNodes(impactNode)) {
				trackedNodes[valueNode] = shared;
			}
		}
		return impact;
	}

	inline void __fastcall MuzzleFlashEnable(MuzzleFlash* muzzleFlash)
	{
		NiNodePtr muzzleNode = muzzleFlash->spNode;
		UInt32 sourceRef = muzzleFlash->pSourceActor->uiFormID;
		UInt32 sourceWeap = muzzleFlash->pSourceWeapon->uiFormID;

		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap))
			{
				SetEmissiveClone(muzzleNode.m_pObject, it.fx.currCol);
				muzzleFlash->spLight->SetDiffuseColor(it.fx.currCol);
			}
		}
		ThisStdCall(0x9BB690, muzzleFlash);
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		UInt32 sourceRef = rActor->uiFormID;
		UInt32 sourceWeap = rWeap->uiFormID;
		NiNodePtr sourceNode = rActor->Get3D();
																			
		bool found = false;
		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap))
			{
				found = true;
				it.heat.HeatOnFire();
				break; 
			}
		}
		if (!found)
		{
			auto weaponIt = weaponDataMap.find(sourceWeap);
			if (weaponIt != weaponDataMap.end())
			{
				HeatInstance newHeat = MakeHeatFromTemplate(weaponIt->second, sourceNode.m_pObject, sourceRef, sourceWeap);
				newHeat.heat.HeatOnFire();
				newHeat.fx.currCol = newHeat.fx.SmoothShift(newHeat.heat.heatVal, newHeat.heat.maxHeat);

				for (NiAVObjectPtr it : newHeat.targetBlocks)
				{
					SetEmissiveClone(it, newHeat.fx.currCol);
				}
				activeWeapons.emplace_back(newHeat);
			}
		}
		// Calls original Actor::FireWeapon
		ThisStdCall(0x523150, rWeap, rActor);
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

	inline void __fastcall ColorModifierUpdate(BSPSysSimpleColorModifier* apThis, void* edx, float afTime, NiPSysData* apData) {
		if (colorModifiers.contains(apThis)) {
			if (apData->m_pkColor) {

				const auto oldColor = apData->m_pkColor[0];

				OriginalColorModifierUpdate(apThis, edx, afTime, apData);

				const auto newColor = apData->m_pkColor[0];
				for (int i = 0; i < apData->m_usVertices; i++)
				{
					apData->m_pkColor[i] = oldColor;
				}

				//Now update alpha of each affected heat controller
				for (const auto& hv : colorModifiers[apThis]) {
					if (const auto sptr = hv.lock()) {
						sptr->kColor.a = newColor.a;
					}
				}

				//Update vertex color alphas
				for (int i = 0; i < apData->m_usActiveVertices; i++) {
					apData->m_pkColor[i].a = newColor.a;
				}
			}
			//Skip vanilla
			return;
		}
		OriginalColorModifierUpdate(apThis, edx, afTime, apData);
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

		//WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		//WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
		WriteRelCall(MPSAddon, &MPSAddonWrapper); 
		WriteRelJump(initialPosVelocity, &InitialPosVelocityHook);
		WriteRelJump(clearUnrefEmitters, &ClearUnrefEmittersHook);
		WriteRelJump(colorModUpdate, &ColorModifierUpdate);
		//SafeWrite8(0xC602E5, 0x90);
	}

}