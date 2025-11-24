#pragma once

#include "MainHeader.hpp"
#include "Overcharge.hpp"
#include "OverchargeHooks.hpp"
#include "OverchargeConfig.hpp"
#include "main.hpp"

void Console_Print(const char* fmt, ...) 
{
	ConsoleManager* mgr = ConsoleManager::GetSingleton();
	if (mgr)
	{
		va_list	args;
		va_start(args, fmt);
		CALL_MEMBER_FN(mgr, Print)(fmt, args);
		va_end(args);
	}
}

static ParamInfo kParams_1Int[1] = 
{
	"Integer", kParamType_Integer, 0
};

static ParamInfo kParams_1Form_1Int[2] =
{
	{"Form",    kParamType_AnyForm, 0},
	{"Integer", kParamType_Integer, 0}
};

static ParamInfo kParams_1Form_1Int_1Float[3] =
{
	{"Form",    kParamType_AnyForm, 0},
	{"Integer", kParamType_Integer, 0},
	{"Float",	kParamType_Float,   0}
};

static ParamInfo kParams_2Form_1Int[3] =
{
	{"Form 1",  kParamType_AnyForm, 0},
	{"Form 2",  kParamType_AnyForm, 0},
	{"Integer",	kParamType_Integer, 0}
};

DEFINE_COMMAND_PLUGIN(GetOCWeaponState, "", 1, 2, kParams_1Form_1Int);
DEFINE_COMMAND_PLUGIN(SetOCWeaponState, "", 1, 3, kParams_1Form_1Int_1Float);
DEFINE_COMMAND_PLUGIN(GetOCWeaponConfig, "", 0, 3, kParams_2Form_1Int);
DEFINE_COMMAND_PLUGIN(GetOCSettings, "", 0, 1, kParams_1Int);


bool Cmd_GetOCWeaponState_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* sourceRef = (Actor*)thisObj;
	TESObjectWEAP* sourceWeap;
	std::string resultString;
	UInt32 type;
	UInt32 col;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &type) && type > 0 && type <= 23)
	{
		std::shared_ptr<Overcharge::HeatData> heat =
			Overcharge::GetActiveHeat(sourceRef->uiFormID, sourceWeap->uiFormID);

		if (!heat) type = 0;

		switch (type)
		{
		case 1:
			*result = heat->state.bIsActive;
			break;
		case 2:
			*result = heat->state.iCanOverheat;
			break;
		case 3:
			*result = heat->state.uiAmmoUsed;
			break;
		case 4:
			*result = heat->state.uiProjectiles;
			break;
		case 5:
			*result = heat->state.uiAmmoThreshold;
			break;
		case 6:
			*result = heat->state.uiProjThreshold;
			break;
		case 7:
			*result = heat->state.uiEnchThreshold;
			break;
		case 8:
			*result = heat->state.uiOCEffectThreshold;
			break;
		case 9:
			*result = heat->state.uiDamage;
			break;
		case 10:
			*result = heat->state.uiCritDamage;
			break;
		case 11:
			*result = heat->state.uiTicksPassed;
			break;
		case 12:
			*result = heat->state.uiOCEffect;
			resultString = FlagsToString(heat->state.uiOCEffect, Overcharge::OCEffectNames);
			break;
		case 13:
			*(UInt32*)result = heat->state.uiObjectEffectID;
			break;
		case 14:
			*result = heat->state.fAccuracy;
			break;
		case 15:
			*result = heat->state.fFireRate;
			break;
		case 16:
			*result = heat->state.fProjectileSpeed;
			break;
		case 17:
			*result = heat->state.fProjectileSize;
			break;
		case 18:
			*result = heat->state.fHeatPerShot;
			break;
		case 19:
			*result = heat->state.fCooldownRate;
			break;
		case 20:
			*result = heat->state.fHeatVal;
			break;
		case 21:
			*result = heat->state.fStartingVal;
			break;
		case 22:
			*result = heat->state.fTargetVal;
			break;
		case 23:
			col = Overcharge::RGBtoUInt32(heat->fx.currCol);
			col = ((col & 0xFF0000) >> 16) | (col & 0x00FF00) | ((col & 0x0000FF) << 16);
			*(UInt32*)result = col;
			break;
		default:
			*result = 0;
			return true;
		}
		if (type == 12 && IsConsoleOpen())
			Console_Print("GetOCWeaponState >> %d >> %g >> %s", type, *result, resultString.c_str());
		else if (type == 13 && IsConsoleOpen())
			Console_Print("GetOCWeaponState >> %d >> %X", type, *(UInt32*)result);
		else if (IsConsoleOpen())
			Console_Print("GetOCWeaponState >> %d >> %g", type, *result);
	}
	return true;
}

