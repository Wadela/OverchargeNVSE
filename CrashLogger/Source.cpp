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

namespace Overcharge
{
	std::vector<HeatedWeaponData*> activeWeapons;				//Vector containing all weapons that are currently heating up
	std::vector<Projectile*> activeProjectiles;

	NiPoint3* GetRoundedPosition(NiPoint3* pos)
	{
		float roundX = std::round(pos->x);
		float roundY = std::round(pos->y);
		float roundZ = std::round(pos->z);

		return new NiPoint3(roundX, roundY, roundZ);
	}

	static void SetEmissiveColor(NiAVObject* obj, NiMaterialProperty* matProp, NiColor& blendedColor, const char* blockName = nullptr)
	{
		NiAVObject* target = blockName ? obj->GetObjectByName(blockName) : obj;

		if (NiGeometry * geom = target->NiDynamicCast<NiGeometry>())
		{
			geom->m_kProperties.m_spMaterialProperty = matProp;
			geom->m_kProperties.m_spMaterialProperty->m_emit = blendedColor;
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
		TimeGlobal* timeGlobal = TimeGlobal::GetSingleton();

		activeWeapons.erase(
			std::remove_if(activeWeapons.begin(), activeWeapons.end(),
				[&](HeatedWeaponData* weaponData)
				{
					float cooldownStep = timeGlobal->fDelta * weaponData->heatData->cooldownRate;
					weaponData->heatData->heatVal -= cooldownStep;
					if (weaponData->heatData->heatVal <= weaponData->heatData->baseHeatVal)
						return true;

					NiColor blendedColor = weaponData->colorData->StepShift(
						weaponData->heatData->heatVal,
						weaponData->colorData->startIndex,
						weaponData->colorData->targetIndex,
						weaponData->colorData->colorType
					);
					weaponData->currentColor = &blendedColor;

					for (const char* overchargeBlockName : weaponData->blockNames)
					{
						SetEmissiveColor(weaponData->sourceModel, weaponData->sourceMatProp, blendedColor, overchargeBlockName);
					}

					return false;
				}),
			activeWeapons.end());
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj) 
	{
		NiNode* projNode = proj->Get3D();
		UInt32 sourceRef = proj->pSourceRef->uiFormID;
		UInt32 sourceWeap = proj->pSourceWeapon->uiFormID;
		
		for (auto it : activeWeapons)
		{
			if (it->CompareForms(sourceRef, sourceWeap))
			{
				if (!it->projectileModel || it->projectileModel != projNode)
				{
					it->projectileModel = projNode;
				}
				ProcessNiNode2(it->projectileModel, it->projMatProp, *it->currentColor);
			}
		}
		activeProjectiles.emplace_back(proj);

		ThisStdCall(0x9A52F0, a1, proj); 
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 impactPos, float a6, char a7, NiRefObject* parent) 
	{  
		BSTempEffectParticle* impact = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, impactPos, a6, a7, parent);
		NiNode* impactNode = impact->spParticleObject->IsNiNode();
		NiPoint3* roundedPos = GetRoundedPosition(&impactPos); 

		activeProjectiles.erase(
			std::remove_if(activeProjectiles.begin(), activeProjectiles.end(),
				[&](Projectile* it)
				{
					if (it->bHasImpacted || it->IsDestroyed() || it->IsDeleted()) return true;
					if (it->pSourceRef && it->pSourceWeapon)
					{
						UInt32 sourceRef = it->pSourceRef->uiFormID;
						UInt32 sourceWeap = it->pSourceWeapon->uiFormID;

						for (auto weaponIt : activeWeapons)
						{
							if (weaponIt->CompareForms(sourceRef, sourceWeap))
							{
								if (!weaponIt->impactModel || weaponIt->impactModel != impactNode)
								{
									weaponIt->impactModel = impactNode;
								}
								ProcessNiNode2(weaponIt->impactModel, weaponIt->impactMatProp, *weaponIt->currentColor);
							}
						}
					}
					return false;
				}),
			activeProjectiles.end());

		return impact;
	}

	inline void __fastcall MuzzleFlashEnable(MuzzleFlash* muzzleFlash)
	{
		NiNode* muzzleNode = muzzleFlash->spNode;
		UInt32 sourceRef = muzzleFlash->pSourceActor->uiFormID;
		UInt32 sourceWeap = muzzleFlash->pSourceWeapon->uiFormID;

		for (auto it : activeWeapons)
		{
			if (it->CompareForms(sourceRef, sourceWeap))
			{
				if (!it->muzzleFlashModel || it->muzzleFlashModel != muzzleNode)
				{
					it->muzzleFlashModel = muzzleNode;
				}
				ProcessNiNode2(it->muzzleFlashModel, it->muzzleMatProp, *it->currentColor);
				muzzleFlash->spLight->SetDiffuseColor(*it->currentColor);
			}
		}
		ThisStdCall(0x9BB690, muzzleFlash);
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		ThisStdCall(0x523150, rWeap, rActor);		//Plays original Actor::FireWeapon
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

		WriteRelCall(actorFire, &FireWeaponWrapper);
		WriteRelCall(createProjectile, &ProjectileWrapper); 
		WriteRelCall(spawnImpactEffects, &ImpactWrapper);  
		WriteRelCall(muzzleFlashEnable, &MuzzleFlashEnable); 
	}

}