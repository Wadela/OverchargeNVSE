#pragma once

#include "MainHeader.hpp"
#include "Overcharge.hpp"
#include "OverchargeConfig.hpp"
#include "SafeWrite.hpp"
#include "NifOverride.hpp"


//Gamebryo
#include <NiPoint3.hpp>
#include <NiPointLight.hpp>
#include <NiPSysData.hpp>
#include <NiPSysVolumeEmitter.hpp>

//Bethesda
#include <BGSPrimitive.hpp>
#include <BSTempEffectParticle.hpp>
#include <TESEffectShader.hpp>
#include <Animation.hpp>
#include <MuzzleFlash.hpp>
#include <EffectSetting.hpp>
#include <MagicItemForm.hpp>

//#include <tracy/Tracy.hpp>

namespace Overcharge
{
	extern std::unordered_map<UInt64, std::shared_ptr<HeatData>>		activeWeapons;
	extern std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>	activeInstances;
	extern std::unordered_set<BSPSysSimpleColorModifier*>				colorModifiers;

	inline std::shared_ptr<HeatData> GetActiveHeat(const UInt32 actorID, const UInt32 weaponID)
	{
		auto it = activeWeapons.find(MakeHashKey(actorID, weaponID));
		return (it != activeWeapons.end()) ? it->second : nullptr;
	}

	inline std::shared_ptr<HeatData> GetOrCreateHeat(const UInt32 actorID, const UInt32 weaponID, const NiAVObjectPtr sourceNode, const HeatConfiguration& config)
	{
		const UInt64 key = MakeHashKey(actorID, weaponID);

		auto [it, inserted] = activeWeapons.try_emplace(key);
		if (inserted) {
			auto heat = std::make_shared<HeatData>(MakeHeatFromConfig(&config, sourceNode));
			heat->fx.matProp = NiMaterialProperty::CreateObject();
			it->second = std::move(heat);
		}
		it->second->data = &config;
		return it->second;
	}

	void WeaponCooldown();
}