#include "MainHeader.h"
#include "InitHooks.h"
#include "NiNodes.h"
#include "NiTypes.h"
#include "NiPoint.h"
#include "NiObjects.h"
#include "GameObjects.h"
#include "OverCharge.h"

namespace Overcharge
{
	UInt32 originalAddress; 
	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::Create();
	ColorShift shiftedColor(PlasmaColor::plasmaColorSet[1], PlasmaColor::plasmaColorSet[5], 0.15f);
	WeaponHeat heatSystem = WeaponHeat(50.0f, 20.0f, 5.0f); 
	//NiGeometryData* g_customPlayerGeomDat = NiGeometryData::Create();

	void SetEmissiveRGB(TESObjectREFR* actorRef, const char* blockName, HeatRGB blendedColor)
	{

		if (NiNode* niNode = actorRef->GetNiNode())
		{
			if (NiAVObject* block = niNode->GetBlock(blockName))
			{
				((NiGeometry*)block)->materialProp = g_customPlayerMatProperty; 
				((NiGeometry*)block)->materialProp->emissiveRGB.r = blendedColor.heatRed;
				((NiGeometry*)block)->materialProp->emissiveRGB.g = blendedColor.heatGreen;
				((NiGeometry*)block)->materialProp->emissiveRGB.b = blendedColor.heatBlue;
				((NiGeometry*)block)->materialProp->emitMult = 2.0;
			}
		}
	}

	void __fastcall FireWeaponWrapper(TESForm* rWeap, void* edx, TESObjectREFR* rActor)
	{
		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();
		const char* blockName = "##PLRPlane1:0"; //Plasma Rifle zap effect in the barrel 
		
		heatSystem.HeatOnFire();
		//heatSystem.HeatCooldown(50.0f);
		//heatSystem.UpdateCooldown(50.0f);
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 

		HeatRGB blendedColor = shiftedColor.Shift();

		SetEmissiveRGB(actorRef, blockName, blendedColor); 
		ThisStdCall<int>(originalAddress, rWeap, rActor);
	}

	void __fastcall CooldownWrapper()
	{
		heatSystem.UpdateCooldown(50.0f);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	}

	void InitHooks()
	{
		UInt32 actorFireAddr = 0x8BADE9; //0x8BADE9 Actor:FireWeapon

		AppendToCallChain(actorFireAddr, UInt32(FireWeaponWrapper), originalAddress); 
	}

	void InitHooks1()
	{
		CooldownWrapper();
	}
}
