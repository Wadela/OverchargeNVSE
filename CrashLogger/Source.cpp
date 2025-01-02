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
#include "NiHeatController.hpp"
#include "NiStream.hpp"
#include "NiLight.hpp"
#include "NiParticleSystem.hpp"
#include "NiPointLight.hpp"
#include "TESDataHandler.hpp"
#include <BSTempEffectParticle.hpp>
#include <BSPSysSimpleColorModifier.hpp>
#include "NiUpdateData.hpp"
//#include "NiHeatController.hpp"
//#include "NifOverride.hpp"

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

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		SetEmissiveRGB(rActor, g_customPlayerMatProperty, "##PLRCylinder1:0", { 1.0f, 0.0f, 0.0f });

		ThisStdCall<int>(0x523150, rWeap, rActor);		//Plays original Actor::FireWeapon
	}

	BSTempEffectParticle* __cdecl hkTempEffectParticle(void* a1, float a2, char* a3, NiPoint3 a4, NiPoint3 a5, float a6, char a7, void* a8) { 
		auto result = CdeclCall<BSTempEffectParticle*>(0x6890B0, a1, a2, a3, a4, a5, a6, a7, a8);
		// do shit

		auto particle = result->spParticleObject;
		NiNodePtr impactNode = particle->IsNiNode();
		NiColorA redColor = (1, 0, 0, 1); 
		NiUpdateData update = {true};
		if (NiAVObjectPtr block = impactNode->GetBlock("AddOnNode16"))
		{
			if (auto* valueNode = block->NiDynamicCast<BSValueNode>())
			{
				const auto addonNodeID = valueNode->iValue;
				const auto addonNode = TESDataHandler::GetSingleton()->GetAddonNode(addonNodeID);

				auto manager = BSParticleSystemManager::GetInstance();
				auto particleSystemIndex = addonNode->particleSystemID;

				auto mps = manager->GetMasterParticleSystem(particleSystemIndex)->NiDynamicCast<BSMasterParticleSystem>();
				auto newNode = mps->GetAt(0)->Clone()->NiDynamicCast<NiNode>();
				auto childrenSize = newNode->m_kChildren.m_usESize;
				for (int i = 0; i < childrenSize; ++i) { 
					if (auto child = newNode->m_kChildren.m_pBase[i])  
					{
						auto childGeom = child->IsGeometry(); 
						auto childParticle = child->NiDynamicCast<NiParticleSystem>();
						if (childParticle)
						{
							for (auto it = childParticle->m_kModifierList.begin(); it != childParticle->m_kModifierList.end(); ++it)
							{
								auto colorMod = it->NiDynamicCast<BSPSysSimpleColorModifier>();
								if (colorMod)
								{
									colorMod->m_pkTarget->m_kProperties.m_spMaterialProperty.m_pObject->m_emit = NiColor(1, 0, 0);
									colorMod->m_pkTarget->m_spModelData->m_pkColor->r = 1;
									colorMod->m_pkTarget->m_spModelData->m_pkColor->g = 0;
									colorMod->m_pkTarget->m_spModelData->m_pkColor->b = 0;
									colorMod->m_pkTarget->m_spModelData->m_pkColor->a = 1;

									colorMod->kColor1.r = 1;
									colorMod->kColor1.g = 0;
									colorMod->kColor1.b = 0;
									colorMod->kColor1.a = 1;

									colorMod->kColor2.r = 1;
									colorMod->kColor2.g = 0;
									colorMod->kColor2.b = 0;
									colorMod->kColor2.a = 1;

									colorMod->kColor3.r = 1;
									colorMod->kColor3.g = 0;
									colorMod->kColor3.b = 0;
									colorMod->kColor3.a = 1;


									childGeom->m_spModelData->m_pkColor->r = 1;
									childGeom->m_spModelData->m_pkColor->g = 0;
									childGeom->m_spModelData->m_pkColor->b = 0;
									childGeom->m_spModelData->m_pkColor->a = 0;
									//result->Update(0);
									//colorMod->SetSystemPointer(it->m_pkTarget);
									auto joebiden = childParticle->m_spControllers;
								}
							}
							childGeom->m_kProperties.m_spMaterialProperty->m_emit = NiColor(1, 0, 0);
							childGeom->GetRTTIName();
						}
						//newNode->AttachChild(child->Clone()->NiDynamicCast<NiNode>(), true); 
					}
				}
			}
		}
		return result;
	}

	BSTempEffectParticle* __cdecl hkTempEffectParticle2(void* a1, float a2, char* a3, NiPoint3 a4, NiPoint3 a5, float a6, char a7, void* a8) {
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

		//WriteRelCall(actorFire, &FireWeaponWrapper); 
		//WriteRelCall(0x9C2AC3, &hkTempEffectParticle); 
		//WriteRelCall(0xC2237A, &InitNewParticle);
		//WriteRelCall(0x447168, &hkModelLoaderLoadFile);
	}

}