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
	UInt32 originalImpactAddr;
	UInt32 originalGetImpactAddr;
	NiMaterialProperty* g_customPlayerMatProperty = NiMaterialProperty::Create();
	NiMaterialProperty* g_customActorMatProperty = NiMaterialProperty::Create();
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
			SetEmissiveRGB(projectile, it->second.matProperty, "pShockTrail", blendedColor);
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

	void __fastcall ProjectileWrapper(NiAVObject* a1, void* edx, Projectile* projectile) //void * ecx 
	{
		//auto* ebp = GetParentBasePtr(_AddressOfReturnAddress(), false);
		//auto ref = *reinterpret_cast<TESForm**>(ebp + 0xC);

		//auto it = heatedWeapons.find(ref->refID);
		//auto& weaponData = it->second;
		//HeatRGB blendedColor = weaponData.colorData.Shift(weaponData.heatData.heatVal, weaponData.colorData.startIndex, weaponData.colorData.targetIndex, weaponData.colorData.colorType);
		//SetEmissiveRGB(projectile, it->second.matProperty, "CoreHot01:1", blendedColor);
		//SetEmissiveRGB(projectile, it->second.matProperty, "pWisps01", blendedColor);
		//ThisStdCall<int>(originalProjAddr, a1, projectile); 
		return;
	}

	BSTempEffectParticle* __cdecl hkTempEffectParticle(void* a1, float a2, char* a3, NiPoint3 a4, NiPoint3 a5, float a6, char a7, void* a8) {
		auto result = CdeclCall<BSTempEffectParticle*>(0x6890B0, a1, a2, a3, a4, a5, a6, a7, a8);
		// do shit
		NiObjectNET* result1 = result->spParticleObject;
		NiNode* resultNode = result1->GetNiNode();

		SetMuzzleRGB(resultNode, NiMaterialProperty::Create(), "Plane01:0", { 0.0f, 0.0f, 1.0f });
		SetMuzzleRGB(resultNode, NiMaterialProperty::Create(), "pRingImpact", { 0.0f, 0.0f, 1.0f });
		SetMuzzleRGB(resultNode, NiMaterialProperty::Create(), "pEnergyHit", { 0.0f, 0.0f, 1.0f });
		return result;
	}

	void __cdecl hk_AddMasterParticleAddonNodes(NiNode* node) {
		CdeclCall(0x578060, node);
		// Process after addon nodes are created
		auto& children = node->m_children;
		for (int i = 1; i < children.capacity; i++) {
			if (auto child = children.Get(i)) {
				if (auto childNode = (BSValueNode*)child) {
					NiNode* childNode1 = childNode->GetNiNode();
					NiAVObject* childBlock = childNode1->GetBlock("pShockTrail");
					//((NiGeometry*)childBlock)->materialProp->emissiveRGB = NiColor(0, 0, 1);
				}
			}
		}
		return;
	}
	/*void __fastcall SpawnImpactWrapper(Projectile* projectile, void* edx, TESObjectREFR* a2, NiPoint3* aCoord, NiPoint3* a4, int _330, unsigned __int32 Material_1)
	{
		ThisStdCall<int>(originalImpactAddr, projectile, a2, aCoord, a4, _330, Material_1);
		const ListNode<Projectile::ImpactData>* traverse = projectile->impactDataList.Head();
	}*/

	void __fastcall InitNewParticle(NiParticleSystem* system, void* edx, int newParticle)
	{
		if (system->GetNiGeometry() && strcmp(system->m_pcName, "pShockTrail") == 0) {

			if (auto matprop = system->GetNiGeometry()->materialProp) {
				matprop->emissiveRGB = NiColor(0, 0, 1);
				matprop->emitMult = 1.2f;
			}
		}

		ThisStdCall(0xC1AEE0, system, newParticle);
	}

	TESObjectREFR* __cdecl GetImpact(NiObjectNET* apObject)
	{
		return ThisStdCall<TESObjectREFR*>(originalGetImpactAddr, apObject);
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
		UInt32 SpawnImpactAddr = 0x9C2058;	//Projectile::SpawnCollisionEffects @ Projectile::ProcessImpacts
		UInt32 ImpactAddr = 0x9C2617;
		UInt32 BGSTempFXAddr = 0x9C2AC3;
		UInt32 someParticleAddr = 0xC247D9;
		WriteRelCall(CreateProjectile, UInt32(ProjectileWrapper)); 
		WriteRelCall(0x9C2AC3, UInt32(hkTempEffectParticle));
		WriteRelCall(0xC2237A, UInt32(InitNewParticle)); 
		WriteRelCall(0x9BE07A, UInt32(hk_AddMasterParticleAddonNodes));
		//AppendToCallChain(readyWeapAddr, UInt32(EquipItemWrapper), originalEquipAddr);
		//AppendToCallChain(actorFireAddr, UInt32(FireWeaponWrapper), originalFireAddr);
		//AppendToCallChain(CreateProjectile, UInt32(ProjectileWrapper), originalProjAddr); 
		//WriteRelJump(projectileData, UInt32(ProjHook)); 
		//AppendToCallChain(ImpactAddr, UInt32(GetImpact), originalGetImpactAddr);
		AppendToCallChain(MuzzleFlashEnable, UInt32(MuzzleFlashWrapper), originalMuzzleAddr);
		//AppendToCallChain(BGSTempFXAddr, UInt32(SpawnImpactWrapper), originalImpactAddr);
		//WriteRelJump(startFireAnim, UInt32(FireAnimDetour));  
	} 
}
