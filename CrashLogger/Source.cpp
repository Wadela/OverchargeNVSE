#include "MainHeader.hpp"
#include "Overcharge.hpp"

#include "NiRTTI.hpp"
#include "NiObject.hpp"
#include "NiParticleSystem.hpp"
#include "NiAVObject.hpp"
#include <BSTempEffectParticle.hpp>
#include <BSValueNode.hpp>
#include <BSParticleSystemManager.h>
#include "TESDataHandler.hpp"

#include <filesystem>
#include <numbers>
#include <print>
#include <set>
#include <unordered_map>

#include "BGSAddonNode.hpp"
#include "BSMasterParticleSystem.hpp"
#include "BSParticleSystemManager.h"
#include "BSPSysMultiTargetEmitterCtlr.hpp"
#include "BSStream.hpp"
#include "BSValueNode.hpp"
#include "Defines.h"
#include "NiCloningProcess.hpp"
#include "NiGeometryData.hpp"
#include "NiNode.hpp"
#include "SafeWrite.hpp"
#include "TESMain.hpp"
#include "NiGeometry.hpp"
#include "NiStream.hpp"
#include "NiLight.hpp"
#include "NiParticleSystem.hpp"
#include "NiPointLight.hpp"
#include "TESDataHandler.hpp"
#include <BSTempEffectParticle.hpp>
#include <BSPSysSimpleColorModifier.hpp>
#include "NiUpdateData.hpp"
#include "TimeGlobal.hpp"
#include <TESObjectWEAP.hpp>
//#include "NifOverride.hpp"
#include "PluginAPI.hpp"

namespace Overcharge
{
	std::vector<BSPSysSimpleColorModifier*> colorMods;

	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::CreateObject();
	NiMaterialProperty* g_customActorMatProperty = NiMaterialProperty::CreateObject();

	TESObjectREFR* projectile;
	Actor* projOwner;
	TESObjectWEAP* weap;

	std::unordered_map<UInt32, WeaponData> heatedWeapons;										//Vector containing all weapons that are currently heating up

	bool CmpNiObject(NiObject* obj, int rttiAddr)
	{
		if (!obj) {
			return false;
		}

		const NiRTTI* CmpRTTI = obj->GetRTTI();
		const NiRTTI* RTTI = (NiRTTI*)rttiAddr;

		return (CmpRTTI == RTTI); 
	} 

	void SetEmissiveRGB(TESObjectREFR* actorRef, NiMaterialProperty* matProp, const char* blockName, HeatRGB blendedColor)	//Rewritten SetMaterialProperty function
	{
		if (NiNode* niNode = actorRef->Get3D())
		{
			if (NiAVObject* block = niNode->GetBlock(blockName))
			{
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty = matProp;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.r = blendedColor.heatRed;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.g = blendedColor.heatGreen;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_emit.b = blendedColor.heatBlue;
				((NiGeometry*)block)->m_kProperties.m_spMaterialProperty->m_fEmitMult = 2.0;
			}
		}
	}

	void __fastcall InitNewParticle(NiParticleSystem* system, void* edx, int newParticle)
	{
		system->GetStreamableRTTI();
		const NiRTTI* rtti = system->GetRTTI();
		if (system->IsGeometry() && strcmp(system->GetName(), "pRingImpact") == 0) 
		{
			system->GetRTTI();
		}

		ThisStdCall(0xC1AEE0, system, newParticle);
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
				HeatRGB blendedColor = weaponData.colorData.Shift(weaponData.heatData.heatVal, weaponData.colorData.startIndex, weaponData.colorData.targetIndex, weaponData.colorData.colorType);
				for (const char* overchargeBlockName : weaponData.blockNames)
				{
					SetEmissiveRGB(weaponData.actorRef, weaponData.matProperty, overchargeBlockName, blendedColor);
				}

				++it;
			}
		}
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		ThisStdCall<int>(0x523150, rWeap, rActor);		//Plays original Actor::FireWeapon
	}

	BSTempEffectParticle* __cdecl hkTempEffectParticle(void* a1, float a2, char* a3, NiPoint3 a4, NiPoint3 a5, float a6, char a7, void* a8) {
		auto result = CdeclCall<BSTempEffectParticle*>(0x6890B0, a1, a2, a3, a4, a5, a6, a7, a8);
		// do shit

		return result;
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;		//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;		//0x447168 Checks loaded NIF file

		// Hooks

		WriteRelCall(actorFire, &FireWeaponWrapper); 
		//WriteRelCall(0x9C2AC3, &hkTempEffectParticle); 
		//WriteRelCall(0xC2237A, &InitNewParticle);
		//WriteRelCall(0x447168, &hkModelLoaderLoadFile);

		// Event Handlers
		//SetEventHandler("FireWeaponEvent", FireWeaponWrapper); 
	}

}