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

namespace Overcharge
{
	NiMaterialPropertyPtr g_customPlayerMatProp = NiMaterialProperty::CreateObject();

	std::vector<HeatedWeaponData> activeWeapons;				//Vector containing all weapons that are currently heating up
	std::vector<Projectile*> activeProjectiles;
	std::unordered_map<UInt32, HeatedWeaponData> weaponDataMap;
	std::vector<NiTimeController*> particleControllers;
	std::vector<NiTimeController*> particleControllers1;
	std::vector<NiParticleSystem*> activeParticles;
	std::vector<NiPSysModifier*> activeModifiers; 

	struct ParticleInstance
	{
		NiParticleSystem* particle;
		std::pair<NiNode*, NiNode*> nodePair;
	};

	std::vector<ParticleInstance> newActiveParticles;

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

	static void SetEmissiveColor(NiAVObject* obj, NiMaterialProperty* matProp, NiColor& blendedColor, const char* blockName = nullptr)
	{
		if (!obj) return;

		NiAVObject* target = blockName ? obj->GetObjectByName(blockName) : obj;

		if (!target) return;

		if (NiGeometry* geom = target->NiDynamicCast<NiGeometry>())
		{
			if (!geom->m_kProperties.m_spMaterialProperty) return;
			geom->m_kProperties.m_spMaterialProperty = matProp;
			geom->m_kProperties.m_spMaterialProperty->m_emit = blendedColor;
			geom->m_kProperties.m_spMaterialProperty->m_fEmitMult = 1.5f;
		}
	}

