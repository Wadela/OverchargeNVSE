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

	void SetEmissiveRGB(TESObjectREFR* actorRef, const char* blockName)
	{

		if (NiNode* niNode = actorRef->GetNiNode())
		{
			if (NiAVObject* block = niNode->GetBlock(blockName))
			{
				((NiGeometry*)block)->materialProp = g_customPlayerMatProperty; 
				((NiGeometry*)block)->materialProp->emissiveRGB.r = PlasmaColor::plasmaColorSet[4].heatRed;
				((NiGeometry*)block)->materialProp->emissiveRGB.g = PlasmaColor::plasmaColorSet[4].heatGreen;
				((NiGeometry*)block)->materialProp->emissiveRGB.b = PlasmaColor::plasmaColorSet[4].heatBlue;
				((NiGeometry*)block)->materialProp->emitMult = 2.0;
			}
		}
	}

	void __fastcall FireWeaponWrapper(TESForm* rWeap, void* edx, TESObjectREFR* rActor)
	{
		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();
		const char* blockName = "##PLRPlane1:0"; //Plasma Rifle zap effect in the barrel 

		SetEmissiveRGB(actorRef, blockName); 
		ThisStdCall<int>(originalAddress, rWeap, rActor);
	}

	void InitHooks()
	{
		UInt32 actorFireAddr = 0x8BADE9; //0x8BADE9 Actor:FireWeapon

		AppendToCallChain(actorFireAddr, UInt32(FireWeaponWrapper), originalAddress); 
	}

}
