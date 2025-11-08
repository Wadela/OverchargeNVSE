#pragma once

//Overcharge
#include "MainHeader.hpp"

//Bethesda
#include "TESObjectWEAP.hpp"
#include "BGSImpactDataSet.hpp"
#include "ModelLoader.hpp"
#include "BSWin32Audio.hpp"
#include "BGSPerk.hpp"

//Other Libraries
#include "libraries/SimpleINILibrary.h"


namespace Overcharge
{
	extern case_insensitive_set extraModels;
	extern case_insensitive_set definedModels;

	extern BGSPerk* OCPerkOverclocker;
	extern BGSPerk* OCPerkVoltageRegulator;
	extern BGSPerk* OCPerkGalvanicRelativist;
	extern BGSPerk* OCPerkCircuitBender;
	extern BGSPerk* OCPerkCriticalMass;
	extern BGSPerk* OCPerkCoolantLeak;
	extern BGSPerk* OCPerkThermicInversion;

	enum OCXAddons : UInt32
	{
		None			  = 0,
		OCXColor		  = 1 << 1,
		OCXParticle		  = 1 << 2,
		OCXOnFire		  = 1 << 3,
		OCXOnOverheat	  = 1 << 4,
		OCXOnOvercharge   = 1 << 5,
		OCXOnDelay		  = 1 << 6,
		OCXOnAltProj	  = 1 << 7,
		OCXOnInactive     = 1 << 8,
		OCXRotateX		  = 1 << 9,
		OCXRotateY		  = 1 << 10,
		OCXRotateZ		  = 1 << 11,
		OCXSpinX		  = 1 << 12,
		OCXSpinY		  = 1 << 13,
		OCXSpinZ		  = 1 << 14,
		OCXFlicker		  = 1 << 15,
		OCXNegative		  = 1 << 16,
		OCXCull		      = 1 << 17,
	};

	enum OCFlags : UInt16
	{
		OCFlags_None		  = 0,

		OCFlags_Meshes		  = 1 << 0,
		OCFlags_Animations	  = 1 << 1,
		OCFlags_Sounds		  = 1 << 2,
		OCFlags_Gameplay	  = 1 << 3,
		OCFlags_MuzzleFlash	  = 1 << 4,
		OCFlags_Projectile	  = 1 << 5,
		OCFlags_Impacts		  = 1 << 6,
		OCFlags_KillEffects	  = 1 << 7,
		OCFlags_AshPiles	  = 1 << 8,
		OCFlags_Perks		  = 1 << 9,

		OCFlags_All			  = 0xFFFF
	};

	enum OCEffect : UInt16
	{
		OCEffects_None			 = 0,

		OCEffects_Overheat		 = 1 << 0,
		OCEffects_Overcharge	 = 1 << 1,
		OCEffects_ChargeDelay	 = 1 << 2,
		OCEffects_AltProjectile  = 1 << 3,
		OCEffects_SelfDamage	 = 1 << 4,

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

	constexpr std::array<std::pair<UInt32, std::string_view>, 17> OCXAddonNames
	{ {
		{ OCXColor,				"color"	     },
		{ OCXParticle,			"particle"   },
		{ OCXOnFire,			"onfire"	 },
		{ OCXOnOverheat,		"onoverheat" },
		{ OCXOnOvercharge,		"oncharge"	 },
		{ OCXOnDelay,			"ondelay"	 },
		{ OCXOnAltProj,		    "onaltproj"	 },
		{ OCXOnInactive,		"oninactive" },
		{ OCXRotateX,			"rotateX"	 },
		{ OCXRotateY,			"rotatey"    },
		{ OCXRotateZ,			"rotatez"    },
		{ OCXSpinX,				"spinx"	     },
		{ OCXSpinY,				"spiny"		 },
		{ OCXSpinZ,				"spinz"		 },
		{ OCXFlicker,			"flicker"	 },
		{ OCXNegative,			"negative"	 },
		{ OCXCull,				"cull"		 }
	} };

