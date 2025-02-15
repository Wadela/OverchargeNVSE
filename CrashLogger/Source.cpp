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

namespace Overcharge
{
	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::CreateObject();
	NiMaterialProperty* g_customActorMatProperty = NiMaterialProperty::CreateObject();

	std::unordered_map<UInt32, WeaponData> heatedWeapons;								//Vector containing all weapons that are currently heating up

	void SetEmissiveColor(NiAVObject* obj, NiColor& blendedColor, const char* blockName = nullptr)
	{
		NiAVObject* target = blockName ? obj->GetObjectByName(blockName) : obj;

		if (target && target->IsNiType<NiGeometry>())
		{
			NiGeometry* geom = static_cast<NiGeometry*>(target);
			geom->m_kProperties.m_spMaterialProperty = g_customPlayerMatProperty;
			geom->m_kProperties.m_spMaterialProperty->m_emit = blendedColor;
		}
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
					ChangeParticleColor(childValNode, blendedColor);
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

	void WeaponCooldown()
	{
		TimeGlobal* timeGlobal = TimeGlobal::GetSingleton();
		for (auto it = Overcharge::heatedWeapons.begin(); it != Overcharge::heatedWeapons.end();)
		{
			auto& weaponData = it->second;
			float cooldownStep = timeGlobal->fDelta * weaponData.heatData.cooldownRate;
			if ((weaponData.heatData.heatVal -= cooldownStep) <= weaponData.heatData.baseHeatVal)
			{
				g_isOverheated = 0;
				it = Overcharge::heatedWeapons.erase(it);
			}
			else
			{
				NiColor blendedColor = weaponData.colorData.StepShift(weaponData.heatData.heatVal, weaponData.colorData.startIndex, weaponData.colorData.targetIndex, weaponData.colorData.colorType);
				for (const char* overchargeBlockName : weaponData.blockNames)
				{
					SetEmissiveColor(weaponData.meshData, blendedColor, overchargeBlockName);
				}
				++it;
			}
		}
	}

	void HeatColorEffect(UInt32 formID, NiAVObject* obj, const char* blockName)
	{
		const ColorGroup* selectedCG = ColorGroup::GetColorSet("Laser");
		ColorShift shiftedColor(selectedCG, selectedCG->GetColor(3), selectedCG->GetColor(5), 3, 5);
		WeaponHeat heatData = WeaponHeat(0.0f, 40.0f, 25.0f);
		auto iter = heatedWeapons.try_emplace(formID, WeaponData(obj, shiftedColor, heatData)).first;
		iter->second.heatData.HeatOnFire();
		NiColor blendedColor = shiftedColor.StepShift(iter->second.heatData.heatVal, 3, 5, selectedCG);
		if (blockName)
		{
			iter->second.blockNames.emplace_back(blockName);
			SetEmissiveColor(obj, blendedColor, blockName);
		}
		else
		{
			if (auto objNode = obj->IsNiNode())
			{
				ProcessNiNode2(objNode, blendedColor);
			}
		}
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj) 
	{
		TESObjectREFR* objRef = proj->pSourceRef;
		TESObjectREFR* playerRef = PlayerCharacter::GetSingleton();
		UInt32 formID = objRef->uiFormID; 
		UInt32 projFormID = proj->uiFormID;
		NiNode* projNode = proj->Get3D();
		NiNode* playerNode = playerRef->Get3D();
		NiColor col = (0, 0, 1);
		HeatColorEffect(projFormID, projNode, nullptr);
		HeatColorEffect(formID, playerNode, "##PLRCylinder1:0"); 
		HeatColorEffect(formID, playerNode, "##LPSideLatch:0");

		ThisStdCall(0x9A52F0, a1, proj); 
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 a5, float a6, char a7, NiRefObject* parent) 
	{  
		BSTempEffectParticle* result = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, a5, a6, a7, parent);

		NiNode* resultNode = result->spParticleObject->IsNiNode(); 
		HeatColorEffect(cell->uiFormID, resultNode, nullptr); 

		return result;
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;		//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;		//0x447168 Checks loaded NIF file
		UInt32 CreateProjectile = 0x9BD518; 
		UInt32 SpawnImpactAddr = 0x9C2058;	//Projectile::SpawnCollisionEffects @ Projectile::ProcessImpacts
		UInt32 SpawnImpactEffects = 0x9C2AC3; 

		// Hooks

		WriteRelCall(CreateProjectile, &ProjectileWrapper);
		WriteRelCall(SpawnImpactEffects, &ImpactWrapper); 
	}

}