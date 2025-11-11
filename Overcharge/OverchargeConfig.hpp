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
#include <DirectXPackedVector.h>

namespace Overcharge
{
	using float16 = DirectX::PackedVector::HALF;

	constexpr UInt8 INVALID_U8 = 0xFF;
	constexpr UInt16 INVALID_U16 = 0xFFFF;
	constexpr UInt32 INVALID_U32 = 0xFFFFFFFF;
	constexpr float16 INVALID_F16 = 0xFFFF;
	constexpr float INVALID_F32 = -1.0f;


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
		OCXColor		  = 1 << 0,
		OCXParticle		  = 1 << 1,
		OCXOnFire		  = 1 << 2,
		OCXOnOverheat	  = 1 << 3,
		OCXOnOvercharge   = 1 << 4,
		OCXOnDelay		  = 1 << 5,
		OCXOnAltProj	  = 1 << 6,
		OCXOnHolster      = 1 << 7,
		OCXRotateX		  = 1 << 8,
		OCXRotateY		  = 1 << 9,
		OCXRotateZ		  = 1 << 10,
		OCXSpinX		  = 1 << 11,
		OCXSpinY		  = 1 << 12,
		OCXSpinZ		  = 1 << 13,
		OCXFlicker		  = 1 << 14,
		OCXNegative		  = 1 << 15,
		OCXCull		      = 1 << 16,
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
		{ OCXOnHolster,		    "onholster"  },
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
		UInt8 iAddAmmoThreshold				= 0;
		UInt8 iAddProjectileThreshold		= 0;
		UInt8 iObjectEffectThreshold		= 0;
		UInt8 iOverchargeEffectThreshold	= 0;

		UInt8 iMinAmmoUsed					= INVALID_U8;
		UInt8 iMaxAmmoUsed					= INVALID_U8;
		UInt8 iMinProjectiles				= INVALID_U8;
		UInt8 iMaxProjectiles				= INVALID_U8;

		UInt16 iOverchargeEffect			= OCEffects_None;
		UInt16 iOverchargeFlags				= OCFlags_None;

		UInt32 iAltProjectileID				= INVALID_U32;
		UInt32 iObjectEffectID				= INVALID_U32;
		UInt32 iMinColor					= INVALID_U32;
		UInt32 iMaxColor					= INVALID_U32;

		float fMinDamage					= INVALID_F32;
		float fMaxDamage					= INVALID_F32;
		float fMinCritDamage				= INVALID_F32;
		float fMaxCritDamage				= INVALID_F32;
		float fMinFireRate					= INVALID_F32;
		float fMaxFireRate					= INVALID_F32;
		float fMinSpread					= INVALID_F32;
		float fMaxSpread					= INVALID_F32;
		float fMinProjectileSpeed			= INVALID_F32;
		float fMaxProjectileSpeed			= INVALID_F32;
		float fMinProjectileSize			= INVALID_F32;
		float fMaxProjectileSize			= INVALID_F32;

		float fHeatPerShot					= INVALID_F32;
		float fCooldownPerSecond			= INVALID_F32;

		NiFixedString sAnimFile;
		NiFixedString sHeatSoundFile;
		NiFixedString sChargeSoundFile;

		std::vector<HeatedNode> sHeatedNodes;
	};

	extern std::unordered_map<UInt64, const HeatConfiguration> weaponDataMap;
	extern std::vector<OCXNode> OCExtraModels;
	extern OverchargeSettings g_OCSettings;



	void ParseGameData(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, const HeatConfiguration& defaults);
	void ParseOverchargeData(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, const HeatConfiguration& defaults);
	void ParseAssetData(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, TESObjectWEAP* rWeap);

	void LoadConfigMain(const std::string& filePath);
	void InitConfigModelPaths(TESObjectWEAP* rWeap);
	void LoadWeaponConfigs(const std::string& filePath);
}

