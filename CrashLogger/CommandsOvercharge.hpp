#pragma once

#include "MainHeader.hpp"
#include "Overcharge.hpp"
#include "OverchargeHooks.hpp"
#include "OverchargeConfig.hpp"
#include <main.hpp>

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

DEFINE_COMMAND_PLUGIN(GetHeatState, "", 1, 2, kParams_1Form_1Int);
DEFINE_COMMAND_PLUGIN(SetHeatState, "", 1, 3, kParams_1Form_1Int_1Float);
DEFINE_COMMAND_PLUGIN(GetHeatConfig, "", 0, 3, kParams_2Form_1Int);

bool Cmd_GetHeatState_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* sourceRef = (Actor*)thisObj;
	TESObjectWEAP* sourceWeap;
	std::string resultString;
	UInt32 type;
	UInt32 col;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &type) && type > 0 && type <= 19)
	{
		std::shared_ptr<Overcharge::HeatData> heat =
			Overcharge::GetActiveHeat(sourceRef->uiFormID, sourceWeap->uiFormID);

		if (!heat) type = 0;

		switch (type)
		{
		case 1:
			*result = heat->state.bIsWeaponLocked;
			break;
		case 2:
			*result = heat->state.fHeatVal;
			break;
		case 3:
			*result = heat->state.fCooldownRate;
			break;
		case 4:
			*result = heat->state.fHeatPerShot;
			break;
		case 5:
			*result = heat->state.uiDamage;
			break;
		case 6:
			*result = heat->state.uiCritDamage;
			break;
		case 7:
			*result = heat->state.fFireRate;
			break;
		case 8:
			*result = heat->state.fAccuracy;
			break;
		case 9:
			*result = heat->state.uiAmmoUsed;
			break;
		case 10:
			*result = heat->state.uiProjectiles;
			break;
		case 11:
			*result = heat->state.fProjectileSize;
			break;
		case 12:
			*result = heat->state.fProjectileSpeed;
			break;
		case 13:
			*result = heat->state.uiAmmoThreshold;
			break;
		case 14:
			*result = heat->state.uiProjThreshold;
			break;
		case 15:
			*result = heat->state.uiOCEffectThreshold;
			break;
		case 16:
			*result = heat->state.uiEnchThreshold;
			break;
		case 17:
			*result = heat->state.uiOCEffect;
			resultString = FlagsToString(heat->state.uiOCEffect, Overcharge::OCEffectNames);
			break;
		case 18:
			*(UInt32*)result = heat->state.uiObjectEffectID;
			break;
		case 19:
			col = Overcharge::RGBtoUInt32(heat->fx.currCol);
			col = ((col & 0xFF0000) >> 16) | (col & 0x00FF00) | ((col & 0x0000FF) << 16);
			*(UInt32*)result = col;
			break;
		default:
			*result = 0;
			return true;
		}
		if (type < 17 && IsConsoleOpen())
			Console_Print("GetHeatState >> %d >> %g", type, *result);
		else if (type > 17 && IsConsoleOpen())
			Console_Print("GetHeatState >> %d >> %X", type, *(UInt32*)result);
		else if (type == 17 && IsConsoleOpen())
			Console_Print("GetHeatState >> %d >> %g >> %s", type, *result, resultString.c_str());
	}
	return true;
}

bool Cmd_SetHeatState_Execute(COMMAND_ARGS)
{
	*result = 0;
	Actor* sourceRef = (Actor*)thisObj;
	TESObjectWEAP* sourceWeap;
	std::string resultString;
	UInt32 type;
	float value;
	UInt32 col;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &type, &value) && type > 0 && type <= 19)
	{
		const NiNodePtr sourceNode = sourceRef->Get3D();
		const TESAmmo* equippedAmmo = sourceWeap->GetEquippedAmmo(sourceRef);

		if (!sourceNode || !equippedAmmo || !equippedAmmo->uiFormID) return true;

		UInt64 configKey = MakeHashKey(sourceWeap->uiFormID, equippedAmmo->uiFormID);
		const auto dataIt = Overcharge::weaponDataMap.find(configKey);
		if (dataIt == Overcharge::weaponDataMap.end()) return true;
		*result = 1;

		std::shared_ptr<Overcharge::HeatData> heat =
			Overcharge::GetOrCreateHeat(
				sourceRef->uiFormID,
				sourceWeap->uiFormID,
				sourceNode.m_pObject,
				dataIt->second
			);

		switch (type)
		{
		case 1:
			heat->state.bIsWeaponLocked = value;
			break;
		case 2:
			heat->state.fHeatVal = value;
			break;
		case 3:
			heat->state.fCooldownRate = value;
			break;
		case 4:
			heat->state.fHeatPerShot = value;
			break;
		case 5:
			heat->state.uiDamage = static_cast<UInt16>(value);
			break;
		case 6:
			heat->state.uiCritDamage = static_cast<UInt16>(value);
			break;
		case 7:
			heat->state.fFireRate = value;
			break;
		case 8:
			heat->state.fAccuracy = value;
			break;
		case 9:
			heat->state.uiAmmoUsed = static_cast<UInt8>(value);
			break;
		case 10:
			heat->state.uiProjectiles = value;
			break;
		case 11:
			heat->state.fProjectileSize = value;
			break;
		case 12:
			heat->state.fProjectileSpeed = value;
			break;
		case 13:
			heat->state.uiAmmoThreshold = static_cast<UInt8>(value);
			break;
		case 14:
			heat->state.uiProjThreshold = static_cast<UInt8>(value);
			break;
		case 15:
			heat->state.uiOCEffectThreshold = static_cast<UInt8>(value);
			break;
		case 16:
			heat->state.uiEnchThreshold = static_cast<UInt8>(value);
			break;
		case 17:
			heat->state.uiOCEffect = static_cast<UInt8>(value);
			resultString = FlagsToString(heat->state.uiOCEffect, Overcharge::OCEffectNames);
			break;
		case 18:
			heat->state.uiObjectEffectID = static_cast<UInt32>(value);
			break;
		case 19:
			col = static_cast<UInt32>(value);
			col = ((col & 0xFF0000) >> 16) | (col & 0x00FF00) | ((col & 0x0000FF) << 16);
			col = Overcharge::UInt32toRGB(col);
			heat->fx.currCol = col;
			break;
		default:
			*result = 0;
			return true;
		}
		if (type = 17 && IsConsoleOpen())
			Console_Print("SetHeatState >> %d >> %g", type, value);
		else if (type > 17 && IsConsoleOpen())
			Console_Print("SetHeatState >> %d >> %X", type, (UInt32)value);
		else if (type == 17 && IsConsoleOpen())
			Console_Print("SetHeatState >> %d >> %s", type, resultString.c_str());
	}
	return true;
}

