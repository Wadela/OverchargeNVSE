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

	enum OverChargeWeaps
	{
		kAlienBlaster =		0x004322,
		kArcWelder =		0x00766B,
		kFlamer =			0x00432D,
		kFlamerCF =			0x00080B,
		kGatlingLaser =		0x00432E,
		kGatlingLaserSW =	0x000803,
		kHoloRifle =		0x0092EF,
		kIncinerator =		0x0906DA,
		kIncineratorHeavy = 0x0E2BFC,
		kLAER =				0x00B9CF,
		kLAERElijah =		0x015DD3,
		kLaserPistol =		0x004335,
		kLaserPistolGRA =	0x00084D,
		kLaserPistolPP =	0x103B1D,
		kLaserRCW =			0x09073B,
		kLaserRifle =		0x004336,

		kPlasmaRifle =		0x004344,
		kPlasmaRifleQ35 =	0x0E6064,




	};

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

	void SetEmissiveRGB(TESObjectREFR* actorRef, const char* blockName, const HeatRGB& emissiveColor)
	{
		if (NiNode* niNode = actorRef->GetNiNode())
		{
			if (NiAVObject* block = niNode->GetBlock(blockName))
			{
				//if (NiMaterialProperty* matProp = ((NiGeometry*)block)->materialProp->Create())
				if (NiAVObject* block = niNode->GetBlock(blockName))
				{
					//((NiGeometry*)block)->materialProp = matProp;
					HeatRGB currentColor(0.0f, 0.0f, 0.0f);
					float increment = 0.1f;
					((NiGeometry*)block)->materialProp = g_customPlayerMatProperty; 
					((NiGeometry*)block)->materialProp->emissiveRGB.r = emissiveColor.heatRed;  
					((NiGeometry*)block)->materialProp->emissiveRGB.g = emissiveColor.heatGreen;
					((NiGeometry*)block)->materialProp->emissiveRGB.b = emissiveColor.heatBlue;
					((NiGeometry*)block)->materialProp->emitMult = 2.0;
				}
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
