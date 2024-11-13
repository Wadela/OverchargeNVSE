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

	std::unordered_map<UInt32, WeaponHeat> heatedWeapons;														//Vector containing all weapons that are currently heating up
	
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

	void DevKitFork(TESForm* rWeap, TESObjectREFR* rActor)
	{
		ColorShift colorshift;

		//Color Data: color type, start color, target color, increment
		AuxVector* colorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);
		//Heat Data: starting heat, heat per shot, cooldown rate (per second)
		AuxVector* heatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);
		//Node Data: names of node(s) being put into SetEmmissiveRGB()
		AuxVector* nodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);

		if (!colorDataArgs || !heatDataArgs || !nodeDataArgs) { //If any of these are null, skip.
			ThisStdCall<int>(originalAddress, rWeap, rActor);						//Plays original Actor::FireWeapon
			return;
		}

		if ((*colorDataArgs)[0].type == kRetnType_String)						
		{
			const char* colorType = (*colorDataArgs)[0].str;	//"Plasma", "Laser", "Flame", "Zap"
			const ColorGroup* selectedCG = ColorGroup::GetColorSet(colorType);
			if (selectedCG) {
				//0 (Red), 1 (Orange), 2 (Yellow), 3 (Green), 4 (Blue), 5 (Violet), 6 (White)
				UInt32 colorStart = (UInt32)(*colorDataArgs)[1].num;
				if (colorStart >= 6)
				{
					colorStart = 0;												//0 if invalid color is selected
				}
				UInt32 colorTarget = (UInt32)(*colorDataArgs)[2].num;
				if (colorTarget >= 6)
				{
					colorTarget = 0;											//0 if invalid color is selected
				}
				float colorIncrement = ((*colorDataArgs)[3].num) / 100.0f;	//Percent that color shifts out of 100
				colorshift = ColorShift(selectedCG->colorSet[colorStart], selectedCG->colorSet[colorTarget], colorIncrement);
			}
		}
					
		double overchargeHeatStart = (*heatDataArgs)[0].num;      // Starting [Default] Heat Level when you pull out the gun or the number it cools down to
		double overchargeHeatPerShot = (*heatDataArgs)[1].num;
		double overchargeCooldown = (*heatDataArgs)[2].num;       // The amount of heat that decays per second, never below starting heat level

		auto iter = heatedWeapons.find(rWeap->refID);
		if (iter == heatedWeapons.end()) {
			auto pair = heatedWeapons.emplace(rWeap->refID, WeaponHeat(overchargeHeatStart, overchargeHeatPerShot, overchargeCooldown));
			iter = pair.first;
		}
		iter->second.HeatOnFire();

		//blockNames.clear();
		HeatRGB blendedColor = colorshift.Shift();
		for (size_t i = 0; i < nodeDataArgs->size(); ++i)						//Iterates through and adds all items to blockNames
		{
			if ((*nodeDataArgs)[i].type == kRetnType_String)					//Block names (i.e. "##PLRPlane1:0" for zap effect in Plasma Rifle
			{
				const char* overchargeBlockName = (*nodeDataArgs)[i].str;
				//blockNames.emplace_back(overchargeBlockName);
				SetEmissiveRGB(rActor, overchargeBlockName, blendedColor);
			}
		}

		ThisStdCall<int>(originalAddress, rWeap, rActor);						//Plays original Actor::FireWeapon
		return;

	}

	void __fastcall FireWeaponWrapper(TESForm* rWeap, void* edx, TESObjectREFR* rActor)
	{
		TESObjectREFR* actorRef = PlayerCharacter::GetSingleton();

		if (PluginFunctions::pNVSE == true)
		{
			DevKitFork(rWeap, rActor);
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
