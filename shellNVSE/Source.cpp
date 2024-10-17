#include "MainHeader.h"
#include "InitHooks.h"
#include "NiNodes.h"
#include "NiTypes.h"
#include "NiPoint.h"
#include "NiObjects.h"
#include "GameObjects.h"
#include "OverCharge.h"
#include <vector>

namespace Overcharge
{
	UInt32 originalAddress; 
	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::Create();

	std::vector<WeaponHeat> heatedWeapons;
	ColorShift shiftedColor(PlasmaColor::plasmaColorSet[1], PlasmaColor::plasmaColorSet[5], 0.15f);

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

		if (heatedWeapons.empty())
		{
			heatedWeapons.emplace_back(WeaponHeat(50.0f, 40.0f, 20.0f));
		}
		heatedWeapons[0].HeatOnFire();
		//heatedWeapons[0].CheckCooldown(); 
		HeatRGB blendedColor = shiftedColor.Shift();  

		SetEmissiveRGB(actorRef, blockName, blendedColor); 
		ThisStdCall<int>(originalAddress, rWeap, rActor);
	}

	void InitHooks()
	{
		UInt32 actorFireAddr = 0x8BADE9; //0x8BADE9 Actor:FireWeapon

		AppendToCallChain(actorFireAddr, UInt32(FireWeaponWrapper), originalAddress); 
	} 

}