bool Cmd_SetOCWeaponState_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* sourceRef = (Actor*)thisObj;
	TESObjectWEAP* sourceWeap;
	std::string resultString;
	UInt32 type;
	float value;
	UInt32 col;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &type, &value) && type > 0 && type <= 23)
	{
		const TESAmmo* equippedAmmo = sourceWeap->GetEquippedAmmo(sourceRef);

		if (!equippedAmmo || !equippedAmmo->uiFormID) return true;

		UInt64 configKey = MakeHashKey(sourceWeap->uiFormID, equippedAmmo->uiFormID);
		const auto dataIt = Overcharge::weaponDataMap.find(configKey);
		if (dataIt == Overcharge::weaponDataMap.end()) return true;
		*result = 1;

		std::shared_ptr<Overcharge::HeatData> heat =
			Overcharge::GetOrCreateHeat(
				sourceRef
			);

		switch (type)
		{
		case 1:
			heat->state.bIsActive = static_cast<bool>(value);
			break;
		case 2:
			heat->state.iCanOverheat = static_cast<UInt8>(value);
			break;
		case 3:
			heat->state.uiAmmoUsed = static_cast<UInt8>(value);
			break;
		case 4:
			heat->state.uiProjectiles = static_cast<UInt8>(value);
			break;
		case 5:
			heat->state.uiAmmoThreshold = static_cast<UInt8>(value);
			break;
		case 6:
			heat->state.uiProjThreshold = static_cast<UInt8>(value);
			break;
		case 7:
			heat->state.uiEnchThreshold = static_cast<UInt8>(value);
			break;
		case 8:
			heat->state.uiOCEffectThreshold = static_cast<UInt8>(value);
			break;
		case 9:
			heat->state.uiDamage = static_cast<UInt16>(value);
			break;
		case 10:
			heat->state.uiCritDamage = static_cast<UInt16>(value);
			break;
		case 11:
			heat->state.uiTicksPassed = static_cast<UInt16>(value);
			break;
		case 12:
			heat->state.uiOCEffect = static_cast<UInt16>(value);
			resultString = FlagsToString(heat->state.uiOCEffect, Overcharge::OCEffectNames);
			break;
		case 13:
			heat->state.uiObjectEffectID = static_cast<UInt32>(value);
			break;
		case 14:
			heat->state.fAccuracy = value;
			break;
		case 15:
			heat->state.fFireRate = value;
			break;
		case 16:
			heat->state.fProjectileSpeed = value;
			break;
		case 17:
			heat->state.fProjectileSize = value;
			break;
		case 18:
			heat->state.fHeatPerShot = value;
			break;
		case 19:
			heat->state.fCooldownRate = value;
			break;
		case 20:
			heat->state.fHeatVal = value;
			break;
		case 21:
			heat->state.fStartingVal = value;
			break;
		case 22:
			heat->state.fTargetVal = value;
			break;
		case 23:
			col = static_cast<UInt32>(value);
			col = ((col & 0xFF0000) >> 16) | (col & 0x00FF00) | ((col & 0x0000FF) << 16);
			col = Overcharge::UInt32toRGB(col);
			heat->fx.currCol = col;
			break;
		default:
			*result = 0;
			return true;
		}
		if (type == 12 && IsConsoleOpen())
			Console_Print("SetOCWeaponState >> %d >> %s", type, resultString.c_str());
		else if (type == 13 && IsConsoleOpen())
			Console_Print("SetOCWeaponState >> %d >> %X", type, (UInt32)value);
		else if (IsConsoleOpen())
			Console_Print("SetOCWeaponState >> %d >> %g", type, value);
	}
	return true;
}

