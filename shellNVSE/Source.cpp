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
	
	std::vector<const char*> blockNames;

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

	void DevKitFork(TESForm* rWeap, TESObjectREFR* rActor, ColorShift& matColorShift)
	{
		//Color Data: color type, start color, target color, increment
		AuxVector* colorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);
		const char* colorType;
		int colorStart;
		int colorTarget;
		float colorIncrement;

		if (colorDataArgs == nullptr || colorDataArgs->size() <= 4)				//Returns if invalid number of arguments are entered
		{
			ThisStdCall<int>(originalAddress, rWeap, rActor);
			return;
		}
		if ((*colorDataArgs)[0].type == kRetnType_String)						//"Plasma", "Laser", "Flame", "Zap"
		{
			colorType = (*colorDataArgs)[0].str;
			const ColorGroup& selectedCG = ColorGroup::GetColorSet(colorType);

			if ((*colorDataArgs)[1].type == kRetnType_Default)					//0 (Red), 1 (Orange), 2 (Yellow), 3 (Green), 4 (Blue), 5 (Violet), 6 (White)
			{
				colorStart = (*colorDataArgs)[1].num;
				if (colorStart >= selectedCG.size)
				{
					colorStart = 0;												//0 if invalid color is selected
				}
			}
			if ((*colorDataArgs)[2].type == kRetnType_Default)					//0 (Red), 1 (Orange), 2 (Yellow), 3 (Green), 4 (Blue), 5 (Violet), 6 (White)
			{
				colorTarget = (*colorDataArgs)[2].num;
				if (colorTarget >= selectedCG.size)
				{
					colorTarget = 0;											//0 if invalid color is selected
				}
			}
			if ((*colorDataArgs)[3].type == kRetnType_Default)					//Percent that color shifts out of 100
			{
				colorIncrement = ((*colorDataArgs)[3].num) / 100.0f;
				matColorShift = ColorShift(selectedCG.colorSet[colorStart], selectedCG.colorSet[colorTarget], colorIncrement);
			}
		}

		//Heat Data: starting heat, heat per shot, cooldown rate (per second)
		AuxVector* heatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);
		float overchargeHeatStart;
		float overchargeHeatPerShot;
		float overchargeCooldown;

		if (heatDataArgs == nullptr || heatDataArgs->size() <= 3)				//Returns if invalid number of arguments are entered
		{
			ThisStdCall<int>(originalAddress, rWeap, rActor);
			return;
		}
		if ((*heatDataArgs)[0].type == kRetnType_Default)						//Starting [Default] Heat Level when you pull out the gun or the number it cools down to
		{
			overchargeHeatStart = (*heatDataArgs)[0].num;
		}
		if ((*heatDataArgs)[1].type == kRetnType_Default)						//The amount of heat that is added when you fire, max is 300
		{
			overchargeHeatPerShot = (*heatDataArgs)[1].num;
		}
		if ((*heatDataArgs)[2].type == kRetnType_Default)						//The amount of heat that decays per second, never below starting heat level
		{
			overchargeCooldown = (*heatDataArgs)[2].num;
		}
		if (heatedWeapons.empty())
		{
			heatedWeapons.emplace_back(WeaponHeat(overchargeHeatStart, overchargeHeatPerShot, overchargeCooldown));   
		}

		//Node Data: names of node(s) being put into SetEmmissiveRGB()
		AuxVector* nodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);
		const char* overchargeBlockName;
		blockNames.clear();

		if (nodeDataArgs == nullptr)
		{
			ThisStdCall<int>(originalAddress, rWeap, rActor);
			return;
		}
		for (size_t i = 0; i < nodeDataArgs->size(); ++i)						//Iterates through and adds all items to blockNames
		{
			if ((*nodeDataArgs)[i].type == kRetnType_String)					//Block names (i.e. "##PLRPlane1:0" for zap effect in Plasma Rifle
			{
				const char* overchargeBlockName = (*nodeDataArgs)[i].str;

				blockNames.emplace_back(overchargeBlockName);
			}
		}
	}

	void __fastcall FireWeaponWrapper(TESForm* rWeap, void* edx, TESObjectREFR* rActor)
	{
		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();

		if (PluginFunctions::pNVSE == true)
		{
			ColorShift shiftedColor;

			DevKitFork(rWeap, rActor, shiftedColor);
			heatedWeapons[0].HeatOnFire();
			HeatRGB blendedColor = shiftedColor.Shift();

			for (const char* blockName : blockNames)
			{
				SetEmissiveRGB(actorRef, blockName, blendedColor);
			}

			ThisStdCall<int>(originalAddress, rWeap, rActor);						//Plays original Actor::FireWeapon
		}
		else
		{
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
