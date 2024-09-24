#include "MainHeader.h"
#include "InitHooks.h"
#include "NiNodes.h"
#include "NiTypes.h"
#include "NiPoint.h"
#include "NiObjects.h"
#include "GameObjects.h"
#include "OverCharge.h"

NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::Create();

namespace Overcharge
{
	UInt32 originalAddress; 

	/*const char* getMaterialBlock(TESForm* rWeap)
	{

		switch (rWeap->refID) 
		{
		case kPlasmaRifle:
		{
			return "##PLRPlane1:0"; 
		}
		break; 

		case kPlasmaRifleUnique:
		{
			return "##PLRPlane1:0";
		}
		break;
		} 
	}*/
	//struct HeatRGB; 
	//std::unordered_map<UInt32, ColorTiers> DefaultColorMap;
	void SetEmissiveRGB(TESObjectREFR* actorRef, const char* blockName)
	{

		if (NiNode* niNode = actorRef->GetNiNode())
		{
			if (NiAVObject* block = niNode->GetBlock(blockName))
			{
				//if (NiMaterialProperty* matProp = ((NiGeometry*)block)->materialProp->Create())
				//((NiGeometry*)block)->materialProp = matProp;

				((NiGeometry*)block)->materialProp = g_customPlayerMatProperty; 
				((NiGeometry*)block)->materialProp->emissiveRGB.r = 1.0;  
				((NiGeometry*)block)->materialProp->emissiveRGB.g = 0;	 
				((NiGeometry*)block)->materialProp->emissiveRGB.b = 0;
				((NiGeometry*)block)->materialProp->emitMult = 2.0;
			}
		}
	}

	void __fastcall FireWeaponWrapper(TESForm* rWeap, void* edx, TESObjectREFR* rActor)
	{
		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();
		const char* blockName = "##PLRPlane1:0"; //Plasma Rifle zap effect in the barrel 

		float increment = 0.100f; 
		HeatRGB targetColor = Default.blueTier; 
		HeatRGB currentColor = currentColor.ColorShift(targetColor, increment); 
		float emissiveRed = currentColor.heatRed; 
		float emissiveGreen = currentColor.heatGreen;
		float emissiveBlue = currentColor.heatBlue;

		SetEmissiveRGB(actorRef, blockName);

		ThisStdCall<int>(originalAddress, rWeap, rActor);
	}

	void InitHooks()
	{
		UInt32 actorFireAddr = 0x8BADE9; //0x8BADE9 Actor:FireWeapon

		AppendToCallChain(actorFireAddr, UInt32(FireWeaponWrapper), originalAddress); 
	}

}