	//Update Child Particles to all be prepared for emissive color control
	static void ChangeParticleColor(const NiAVObject* obj, NiMaterialProperty* matProp, NiColor& blendedColor)
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
						SetEmissiveColor(mps->kChildParticles[i], matProp, blendedColor);
					}
				}
			}
		}
	}

	//Update Child Particles to all be prepared for emissive color control
	static void CloneParticle(const NiAVObject* obj)
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

	static void ProcessNiNode2(const NiNode* obj, NiMaterialProperty* matProp, NiColor& blendedColor)
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
					SetEmissiveColor(childPsys, matProp, blendedColor);
				}
				else if (child->IsNiType<BSValueNode>())
				{
					BSValueNode* childValNode = static_cast<BSValueNode*>(child);
					ChangeParticleColor(childValNode, matProp, blendedColor); 
					CloneParticle(childValNode);
					//ProcessNiNode2(childValNiNode, matProp, blendedColor);
				}
				else if (child->IsNiType<NiNode>())
				{
					NiNode* childNode = static_cast<NiNode*>(child);
					ProcessNiNode2(childNode, matProp, blendedColor);
				}
				else if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					SetEmissiveColor(childGeom, matProp, blendedColor); 
				} 
			}
		}
	}

	void WeaponCooldown()
	{
		float frameTime = TimeGlobal::GetSingleton()->fDelta;

		activeWeapons.erase( 
			std::remove_if(activeWeapons.begin(), activeWeapons.end(), 
				[&](HeatedWeaponData& weaponData) 
				{
					weaponData.heatData.heatVal -= frameTime * weaponData.heatData.cooldownRate; 
					if (weaponData.heatData.heatVal <= weaponData.heatData.baseHeatVal) return true; 

					weaponData.currentColor = weaponData.SmoothShift(
						weaponData.heatData.heatVal, 
						weaponData.heatData.maxHeat, 
						weaponData.startingColor,
						weaponData.targetColor
					);

					if (weaponData.sourceModel)
					{
						for (const char* overchargeBlockName : weaponData.blockNames)
						{
							SetEmissiveColor(weaponData.sourceModel, weaponData.sourceMatProp, weaponData.currentColor, overchargeBlockName);
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

					if (!psys || !psys->m_spControllers) return true;

					psys->m_fLastTime += frameTime;

					if (auto* sysData = psys->m_spModelData->NiDynamicCast<NiPSysData>())
					{
						for (NiPSysModifier* mod : psys->m_kModifierList)
						{
							if (mod) mod->Update(psys->m_fLastTime, sysData);
						}
					}

					const float hi = psys->m_spControllers->m_fHiKeyTime;
					const float lingerThreshold = hi + 4.0f;
					const bool inLingerWindow = psys->m_fLastTime <= lingerThreshold;

					if (inLingerWindow && !AreNiTransformsNearlyEqual(instance.nodePair.first->m_kWorld, instance.nodePair.second->m_kWorld))
					{
						NiUpdateData updateData{ psys->m_fLastTime, 1, 0, 1, 1, 1 };

						psys->Update(updateData);
						instance.nodePair.first->m_kLocal = instance.nodePair.second->m_kLocal;
						instance.nodePair.first->m_kWorld = instance.nodePair.second->m_kWorld;
					}
					return !inLingerWindow;
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
				if (!it.projectileModel || it.projectileModel != projNode)
				{
					it.projectileModel = projNode;
				}
				ProcessNiNode2(projNode, it.projMatProp, it.currentColor); 
			}
		}
		ThisStdCall(0x9A52F0, a1, proj);
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent)
	{
		BSTempEffectParticle* impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);
		NiNode* impactNode = impact->spParticleObject->IsNiNode();
		NiPoint3 roundedPos = GetRoundedPosition(&impactPos);

		activeProjectiles.erase(
			std::remove_if(activeProjectiles.begin(), activeProjectiles.end(),
				[&](Projectile* it)
				{
					if (it->bHasImpacted || it->IsDestroyed() || it->IsDeleted()) return true;
					if (it->pSourceRef && it->pSourceWeapon)
					{
						UInt32 sourceRef = it->pSourceRef->uiFormID;
						UInt32 sourceWeap = it->pSourceWeapon->uiFormID;

						for (auto& weaponIt : activeWeapons)
						{
							if (weaponIt.CompareForms(sourceRef, sourceWeap))
							{
								if (!weaponIt.impactModel || weaponIt.impactModel != impactNode)
								{
									weaponIt.impactModel = impactNode;
								}
							}
						}
					}
					return roundedPos == GetRoundedPosition(&it->kPosition);
				}),
			activeProjectiles.end());

		return impact;
	}

	inline void __fastcall MuzzleFlashEnable(MuzzleFlash* muzzleFlash)
	{
		NiNode* muzzleNode = muzzleFlash->spNode;
		UInt32 sourceRef = muzzleFlash->pSourceActor->uiFormID;
		UInt32 sourceWeap = muzzleFlash->pSourceWeapon->uiFormID;

		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap))
			{
				if (!it.muzzleFlashModel || it.muzzleFlashModel != muzzleNode)
				{
					it.muzzleFlashModel = muzzleNode;
				}
				muzzleFlash->spLight->SetDiffuseColor(it.currentColor);
			}
		}
		ThisStdCall(0x9BB690, muzzleFlash);
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		UInt32 sourceRef = rActor->uiFormID;
		UInt32 sourceWeap = rWeap->uiFormID;
		NiNode* sourceNode = rActor->Get3D();
																			
		bool found = false;
		for (auto& it : activeWeapons)
		{
			if (it.CompareForms(sourceRef, sourceWeap))
			{
				found = true;
				it.sourceModel = sourceNode;
				it.heatData.HeatOnFire();
				break; 
			}
		}
		if (!found)
		{
			auto weaponIt = weaponDataMap.find(sourceWeap);
			if (weaponIt != weaponDataMap.end())
			{
				HeatedWeaponData newWeaponData = weaponIt->second;
				NiMaterialPropertyPtr sourceMProp = NiMaterialProperty::CreateObject(); 
				NiMaterialPropertyPtr muzzleMProp = NiMaterialProperty::CreateObject();
				NiMaterialPropertyPtr projMProp = NiMaterialProperty::CreateObject(); 
				NiMaterialPropertyPtr impactMProp = NiMaterialProperty::CreateObject(); 

				newWeaponData.sourceModel = sourceNode;
				newWeaponData.actorForm = sourceRef;
				newWeaponData.weaponForm = sourceWeap;
				newWeaponData.sourceMatProp = sourceMProp;
				newWeaponData.muzzleMatProp = muzzleMProp;
				newWeaponData.projMatProp = projMProp;
				newWeaponData.impactMatProp = impactMProp;
				newWeaponData.heatData.HeatOnFire();
				activeWeapons.emplace_back(newWeaponData);
			}
		}
		// Calls original Actor::FireWeapon
		ThisStdCall(0x523150, rWeap, rActor);
	}

	void __fastcall InitNewParticle(NiParticleSystem* system, void* edx, int newParticle)
	{
		// Static variable to toggle between red and blue on each function call
		static bool toggle = false;

		if (system->IsGeometry())
		{
			// Clone the system
			NiParticleSystem* newSystem = system->Clone()->NiDynamicCast<NiParticleSystem>();
			if (newSystem)
			{
				// Toggle the color based on the static variable
				if (toggle)
				{
					newSystem->m_kProperties.m_spMaterialProperty->m_emit = NiColor(1.0f, 0.0f, 0.0f); // Red
				}
				else
				{
					newSystem->m_kProperties.m_spMaterialProperty->m_emit = NiColor(0.0f, 0.0f, 1.0f); // Blue
				}

				// Flip the toggle for the next call
				toggle = !toggle;

				// Replace the original system with the new system
				system = newSystem;
			}
		}

		// Call the original function with the possibly new system
		ThisStdCall(0xC1AEE0, system, newParticle);
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

		// Hooks
		//WriteRelCall(0xC2237A, &InitNewParticle);
		WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
	}

}