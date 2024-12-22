#include "MainHeader.hpp"
#include "Overcharge.hpp"

#include "NiRTTI.hpp"
#include "NiObject.hpp"
#include "NiParticleSystem.hpp"
#include "NiAVObject.hpp"

#include "NiHeatController.hpp"
#include "NifOverride.hpp"

namespace Overcharge
{
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
		if (system->IsGeometry() && strcmp(system->GetName(), "pShockTrail") == 0) {

			if (auto matprop = system->IsGeometry()->m_kProperties.m_spMaterialProperty) {
				matprop->m_emit = NiColor(0, 0, 1);
				matprop->m_fEmitMult = 1.2f;
			}
		}

		ThisStdCall(0xC1AEE0, system, newParticle);
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		SetEmissiveRGB(rActor, g_customPlayerMatProperty, "##PLRCylinder1:0", { 1.0f, 0.0f, 0.0f });

		ThisStdCall<int>(0x523150, rWeap, rActor);		//Plays original Actor::FireWeapon
	}

	void InitHooks()
	{
		// Hook Addresses

		UInt32 actorFire = 0x8BADE9;		//0x8BADE9 Actor:FireWeapon
		UInt32 check3DFile = 0x447168;		//0x447168 Checks loaded NIF file

		// Hooks

		WriteRelCall(actorFire, &FireWeaponWrapper);  
		//WriteRelCall(0xC2237A, &InitNewParticle);
		WriteRelCall(0x447168, &hkModelLoaderLoadFile);
	}

}