	constexpr std::array<std::pair<UInt16, std::string_view>, 10> OCFlagNames
	{ {
		{ OCFlags_Meshes,        "meshes"		 },
		{ OCFlags_Animations,    "animations"	 },
		{ OCFlags_Sounds,        "Sounds"		 },
		{ OCFlags_Gameplay,      "gameplay"		 },
		{ OCFlags_MuzzleFlash,	 "muzzleflash"   },
		{ OCFlags_Projectile,    "Projectiles"   },
		{ OCFlags_Impacts,       "Impacts"		 },
		{ OCFlags_KillEffects,   "KillEffects"	 },
		{ OCFlags_AshPiles,      "piles"		 },
		{ OCFlags_Perks,		 "perks"		 }
	} };

	constexpr std::array<std::pair<UInt16, std::string_view>, 5> OCEffectNames
	{ {
		{ OCEffects_Overheat,		"Overheat"		},
		{ OCEffects_Overcharge,		"Overcharge"	},
		{ OCEffects_ChargeDelay,	"Delay"			},
		{ OCEffects_AltProjectile,  "AltProjectile"	},
		{ OCEffects_SelfDamage,		"SelfDamage"	}
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
		UInt8  iVisualEffects = 1;
		UInt8  iGameplayEffects = 1;

		bool   bMeshes = true;
		bool   bAnimations = true;
		bool   bSounds = true;
		bool   bPerks = true;

		float  fSkillLevelScaling = 0.35f;

		UInt8  iHUDIndicator = 1;
		float  fHUDScale = 100.0f;
		float  fHUDOffsetX = 0.0f;
		float  fHUDOffsetY = 0.0f;
	};

	struct HeatedNode
	{
		UInt32			index;
		UInt32			flags;
		NiFixedString	nodeName;
	};

	struct OCXNode
	{
		NiFixedString	targetParent;
		HeatedNode		extraNode;
		float			xNodeScale;
		NiPoint3		xNodeTranslate;
		NiPoint3		xNodeRotation;
	};

	struct HeatConfiguration
	{
		UInt8 iMinProjectiles = 0xFF;
		UInt8 iMaxProjectiles = 0xFF;
		UInt8 iMinAmmoUsed = 0xFF;
		UInt8 iMaxAmmoUsed = 0xFF;
		UInt8 iOverchargeEffectThreshold = 0; 
		UInt8 iAddAmmoThreshold = 0;
		UInt8 iAddProjectileThreshold = 0;
		UInt8 iObjectEffectThreshold = 0;

		UInt16 iOverchargeEffect = OCEffects_None;
		UInt16 iOverchargeFlags = OCFlags_None;

		UInt16 iMinDamage = 0xFFFF;
		UInt16 iMaxDamage = 0xFFFF;
		UInt16 iMinCritDamage = 0xFFFF;
		UInt16 iMaxCritDamage = 0xFFFF;
		UInt16 iMinProjectileSpeedPercent = 0xFFFF;
		UInt16 iMaxProjectileSpeedPercent = 0xFFFF;
		UInt16 iMinProjectileSizePercent = 0xFFFF;
		UInt16 iMaxProjectileSizePercent = 0xFFFF;

		UInt32 iAltProjectileID = 0xFFFFFF;
		UInt32 iObjectEffectID = 0xFFFFFF;
		UInt32 iMinColor = 0xFFFFFF;
		UInt32 iMaxColor = 0xFFFFFF;

		float fHeatPerShot = 0;
		float fCooldownPerSecond = 0;
		float fMinFireRate = -1;
		float fMaxFireRate = -1;
		float fMinAccuracy = -1;
		float fMaxAccuracy = -1;

		NiFixedString sAnimFile;
		NiFixedString sHeatSoundFile;
		NiFixedString sChargeSoundFile;

		std::vector<HeatedNode> sHeatedNodes;
	};

	extern std::unordered_map<UInt64, const HeatConfiguration> weaponDataMap;
	extern std::vector<OCXNode> OCExtraModels;
	extern OverchargeSettings g_OCSettings;


	void LoadConfigMain(const std::string& filePath);
	void InitConfigModelPaths(TESObjectWEAP* rWeap);
	void LoadWeaponConfigs(const std::string& filePath);
}

