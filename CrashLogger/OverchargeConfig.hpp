#pragma once

#include "SimpleINILibrary.h"
#include "MainHeader.hpp"
#include "TESObjectWEAP.hpp"
#include "BGSImpactDataSet.hpp"

namespace Overcharge
{
	extern case_insensitive_set extraModels;
	extern case_insensitive_set definedModels;

	enum OCFlags : UInt16
	{
		OCFlags_None		  = 0,

		OCFlags_Scripts		  = 1 << 0,
		OCFlags_Meshes		  = 1 << 1,
		OCFlags_Animations	  = 1 << 2,
		OCFlags_Sounds		  = 1 << 3,
		OCFlags_Gameplay	  = 1 << 4,
		OCFlags_EnchEffects	  = 1 << 5,
		OCFlags_MuzzleFlashes = 1 << 6,
		OCFlags_Projectiles	  = 1 << 7,
		OCFlags_Impacts		  = 1 << 8,
		OCFlags_KillEffects	  = 1 << 9,
		OCFlags_AshPiles	  = 1 << 10,
		OCFlags_WeapMods	  = 1 << 11,
		OCFlags_Perks		  = 1 << 12,

		OCFlags_All			  = 0xFFFF
	};

	enum OCEffect : UInt8
	{
		OCEffects_None			 = 0,

		OCEffects_Overheat		 = 1 << 0,
		OCEffects_Overcharge	 = 1 << 1,
		OCEffects_ChargeDelay	 = 1 << 2,
		OCEffects_WeapTypeEffect = 1 << 3,


		OCEffects_All			 = 0xFF
	};

	enum OCWeaps : UInt8
	{
		OCWeap_None = 0,
		OCWeap_Energy,
		OCWeap_Gun,
		OCWeap_Melee,
		OCWeap_Explosive,
		OCWeap_Thrown
	};

	constexpr std::array<std::pair<UInt16, std::string_view>, 14> OCFlagNames
	{ {
		{ OCFlags_Scripts,		 "Scripts"		 },
		{ OCFlags_Meshes,        "Meshes"		 },
		{ OCFlags_Animations,    "Animations"	 },
		{ OCFlags_Sounds,        "Sounds"		 },
		{ OCFlags_Gameplay,      "Gameplay"		 },
		{ OCFlags_EnchEffects,   "EnchEffects"	 },
		{ OCFlags_MuzzleFlashes, "MuzzleFlashes" },
		{ OCFlags_Projectiles,   "Projectiles"   },
		{ OCFlags_Impacts,       "Impacts"		 },
		{ OCFlags_KillEffects,   "KillEffects"	 },
		{ OCFlags_AshPiles,      "AshPiles"		 },
		{ OCFlags_WeapMods,      "WeapMods"		 },
		{ OCFlags_Perks,         "Perks"		 }
	} };

	constexpr std::array<std::pair<UInt8, std::string_view>, 6> OCEffectNames
	{ {
		{ OCEffects_Overheat,		"Overheat"	 },
		{ OCEffects_Overcharge,		"Overcharge" },
		{ OCEffects_ChargeDelay,	"Delay"		 },
		{ OCEffects_WeapTypeEffect, "Special"	 }
	} };

	constexpr std::array<std::pair<UInt8, std::string_view>, 15> OCWeapTypeNames
	{ {
		{ TESObjectWEAP::kWeapType_HandToHandMelee,		"h2h" },
		{ TESObjectWEAP::kWeapType_OneHandMelee,		"1hm" },
		{ TESObjectWEAP::kWeapType_TwoHandMelee,		"2hm" },
		{ TESObjectWEAP::kWeapType_OneHandPistol,       "1hp" },
		{ TESObjectWEAP::kWeapType_OneHandPistolEnergy, "1hp" },
		{ TESObjectWEAP::kWeapType_TwoHandRifle,		"2hr" },
		{ TESObjectWEAP::kWeapType_TwoHandAutomatic,	"2ha" },
		{ TESObjectWEAP::kWeapType_TwoHandRifleEnergy,  "2hr" },
		{ TESObjectWEAP::kWeapType_TwoHandHandle,		"2hh" },
		{ TESObjectWEAP::kWeapType_TwoHandLauncher,		"2hl" },
		{ TESObjectWEAP::kWeapType_OneHandGrenade,      "1gt" },
		{ TESObjectWEAP::kWeapType_OneHandMine,			"1md" },
		{ TESObjectWEAP::kWeapType_OneHandLunchboxMine, "1lm" },
		{ TESObjectWEAP::kWeapType_OneHandThrown,		"1ht" },
		{ TESObjectWEAP::kWeapType_Last,				"Unk" }
	} };

	struct OverchargeSettings
	{
		bool   bEnableCustomAnimations = true;
		bool   bEnableCustomMeshes = true;
		bool   bEnableCustomSounds = true;
		bool   bEnableAshPiles = true;
		bool   bEnableScriptedEffects = true;
		UInt8  iEnableVisualEffects = 1;
		UInt8  iEnableGameplayEffects = 1;
		UInt8  iHUDIndicator = 1;
		float  fHUDScale = 100.0f;
		float  fHUDOffsetX = 0.0f;
		float  fHUDOffsetY = 0.0f;
	};

	struct HeatConfiguration
	{
		UInt8 iWeaponType = OCWeap_None;
		UInt8 iMinProjectiles = 0;
		UInt8 iMaxProjectiles = 0;
		UInt8 iMinAmmoUsed = 0;
		UInt8 iMaxAmmoUsed = 0;
		UInt8 iOverchargeEffect = OCEffects_None;
		UInt8 iOverchargeEffectThreshold = 0; 
		UInt8 iAddAmmoThreshold = 0;
		UInt8 iAddProjectileThreshold = 0;
		UInt8 iObjectEffectThreshold = 0;

		UInt16 iOverchargeFlags = OCFlags_None;

		UInt16 iMinDamage = 0;
		UInt16 iMaxDamage = 0;
		UInt16 iMinCritDamage = 0;
		UInt16 iMaxCritDamage = 0;
		UInt16 iMinProjectileSpeedPercent = 0;
		UInt16 iMaxProjectileSpeedPercent = 0;
		UInt16 iMinProjectileSizePercent = 0;
		UInt16 iMaxProjectileSizePercent = 0;

		UInt32 iObjectEffectID = 0;
		UInt32 iMinColor = 0;
		UInt32 iMaxColor = 0;

		float fHeatPerShot = 0;
		float fCooldownPerSecond = 0;
		float fMinFireRate = 0;
		float fMaxFireRate = 0;
		float fMinAccuracy = 0;
		float fMaxAccuracy = 0;

		std::string sAnimFile;
		std::string sHeatedNodes;
	};

	extern std::unordered_map<UInt64, const HeatConfiguration> weaponDataMap;
	extern OverchargeSettings g_OCSettings;


	void LoadConfigMain(const std::string& filePath);
	void InitConfigModelPaths(TESObjectWEAP* rWeap);
	void LoadWeaponConfigs(const std::string& filePath);
}

