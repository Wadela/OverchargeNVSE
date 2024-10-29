#include "MainHeader.h"
#include "InitHooks.h"
#include "NiNodes.h"
#include "NiTypes.h"
#include "NiPoint.h"
#include "NiObjects.h"
#include "GameObjects.h"
#include "OverCharge.h"
#include <vector>
#include "ShellNVSE.h"

namespace Overcharge
{
	UInt32 originalAddress;																		//Needed for AppendToCallChain
	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::Create();				//Needed so that all instances won't change color when a single weapon fires

	std::vector<WeaponHeat> heatedWeapons;														//Vector containing all weapons that are currently heating up

	ColorShift shiftedColor(PlasmaColor::plasmaColorSet[1], PlasmaColor::plasmaColorSet[5], 0.15f);
	

	void SetEmissiveRGB(TESObjectREFR* actorRef, const char* blockName, HeatRGB blendedColor)	//Rewritten SetMaterialProperty function
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

	__declspec(naked) void __stdcall FireAnimDetour()
	{
		static const UInt32 returnAddr = 0x949CF1;

		__asm
		{
			cmp g_isOverheated, 0			//If Overheated
			jz skip							//Returns to original Animation if it isn't overheated
			mov ecx, 0xC9					//Plays kAnimGroup_JamB (201U)
			
		skip:
			push ecx						//ECX contains the fire AnimGroup
			mov ecx, [ebp - 0x9C]			//Rewriting original, overwritten instructions
			jmp returnAddr					//StartFireAnimation Function
		}
	}

	void devKitFork(TESForm* rWeap, TESObjectREFR* rActor)
	{
		/*
		AuxVector* ColorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);

		if (ColorDataArgs == nullptr || ColorDataArgs->size() <= 3)
		{
			ThisStdCall<int>(originalAddress, rWeap, rActor); 
			return;
		}
		if ((*ColorDataArgs)[0].type == kRetnType_String)
		{
			const char* overchargeColorType = (*ColorDataArgs)[0].str;
		}
		if ((*ColorDataArgs)[1].type == kRetnType_Default)
		{
			UInt32 overchargeColorStart = (*ColorDataArgs)[1].num;
		}
		if ((*ColorDataArgs)[2].type == kRetnType_Default)
		{
			UInt32 overchargeColorTarget = (*ColorDataArgs)[2].num;
		}

		AuxVector* HeatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);

		if (HeatDataArgs == nullptr || HeatDataArgs->size() <= 3)
		{
			ThisStdCall<int>(originalAddress, rWeap, rActor);
			return;
		}
		if ((*HeatDataArgs)[0].type == kRetnType_Default)
		{
			UInt32 overchargeHeatStart = (*HeatDataArgs)[0].num;
		}
		if ((*HeatDataArgs)[1].type == kRetnType_Default)
		{
			UInt32 overchargeHeatPerShot = (*HeatDataArgs)[1].num;
		}
		if ((*HeatDataArgs)[2].type == kRetnType_Default)
		{
			UInt32 overchargeCooldown = (*HeatDataArgs)[2].num;
		}

		AuxVector* NodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);
		if (NodeDataArgs == nullptr || NodeDataArgs->size() <= 1)
		{
			ThisStdCall<int>(originalAddress, rWeap, rActor);
			return;
		}
		if ((*NodeDataArgs)[0].type == kRetnType_String)
		{
			const char* overchargeBlockName = (*NodeDataArgs)[0].str;
		}
		*/
	}

	void __fastcall FireWeaponWrapper(TESForm* rWeap, void* edx, TESObjectREFR* rActor)
	{
		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();
		const char* blockName = "##PLRPlane1:0";								//Plasma Rifle zap effect in the barrel 

		if (PluginFunctions::pNVSE == true) 
		{
			devKitFork(rWeap, rActor);
		}
		else
		{
			if (heatedWeapons.empty())
			{
				heatedWeapons.emplace_back(WeaponHeat(50.0f, 40.0f, 20.0f));
			}
			heatedWeapons[0].HeatOnFire();


			HeatRGB blendedColor = shiftedColor.Shift();

			SetEmissiveRGB(actorRef, blockName, blendedColor);
			ThisStdCall<int>(originalAddress, rWeap, rActor);						//Plays original Actor::FireWeapon
		}
	}

	void InitHooks()
	{
		UInt32 actorFireAddr = 0x8BADE9; //0x8BADE9 Actor:FireWeapon
		UInt32 startFireAnim = 0x949CEA; //0x949CF1 Start Fire Animation

		AppendToCallChain(actorFireAddr, UInt32(FireWeaponWrapper), originalAddress); 
		WriteRelJump(startFireAnim, UInt32(FireAnimDetour));  
	} 
}
