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

namespace Overcharge
{
	std::vector<BSPSysSimpleColorModifier*> colorMods;

	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::CreateObject();
	NiMaterialProperty* g_customActorMatProperty = NiMaterialProperty::CreateObject();

	std::unordered_map<UInt32, WeaponData> heatedWeapons;								//Vector containing all weapons that are currently heating up

	void SetEmissiveRGB(NiNode* obj, const char* blockName, NiColor& blendedColor)	//Rewritten SetMaterialProperty function
	{
		if (blockName)
		{
			if (NiAVObject* block = obj->GetObjectByName(blockName))
			{
				if (NiGeometry* blockGeom = static_cast<NiGeometry*>(block))
				{
					blockGeom->m_kProperties.m_spMaterialProperty = g_customPlayerMatProperty; 
					blockGeom->m_kProperties.m_spMaterialProperty->m_emit = blendedColor; 
					blockGeom->m_kProperties.m_spMaterialProperty->m_fEmitMult = 2.0; 
				}
			}
		}
		else if (NiNode* node = obj->IsNiNode())
		{
			for (int i = 0; i < node->m_kChildren.m_usSize; ++i)
			{
				NiAVObject* const child = node->m_kChildren[i].m_pObject;
				if (child->IsNiType<NiGeometry>())
				{
					NiGeometry* childGeom = static_cast<NiGeometry*>(child);
					childGeom->m_kProperties.m_spMaterialProperty = g_customPlayerMatProperty; 
					childGeom->m_kProperties.m_spMaterialProperty->m_emit = blendedColor; 
					childGeom->m_kProperties.m_spMaterialProperty->m_fEmitMult = 2.0; 
				}
				else if (child->IsNiType<NiNode>()) 
				{
					NiNode* childNode = static_cast<NiNode*>(child); 
					SetEmissiveRGB(childNode, nullptr, blendedColor); 
				} 
			}
		}
	}

	//Update Child Particles to all be prepared for emissive color control
	static void ChangeParticleColor(const NiAVObject* obj, NiColor& blendedColor)
	{
		if (BSValueNode* const valueNode = obj->NiDynamicCast<BSValueNode>())
		{
			BGSAddonNode* const addonNode = TESDataHandler::GetSingleton()->GetAddonNode(valueNode->iValue);
			if (addonNode && addonNode->kData.ucFlags.GetBit(1))
			{
				BSParticleSystemManager* const manager = BSParticleSystemManager::GetInstance();
				const UInt32 particleSystemIndex = addonNode->particleSystemID;
				if (BSMasterParticleSystem* const mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>())
				{
					if (NiNode* const mpsNode = mps->GetAt(0)->NiDynamicCast<NiNode>())
					{
						SetEmissiveRGB(mps, nullptr, blendedColor);
					}
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
					SetEmissiveRGB(weaponData.meshData, overchargeBlockName, blendedColor);
				}
				++it;
			}
		}
	}

	void HeatColorEffect(UInt32 formID, NiNode* obj, const char* blockName) 
	{
		const ColorGroup* selectedCG = ColorGroup::GetColorSet("Plasma");
		ColorShift shiftedColor(selectedCG, selectedCG->colorSet[0], selectedCG->colorSet[5], 0, 5);
		WeaponHeat heatData = WeaponHeat(50.0f, 40.0f, 30.0f);
		auto iter = heatedWeapons.try_emplace(formID, WeaponData(obj, shiftedColor, heatData)).first;
		iter->second.heatData.HeatOnFire();
		iter->second.blockNames.emplace_back(blockName);

		NiColor blendedColor = shiftedColor.StepShift(iter->second.heatData.heatVal, selectedCG->colorSet[1], selectedCG->colorSet[5], selectedCG);
		SetEmissiveRGB(obj, blockName, blendedColor);

		if (auto objNode = obj->IsNiNode()) 
		{
			for (int i = 0; i < objNode->m_kChildren.m_usSize; i++)
			{
				if (NiAVObject* const child = objNode->m_kChildren[i].m_pObject)
				{
					if (child->IsNiType<BSValueNode>())
					{
						ChangeParticleColor(static_cast<BSValueNode*>(child), blendedColor);
					}
				}
			}
		}
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* proj) 
	{
		TESObjectREFR* objRef = proj->pSourceRef;
		TESObjectREFR* playerRef = PlayerCharacter::GetSingleton();
		UInt32 formID = objRef->uiFormID; 
		NiNode* node = proj->Get3D();
		NiNode* playerNode = playerRef->Get3D();

		HeatColorEffect(formID, playerNode, "##PLRCylinder1:0");
	}

	BSTempEffectParticle* __cdecl ImpactWrapper(TESObjectCELL* cell, float lifetime, const char* fileName, NiPoint3 a4, NiPoint3 a5, float a6, char a7, NiRefObject* parent) 
	{  
		BSTempEffectParticle* result = CdeclCall<BSTempEffectParticle*>(0x6890B0, cell, lifetime, fileName, a4, a5, a6, a7, parent);
		// do shit
		NiNode* resultNode = result->spParticleObject->IsNiNode(); 
		HeatColorEffect(cell->uiFormID, resultNode, "Plane01:0"); 

		return result;
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;		//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;		//0x447168 Checks loaded NIF file
		UInt32 CreateProjectile = 0x9BD518; 

		// Hooks

		WriteRelCall(CreateProjectile, &ProjectileWrapper);
		WriteRelCall(0x9C2AC3, &ImpactWrapper);
		//WriteRelCall(actorFire, &FireWeaponWrapper); 
		//WriteRelCall(0x9C2AC3, &hkTempEffectParticle); 
		//WriteRelCall(0xC2237A, &InitNewParticle);
		//WriteRelCall(0x447168, &hkModelLoaderLoadFile);

		// Event Handlers
		//SetEventHandler("FireWeaponEvent", FireWeaponWrapper); 
	}

}