bool Cmd_GetHeatConfig_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESObjectWEAP* sourceWeap;
	TESAmmo* sourceAmmo;
	UInt32 type;
	std::string resultString;
	UInt32 col; 

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &sourceAmmo, &type) && type > 0 && type <= 29)
	{
		if (!sourceWeap || !sourceAmmo) return true;

		UInt64 configKey = MakeHashKey(sourceWeap->uiFormID, sourceAmmo->uiFormID);
		const auto dataIt = Overcharge::weaponDataMap.find(configKey);
		if (dataIt == Overcharge::weaponDataMap.end()) return true;

		const auto& config = dataIt->second;

		switch (type)
		{
		case 1:
			*result = config.iOverchargeFlags;
			resultString = FlagsToString(config.iOverchargeFlags, Overcharge::OCFlagNames);
			break;
		case 2:
			*result = config.iOverchargeEffect;
			resultString = FlagsToString(config.iOverchargeEffect, Overcharge::OCEffectNames);
			break;
		case 3:
			resultString = config.sHeatedNodes;
			break;
		case 4:
			*result = config.iWeaponType;
			break;
		case 5:
			*result = config.fCooldownPerSecond;
			break;
		case 6:
			*result = config.fHeatPerShot;
			break;
		case 7:
			*result = config.iMinDamage;
			break;
		case 8:
			*result = config.iMaxDamage;
			break; 
		case 9:
			*result = config.iMinCritDamage;
			break;
		case 10:
			*result = config.iMaxCritDamage;
			break;
		case 11:
			*result = config.fMinFireRate;
			break;
		case 12:
			*result = config.fMaxFireRate;
			break;
		case 13:
			*result = config.fMinAccuracy;
			break;
		case 14:
			*result = config.fMaxAccuracy;
			break;
		case 15:
			*result = config.iMinAmmoUsed;
			break;
		case 16:
			*result = config.iMaxAmmoUsed;
			break;
		case 17:
			*result = config.iMinProjectiles;
			break;
		case 18:
			*result = config.iMaxProjectiles;
			break;
		case 19:
			*result = config.iMinProjectileSizePercent;
			break;
		case 20:
			*result = config.iMaxProjectileSizePercent;
			break;
		case 21:
			*result = config.iMinProjectileSpeedPercent;
			break;
		case 22:
			*result = config.iMaxProjectileSpeedPercent;
			break;
		case 23:
			*result = config.iAddAmmoThreshold;
			break;
		case 24:
			*result = config.iAddProjectileThreshold;
			break;
		case 25:
			*result = config.iOverchargeEffectThreshold;
			break;
		case 26:
			*result = config.iObjectEffectThreshold;
			break;
		case 27:
			*(UInt32*)result = config.iObjectEffectID;
			break;
		case 28:
			*(UInt32*)result = config.iMinColor;
			break;
		case 29:
			*(UInt32*)result = config.iMaxColor;
			break;
		default:
			*result = 0;
			return true;
		}
		if (type <= 3 && IsConsoleOpen())
			Console_Print("GetHeatConfig >> %d >> %s", type, resultString.c_str());
		else if (type > 26 && IsConsoleOpen())
			Console_Print("GetHeatConfig >> %d >> %X", type, *(UInt32*)result);
		else if (type > 3 && type <= 26 && IsConsoleOpen())
			Console_Print("GetHeatConfig >> %d >> %g", type, *result);
	}
	return true;
}