bool Cmd_GetOCWeaponConfig_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESObjectWEAP* sourceWeap;
	TESAmmo* sourceAmmo;
	UInt32 type;
	std::string resultString;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &sourceAmmo, &type) && type > 0 && type <= 32)
	{
		if (!sourceWeap || !sourceAmmo) return true;

		UInt64 configKey = MakeHashKey(sourceWeap->uiFormID, sourceAmmo->uiFormID);
		const auto dataIt = Overcharge::weaponDataMap.find(configKey);
		if (dataIt == Overcharge::weaponDataMap.end()) return true;

		const auto& config = dataIt->second;

		switch (type)
		{
		case 1:
			*result = config.iMinAmmoUsed;
			break;
		case 2:
			*result = config.iMaxAmmoUsed;
			break;
		case 3:
			*result = config.iMinProjectiles;
			break;
		case 4:
			*result = config.iMaxProjectiles;
			break;
		case 5:
			*result = config.iAddAmmoThreshold;
			break;
		case 6:
			*result = config.iAddProjectileThreshold;
			break;
		case 7:
			*result = config.iObjectEffectThreshold;
			break;
		case 8:
			*result = config.iOverchargeEffectThreshold;
			break;
		case 9:
			*result = config.iOverchargeEffect;
			resultString = FlagsToString(config.iOverchargeEffect, Overcharge::OCEffectNames);
			break;
		case 10:
			*result = config.iOverchargeFlags;
			resultString = FlagsToString(config.iOverchargeFlags, Overcharge::OCFlagNames);
			break;
		case 11:
			*(UInt32*)result = config.iAltProjectileID;
			break;
		case 12:
			*(UInt32*)result = config.iObjectEffectID;
			break;
		case 13:
			*(UInt32*)result = config.iMinColor;
			break;
		case 14:
			*(UInt32*)result = config.iMaxColor;
			break;
		case 15:
			*result = config.fMinDamage;
			break;
		case 16:
			*result = config.fMaxDamage;
			break;
		case 17:
			*result = config.fMinCritDamage;
			break;
		case 18:
			*result = config.fMaxCritDamage;
			break;
		case 19:
			*result = config.fMinFireRate;
			break;
		case 20:
			*result = config.fMaxFireRate;
			break;
		case 21:
			*result = config.fMinSpread;
			break;
		case 22:
			*result = config.fMaxSpread;
			break;
		case 23:
			*result = config.fMinProjectileSpeed;
			break;
		case 24:
			*result = config.fMaxProjectileSpeed;
			break;
		case 25:
			*result = config.fMinProjectileSize;
			break;
		case 26:
			*result = config.fMaxProjectileSize;
			break;
		case 27:
			*result = config.fHeatPerShot;
			break;
		case 28:
			*result = config.fCooldownPerSecond;
			break;
		case 29:
			resultString = config.sAnimFile.c_str();
			g_stringInterface->Assign(PASS_COMMAND_ARGS, resultString.c_str());
			break;
		case 30:
			resultString = config.sHeatSoundFile.c_str();
			g_stringInterface->Assign(PASS_COMMAND_ARGS, resultString.c_str());
			break;
		case 31:
			resultString = config.sHeatSoundFile.c_str();
			g_stringInterface->Assign(PASS_COMMAND_ARGS, resultString.c_str());
			break;
		case 32:
			resultString.clear();
			resultString.reserve(config.sHeatedNodes.size() * 32);
			for (size_t i = 0; i < config.sHeatedNodes.size(); ++i)
			{
				const auto& [index, flags, node] = config.sHeatedNodes[i];
				if (i > 0) resultString += ",";
				char buf[256];
				std::snprintf(buf, sizeof(buf), "[%X]%s", flags, node.c_str());
				resultString += buf;
			}
			g_stringInterface->Assign(PASS_COMMAND_ARGS, resultString.c_str());
			break;
		default:
			*result = 0;
			return true;
		}
		if (IsConsoleOpen())
		{
			if (type == 9 || type == 10 || type >= 29)
				Console_Print("GetOCWeaponConfig >> %d >> %s", type, resultString.c_str());
			else if (type >= 11 && type <= 14)
				Console_Print("GetOCWeaponConfig >> %d >> %X", type, *(UInt32*)result);
			else if (type <= 8 || (type >= 15 && type <= 28))
				Console_Print("GetOCWeaponConfig >> %d >> %g", type, *result);
		}
	}
	return true;
}

bool Cmd_GetOCSettings_Execute(COMMAND_ARGS)
{
	*result = 0;
	UInt32 type;
	std::string resultString;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &type) && type > 0 && type <= 12)
	{
		switch (type)
		{
		case 1:
			*result = Overcharge::g_OCSettings.bMeshes;
			break;
		case 2:
			*result = Overcharge::g_OCSettings.bAnimations;
			break;
		case 3:
			*result = Overcharge::g_OCSettings.bSounds;
			break;
		case 4:
			*result = Overcharge::g_OCSettings.bStats;
			break;
		case 5:
			*result = Overcharge::g_OCSettings.bMechanics;
			break;
		case 6:
			*result = Overcharge::g_OCSettings.bVFX;
			break;
		case 7:
			*result = Overcharge::g_OCSettings.bOverheatLockout;
			break;
		case 8:
			*result = Overcharge::g_OCSettings.fSkillLevelScaling;
			break;
		case 9:
			*result = Overcharge::g_OCSettings.iHUDStyle;
			break;
		case 10:
			*result = Overcharge::g_OCSettings.fHUDScale;
			break;
		case 11:
			*result = Overcharge::g_OCSettings.fHUDOffsetX;
			break;
		case 12:
			*result = Overcharge::g_OCSettings.fHUDOffsetY;
			break;
		default:
			*result = 0;
			return true;
		}
		if (IsConsoleOpen()) Console_Print("GetOCSettings >> %d >> %g", type, *result);
	}
	return true;
}
