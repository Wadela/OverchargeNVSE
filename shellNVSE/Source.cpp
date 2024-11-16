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

	std::unordered_map<UInt32, WeaponData> heatedWeapons;										//Vector containing all weapons that are currently heating up

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
		AuxVector* colorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);
		AuxVector* heatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);
		AuxVector* nodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);

		if (!colorDataArgs || !heatDataArgs || !nodeDataArgs) {						//If any of these are null, skip.
			ThisStdCall<int>(originalAddress, rWeap, rActor);						//Plays original Actor::FireWeapon
			return;
		}

		double overchargeHeatStart = (*heatDataArgs)[0].num;  
		double overchargeHeatPerShot = (*heatDataArgs)[1].num;
		double overchargeCooldown = (*heatDataArgs)[2].num;      
		WeaponHeat heatData = { overchargeHeatStart, overchargeHeatPerShot, overchargeCooldown };

		if ((*colorDataArgs)[0].type == kRetnType_String)						
		{
			const char* colorType = (*colorDataArgs)[0].str;
			const ColorGroup* selectedCG = ColorGroup::GetColorSet(colorType);

			if (selectedCG) {
				UInt32 colorStart = (UInt32)(*colorDataArgs)[1].num;
				UInt32 colorTarget = (UInt32)(*colorDataArgs)[2].num;

				ColorShift shiftedColor(selectedCG, selectedCG->colorSet[colorStart], selectedCG->colorSet[colorTarget], colorStart, colorTarget);

				auto iter = heatedWeapons.find(rWeap->refID);
				if (iter == heatedWeapons.end()) {
					auto pair = heatedWeapons.emplace(rWeap->refID, WeaponData(shiftedColor, heatData, rActor));
					iter = pair.first;
				}
				iter->second.heatData.HeatOnFire();

				HeatRGB blendedColor = shiftedColor.Shift(iter->second.heatData.heatVal, colorStart, colorTarget, selectedCG);
				for (size_t i = 0; i < nodeDataArgs->size(); ++i)
				{
					const char* overchargeBlockName = (*nodeDataArgs)[i].str; 
					iter->second.blockNames.emplace_back(overchargeBlockName); 
					SetEmissiveRGB(rActor, overchargeBlockName, blendedColor);
				}
			}
		}
		ThisStdCall<int>(originalAddress, rWeap, rActor);						//Plays original Actor::FireWeapon
		return;
	}

	void __fastcall FireWeaponWrapper(TESForm* rWeap, void* edx, TESObjectREFR* rActor)
	{
		if (PluginFunctions::pNVSE == true)
		{
			DevKitFork(rWeap, rActor);
		}
		else
		{
			ThisStdCall<int>(originalAddress, rWeap, rActor);						//Plays original Actor::FireWeapon
		}
	}

	void WeaponCooldown()                                                                               //Responsible for cooling a weapon down
    {
        if (!Overcharge::heatedWeapons.empty())															//If there are heating weapons...
        {
            for (auto it = Overcharge::heatedWeapons.begin(); it != Overcharge::heatedWeapons.end();)	//Iterates through vector containing all heating weapons 
            {
                if ((it->second.heatData.heatVal -= (g_timeGlobal->secondsPassed * it->second.heatData.cooldownRate)) <= it->second.heatData.baseHeatVal)			//Cools down heatVal by specified cooldown rate per second until starting heat level is reached
                {
                    g_isOverheated = 0;																	//When starting value is reached remove Overheated flag
                    it = Overcharge::heatedWeapons.erase(it);											//Remove weapon from vector containing heating weapons 
                }
                else
                {
					HeatRGB blendedColor = it->second.colorData.Shift(it->second.heatData.heatVal, it->second.colorData.startIndex, it->second.colorData.targetIndex, it->second.colorData.colorType);
					TESObjectREFR* actorRef = it->second.actorRef;
					for (const char* overchargeBlockName : it->second.blockNames) 
					{
						SetEmissiveRGB(actorRef, overchargeBlockName, blendedColor);
					}
                    ++it;																				//Move on to next heating weapons 
                }
            }
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
