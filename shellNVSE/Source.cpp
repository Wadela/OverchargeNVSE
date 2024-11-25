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
	UInt32 originalFireAddr;			
	UInt32 originalEquipAddr;
	UInt32 originalProjAddr;
	UInt32 originalMuzzleAddr;
	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::Create();

	TESObjectREFR* projectile;
	Actor* projOwner;
	TESObjectWEAP* weap;

	std::unordered_map<UInt32, WeaponData> heatedWeapons;										//Vector containing all weapons that are currently heating up

	void SetEmissiveRGB(TESObjectREFR* actorRef, NiMaterialProperty* matProp, const char* blockName, HeatRGB blendedColor)	//Rewritten SetMaterialProperty function
	{
		if (NiNode* niNode = actorRef->GetNiNode())
		{
			if (NiAVObject* block = niNode->GetBlock(blockName))
			{
				((NiGeometry*)block)->materialProp = matProp; 
				((NiGeometry*)block)->materialProp->emissiveRGB.r = blendedColor.heatRed;
				((NiGeometry*)block)->materialProp->emissiveRGB.g = blendedColor.heatGreen;
				((NiGeometry*)block)->materialProp->emissiveRGB.b = blendedColor.heatBlue;
				((NiGeometry*)block)->materialProp->emitMult = 2.0;
			}
		}
	}

	void SetMuzzleRGB(NiNode* node, NiMaterialProperty* matProp, const char* blockName, HeatRGB blendedColor)	//Rewritten SetMaterialProperty function
	{
		if (NiAVObject* block = node->GetBlock(blockName))
		{
			((NiGeometry*)block)->materialProp = matProp;
			((NiGeometry*)block)->materialProp->emissiveRGB.r = blendedColor.heatRed;
			((NiGeometry*)block)->materialProp->emissiveRGB.g = blendedColor.heatGreen;
			((NiGeometry*)block)->materialProp->emissiveRGB.b = blendedColor.heatBlue;
			((NiGeometry*)block)->materialProp->emitMult = 2.0;
		}
	}
	void __declspec(naked) FireAnimDetour()
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

	void DevKitOnProj(TESObjectWEAP* rWeap, TESObjectREFR* projectile, TESObjectREFR* rActor)
	{
		auto it = heatedWeapons.find(rActor->refID);

		if (it == heatedWeapons.end())
		{
			AuxVector* colorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);
			AuxVector* heatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);
			AuxVector* nodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);

			if (!colorDataArgs || !heatDataArgs || !nodeDataArgs)
			{
				return;
			}
			if ((*colorDataArgs)[0].type == kRetnType_String)
			{
				const ColorGroup* selectedCG = ColorGroup::GetColorSet((*colorDataArgs)[0].str);
				if (selectedCG)
				{
					UInt32 colorStart = (UInt32)(*colorDataArgs)[1].num;
					UInt32 colorTarget = (UInt32)(*colorDataArgs)[2].num;
					ColorShift shiftedColor(selectedCG, selectedCG->colorSet[colorStart], selectedCG->colorSet[colorTarget], colorStart, colorTarget);

					WeaponHeat heatData = { (*heatDataArgs)[0].num, (*heatDataArgs)[1].num, (*heatDataArgs)[2].num };
					it = heatedWeapons.emplace(rActor->refID, WeaponData(rWeap, projectile, rActor, NiMaterialProperty::Create(), shiftedColor, heatData)).first;
					it->second.heatData.HeatOnFire();

					HeatRGB blendedColor = shiftedColor.Shift(it->second.heatData.heatVal, colorStart, colorTarget, selectedCG);
					for (size_t i = 0; i < nodeDataArgs->size(); ++i)
					{
						const char* overchargeBlockName = (*nodeDataArgs)[i].str;
						it->second.blockNames.emplace_back(overchargeBlockName);
						SetEmissiveRGB(rActor, it->second.matProperty, overchargeBlockName, blendedColor);
					}

				}
			}
		}
		else
		{
			auto& weaponData = it->second;
			//weaponData.heatData.HeatOnFire();
			HeatRGB blendedColor = weaponData.colorData.Shift(weaponData.heatData.heatVal, weaponData.colorData.startIndex, weaponData.colorData.targetIndex, weaponData.colorData.colorType);
			SetEmissiveRGB(projectile, it->second.matProperty, "CoreHot01:1", blendedColor);
			SetEmissiveRGB(projectile, it->second.matProperty, "pWisps01", blendedColor);
			SetEmissiveRGB(projectile, it->second.matProperty, "CoreWispyEnergy02:0", blendedColor);
			SetEmissiveRGB(projectile, it->second.matProperty, "Plane03:0", blendedColor);
			SetEmissiveRGB(projectile, it->second.matProperty, "LaserGeometry:0", blendedColor);
			SetEmissiveRGB(projectile, it->second.matProperty, "LaserGeometry:1", blendedColor);
		}
		return;
	}

	void DevKitOnMuzzleFlash(MuzzleFlash* muzzle, TESObjectWEAP* rWeap, TESObjectREFR* rActor)
	{
		auto it = heatedWeapons.find(rActor->refID);

		if (it == heatedWeapons.end())
		{
			AuxVector* colorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);
			AuxVector* heatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);
			AuxVector* nodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);

			if (!colorDataArgs || !heatDataArgs || !nodeDataArgs)
			{
				return;
			}
			if ((*colorDataArgs)[0].type == kRetnType_String)
			{
				const ColorGroup* selectedCG = ColorGroup::GetColorSet((*colorDataArgs)[0].str);
				if (selectedCG)
				{
					UInt32 colorStart = (UInt32)(*colorDataArgs)[1].num;
					UInt32 colorTarget = (UInt32)(*colorDataArgs)[2].num;
					ColorShift shiftedColor(selectedCG, selectedCG->colorSet[colorStart], selectedCG->colorSet[colorTarget], colorStart, colorTarget);

					WeaponHeat heatData = { (*heatDataArgs)[0].num, (*heatDataArgs)[1].num, (*heatDataArgs)[2].num };
					it = heatedWeapons.emplace(rActor->refID, WeaponData(rWeap, projectile, rActor, NiMaterialProperty::Create(), shiftedColor, heatData)).first;
					it->second.heatData.HeatOnFire();

					HeatRGB blendedColor = shiftedColor.Shift(it->second.heatData.heatVal, colorStart, colorTarget, selectedCG);
					for (size_t i = 0; i < nodeDataArgs->size(); ++i)
					{
						const char* overchargeBlockName = (*nodeDataArgs)[i].str;
						it->second.blockNames.emplace_back(overchargeBlockName);
						SetEmissiveRGB(rActor, it->second.matProperty, overchargeBlockName, blendedColor);
					}

				}
			}
		}
		else
		{
			auto& weaponData = it->second;
			weaponData.heatData.HeatOnFire();
			HeatRGB blendedColor = weaponData.colorData.Shift(weaponData.heatData.heatVal, weaponData.colorData.startIndex, weaponData.colorData.targetIndex, weaponData.colorData.colorType);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "Plane01:0", blendedColor);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "Plane02:0", blendedColor);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "CoreWispyEnergy02:0", blendedColor);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "CoreWispyEnergy03", blendedColor);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "HorizontalFlash06", blendedColor);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "BigGlow:0", blendedColor);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "PlaneBurst:0", blendedColor);
			SetMuzzleRGB(muzzle->spNode, it->second.matProperty, "PlaneBurst:1", blendedColor);
		}
		return;
	}

	/*void DevKitOnFire(TESObjectWEAP* rWeap, TESObjectREFR* rActor)
	{
		auto it = heatedWeapons.find(rActor->refID);

		if (it == heatedWeapons.end())
		{
			AuxVector* colorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);
			AuxVector* heatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);
			AuxVector* nodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);

			if (!colorDataArgs || !heatDataArgs || !nodeDataArgs)
			{
				ThisStdCall<int>(originalFireAddr, rWeap, rActor);
				return;
			}
			if ((*colorDataArgs)[0].type == kRetnType_String)
			{
				const ColorGroup* selectedCG = ColorGroup::GetColorSet((*colorDataArgs)[0].str);
				if (selectedCG)
				{
					UInt32 colorStart = (UInt32)(*colorDataArgs)[1].num;
					UInt32 colorTarget = (UInt32)(*colorDataArgs)[2].num;
					ColorShift shiftedColor(selectedCG, selectedCG->colorSet[colorStart], selectedCG->colorSet[colorTarget], colorStart, colorTarget);

					WeaponHeat heatData = { (*heatDataArgs)[0].num, (*heatDataArgs)[1].num, (*heatDataArgs)[2].num };
					it = heatedWeapons.emplace(rActor->refID, WeaponData(rWeap, rActor, NiMaterialProperty::Create(), shiftedColor, heatData)).first;
					it->second.heatData.HeatOnFire();

					HeatRGB blendedColor = shiftedColor.Shift(it->second.heatData.heatVal, colorStart, colorTarget, selectedCG);
					for (size_t i = 0; i < nodeDataArgs->size(); ++i)
					{
						const char* overchargeBlockName = (*nodeDataArgs)[i].str;
						it->second.blockNames.emplace_back(overchargeBlockName);
						SetEmissiveRGB(rActor, it->second.matProperty, overchargeBlockName, blendedColor);
					}
				}
			}
		}
		else
		{
			it->second.heatData.HeatOnFire();

		}
		ThisStdCall<int>(originalFireAddr, rWeap, rActor);
		return;
	}

	void DevKitOnReady(TESForm* rWeap, TESObjectREFR* rActor)
	{
		AuxVector* colorDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorData", nullptr, nullptr, 0);
		AuxVector* heatDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeHeatData", nullptr, nullptr, 0);
		AuxVector* nodeDataArgs = PluginFunctions::GetMemberVar(rWeap, "OverchargeColorNodes", nullptr, nullptr, 0);

		if (!colorDataArgs || !heatDataArgs || !nodeDataArgs) 
		{
			ThisStdCall<int>(originalEquipAddr, rActor);
			return;
		}
		if ((*colorDataArgs)[0].type == kRetnType_String)
		{
			const ColorGroup* selectedCG = ColorGroup::GetColorSet((*colorDataArgs)[0].str);
			if (selectedCG)
			{
				UInt32 colorStart = (UInt32)(*colorDataArgs)[1].num;
				UInt32 colorTarget = (UInt32)(*colorDataArgs)[2].num;
				ColorShift shiftedColor(selectedCG, selectedCG->colorSet[colorStart], selectedCG->colorSet[colorTarget], colorStart, colorTarget);

				WeaponHeat heatData = { (*heatDataArgs)[0].num, (*heatDataArgs)[1].num, (*heatDataArgs)[2].num };
				auto iter = heatedWeapons.find(rActor->refID);
				if (iter == heatedWeapons.end())
				{
					iter = heatedWeapons.emplace(rActor->refID, WeaponData(rWeap, rActor, NiMaterialProperty::Create(), shiftedColor, heatData)).first;
				}
				HeatRGB blendedColor = shiftedColor.Shift(iter->second.heatData.heatVal += 1, colorStart, colorTarget, selectedCG);
				for (size_t i = 0; i < nodeDataArgs->size(); ++i)
				{
					const char* overchargeBlockName = (*nodeDataArgs)[i].str;
					iter->second.blockNames.emplace_back(overchargeBlockName);
					SetEmissiveRGB(rActor, iter->second.matProperty, overchargeBlockName, blendedColor);
				}
			}
		}
		ThisStdCall<int>(originalEquipAddr, rActor);
		return;
	}*/

	void WeaponCooldown()                                                                   
	{
		for (auto it = Overcharge::heatedWeapons.begin(); it != Overcharge::heatedWeapons.end();)
		{
			auto& weaponData = it->second;
			float cooldownStep = g_timeGlobal->secondsPassed * it->second.heatData.cooldownRate;
			if ((weaponData.heatData.heatVal -= cooldownStep) <= weaponData.heatData.baseHeatVal)
			{
				g_isOverheated = 0;
				it = Overcharge::heatedWeapons.erase(it);
			}
			else
			{
				HeatRGB blendedColor = weaponData.colorData.Shift(weaponData.heatData.heatVal, weaponData.colorData.startIndex, weaponData.colorData.targetIndex, weaponData.colorData.colorType);
				for (const char* overchargeBlockName : weaponData.blockNames)
				{
					SetEmissiveRGB(weaponData.actorRef, weaponData.matProperty, overchargeBlockName, blendedColor);
				}

				++it;
			}
		}
	}

	void __fastcall FireWeaponWrapper(TESObjectWEAP* rWeap, void* edx, TESObjectREFR* rActor)
	{
		if (PluginFunctions::pNVSE == true)
		{
			//DevKitOnFire(rWeap, rActor); 
		}
		else
		{
			ThisStdCall<int>(originalFireAddr, rWeap, rActor);						//Plays original Actor::FireWeapon

		}
	}
	void __fastcall ProjRGB(TESObjectREFR* projectile, Actor* actor, TESObjectWEAP* weap)
	{
		if (PluginFunctions::pNVSE == true)
		{
			DevKitOnProj(weap, projectile, actor);
		}
		else
		{
			return;						//Plays original Actor::FireWeapon
		}
	}

	void __declspec(naked) ProjHook()
	{
		static UInt32 const retnAddr = 0x9BD52F;

		__asm
		{
			mov		ecx, [ebp - 0x14]	// liveProjectile (arg1)
			mov		edx, [ebp + 0xC]	// actor (arg2)
			mov		esi, [ebp + 0x14]	// weapon (arg3)
			push	esi
			call	ProjRGB

			// do the regular code
			pop		ecx
			pop		edi
			pop		esi
			mov		esp, ebp
			jmp		retnAddr
		}
	}

	void __fastcall EquipItemWrapper(TESObjectREFR* rActor, void* edx)  
	{
		TESForm* weapon = ThisStdCall<TESForm*>(originalEquipAddr, rActor); 
		if (PluginFunctions::pNVSE == true)
		{
			//DevKitOnReady(weapon, rActor);
		}
		else
		{
			ThisStdCall<int>(originalEquipAddr, rActor);
		}
	} 
	void __fastcall MuzzleFlashWrapper(MuzzleFlash* muzzleFlash)
	{
		if (PluginFunctions::pNVSE == true)
		{
			DevKitOnMuzzleFlash(muzzleFlash, muzzleFlash->pSourceWeapon, muzzleFlash->pSourceActor);
		}


		ThisStdCall<int>(originalMuzzleAddr, muzzleFlash);
	}

	void __fastcall ProjectileWrapper(void* a1, void* edx, TESObjectREFR* projectile) //void * ecx 
	{
		auto* ebp = GetParentBasePtr(_AddressOfReturnAddress(), false);
		auto ref = *reinterpret_cast<TESForm**>(ebp + 0xC);

		auto it = heatedWeapons.find(ref->refID);
		auto& weaponData = it->second;
		HeatRGB blendedColor = weaponData.colorData.Shift(weaponData.heatData.heatVal, weaponData.colorData.startIndex, weaponData.colorData.targetIndex, weaponData.colorData.colorType);
		SetEmissiveRGB(projectile, it->second.matProperty, "CoreHot01:1", blendedColor);
		SetEmissiveRGB(projectile, it->second.matProperty, "pWisps01", blendedColor);
		ThisStdCall<int>(originalProjAddr, a1, projectile); 
	}


	void InitHooks()
	{
		UInt32 actorFireAddr = 0x8BADE9; //0x8BADE9 Actor:FireWeapon
		UInt32 startFireAnim = 0x949CEA; //0x949CF1 Start Fire Animation
		UInt32 EquipItem = 0x95DCAC;	 //0x88CAEB Equip Item Function
		UInt32 readyWeapAddr = 0x8A5F93;
		UInt32 projectileData = 0x9BD52A;
		UInt32 Do3DLoadedAddr = 0x9B7CC0;
		UInt32 CreateProjectile = 0x9BD518;
		UInt32 CreateProjectile1 = 0x9BF22D;
		UInt32 AddProj = 0x9BD511;
		UInt32 Do3DLoaded = 0x9BDA10;
		UInt32 getProjBaseForm = 0x9B7CE1;
		UInt32 MuzzleFlashEnable = 0x9BB7CD; //MuzzleFlash::Enable

		//AppendToCallChain(readyWeapAddr, UInt32(EquipItemWrapper), originalEquipAddr);
		//AppendToCallChain(actorFireAddr, UInt32(FireWeaponWrapper), originalFireAddr);
		//AppendToCallChain(CreateProjectile, UInt32(ProjectileWrapper), originalProjAddr); 
		WriteRelJump(projectileData, UInt32(ProjHook)); 
		AppendToCallChain(MuzzleFlashEnable, UInt32(MuzzleFlashWrapper), originalMuzzleAddr);
		//WriteRelJump(startFireAnim, UInt32(FireAnimDetour));  
	} 
}
