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

	std::unordered_map<UInt32, WeaponData> heatedWeapons;										//Vector containing all weapons that are currently heating up

	void SetEmissiveRGB(TESObjectREFR* actorRef, const char* blockName, NiColor blendedColor)	//Rewritten SetMaterialProperty function
	{
		if (NiNode* niNode = actorRef->Get3D())
		{
			if (NiAVObject* block = niNode->GetBlock(blockName))
			{
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty = g_customActorMatProperty;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.r = blendedColor.r;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.g = blendedColor.g;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.b = blendedColor.b;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_fEmitMult = 2.0;
			}
		}
	}

	void SetMuzzleRGB(NiNode* node, const char* blockName, NiColor blendedColor)	//Rewritten SetMaterialProperty function
	{
		if (NiAVObject* block = node->GetBlock(blockName))
		{
			((NiGeometry*)block)->m_kProperties.m_spMaterialProperty = g_customActorMatProperty;
			((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.r = blendedColor.r;
			((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.g = blendedColor.g;
			((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.b = blendedColor.b;
			((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_fEmitMult = 2.0;
		}
	}

	void WeaponCooldown()
	{
		TimeGlobal* timeGlobal = TimeGlobal::GetSingleton();

		for (auto it = Overcharge::heatedWeapons.begin(); it != Overcharge::heatedWeapons.end();)
		{
			auto& weaponData = it->second;
			float cooldownStep = timeGlobal->fDelta * it->second.heatData.cooldownRate;
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
					SetEmissiveRGB(weaponData.actorRef, overchargeBlockName, blendedColor); 
				}

				++it;
			}
		}
	}

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* projectile)
	{
		TESObjectREFR* rActor = projectile->pSourceRef;
		TESObjectWEAP* rWeap = projectile->pSourceWeapon;

		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();
		const char* blockName = "##PLRCylinder1:0"; //Plasma Rifle zap effect in the barrel 
		//const char* blockName = "pShockTrail";

		WeaponHeat heatData = WeaponHeat(50.0f, 40.0f, 30.0f);

		const ColorGroup* selectedCG = ColorGroup::GetColorSet("Plasma");
		ColorShift shiftedColor(selectedCG, selectedCG->colorSet[0], selectedCG->colorSet[5], 0, 5);

		auto iter = heatedWeapons.find(rActor->uiFormID);
		if (iter == heatedWeapons.end()) {
			auto pair = heatedWeapons.emplace(rWeap->uiFormID, WeaponData(rActor, shiftedColor, heatData));
			iter = pair.first; 
		} 
		iter->second.heatData.HeatOnFire(); 

		NiColor blendedColor = shiftedColor.StepShift(iter->second.heatData.heatVal, selectedCG->colorSet[1], selectedCG->colorSet[5], selectedCG); 
		iter->second.blockNames.emplace_back(blockName); 
		SetEmissiveRGB(actorRef, blockName, blendedColor); 

		return;
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();
		const char* blockName = "##PLRCylinder1:0"; //Plasma Rifle zap effect in the barrel 
		//const char* blockName = "pShockTrail";

		WeaponHeat heatData = WeaponHeat(50.0f, 40.0f, 30.0f);

		const ColorGroup* selectedCG = ColorGroup::GetColorSet("Plasma");
		ColorShift shiftedColor(selectedCG, selectedCG->colorSet[0], selectedCG->colorSet[5], 0, 5);

		auto iter = heatedWeapons.find(rActor->uiFormID);
		if (iter == heatedWeapons.end()) {
			auto pair = heatedWeapons.emplace(rWeap->uiFormID, WeaponData(rActor, shiftedColor, heatData));
			iter = pair.first;
		}
		iter->second.heatData.HeatOnFire();

		NiColor blendedColor = shiftedColor.StepShift(iter->second.heatData.heatVal, selectedCG->colorSet[1], selectedCG->colorSet[5], selectedCG);
		iter->second.blockNames.emplace_back(blockName);
		SetEmissiveRGB(actorRef, blockName, blendedColor);


		ThisStdCall<int>(0x523150, rWeap, rActor);		//Plays original Actor::FireWeapon
	}

	//void __fastcall MuzzleFlashWrapper(MuzzleFlash* muzzleFlash)
	//{
		//ThisStdCall<int>()
	//}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;		//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;		//0x447168 Checks loaded NIF file
		UInt32 CreateProjectile = 0x9BD518; 

		// Hooks

		WriteRelCall(CreateProjectile, &ProjectileWrapper);
		//WriteRelCall(actorFire, &FireWeaponWrapper); 
		//WriteRelCall(0x9C2AC3, &hkTempEffectParticle); 
		//WriteRelCall(0xC2237A, &InitNewParticle);
		//WriteRelCall(0x447168, &hkModelLoaderLoadFile);

		// Event Handlers
		//SetEventHandler("FireWeaponEvent", FireWeaponWrapper); 
	}

}