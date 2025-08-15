#pragma once

#include "MainHeader.hpp"
#include "Overcharge.hpp"
#include "OverchargeHooks.hpp"
#include "OverchargeConfig.hpp"
#include <main.hpp>

//DEFINE_COMMAND_PLUGIN();

void Console_Print(const char* fmt, ...) {
	ConsoleManager* mgr = ConsoleManager::GetSingleton();
	if (mgr) 
	{
		va_list	args;
		va_start(args, fmt);
		CALL_MEMBER_FN(mgr, Print)(fmt, args);
		va_end(args);
	}
}

bool Cmd_GetHeatState_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESObjectWEAP* sourceWeap;
	Actor* sourceRef = (Actor*)thisObj;
	UInt32 type;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &type) && type > 0 && type <= 20)
	{
		std::shared_ptr<Overcharge::HeatData> heat =
			Overcharge::GetActiveHeat(sourceRef->uiFormID, sourceWeap->uiFormID);

		switch (type)
		{
		case 1:
			if (heat && heat->state.bIsOverheated)
				*result = heat->state.bIsOverheated;
			break;
		case 2:
			if (heat && heat->state.fHeatPerShot)
				*result = heat->state.fHeatPerShot;
			break;
		case 3:
			if (heat && heat->state.fCooldownRate)
				*result = heat->state.fCooldownRate;
			break;
		case 4:
			if (heat && heat->state.fHeatVal)
				*result = heat->state.fHeatVal;
			break;
		case 5:
			if (heat && heat->fx.currCol)
				*result = Overcharge::RGBtoUInt32(heat->fx.currCol);
			break;
		case 6:
			if (heat && heat->state.uiOCEffect)
				*result = heat->state.uiOCEffect;
			break;
		case 7:
			if (heat && heat->state.uiObjectEffectID)
				*result = heat->state.uiObjectEffectID;
			break;
		case 8:
			if (heat && heat->state.uiEnchThreshold)
				*result = heat->state.uiEnchThreshold;
			break;
		case 9:
			if (heat && heat->state.fProjectileSize)
				*result = heat->state.fProjectileSize;
			break;
		case 10:
			if (heat && heat->state.fProjectileSpeed)
				*result = heat->state.fProjectileSpeed;
			break;
		case 11:
			if (heat && heat->state.uiDamage)
				*result = heat->state.uiDamage;
			break;
		case 12:
			if (heat && heat->state.uiCritDamage)
				*result = heat->state.uiCritDamage;
			break;
		case 13:
			if (heat && heat->state.fFireRate)
				*result = heat->state.fFireRate;
			break;
		case 14:
			if (heat && heat->state.fAccuracy)
				*result = heat->state.fAccuracy;
			break;
		case 15:
			if (heat && heat->state.uiProjectiles)
				*result = heat->state.uiProjectiles;
			break;
		case 16:
			if (heat && heat->state.uiAmmoUsed)
				*result = heat->state.uiAmmoUsed;
			break;
		case 17:
			if (heat && heat->state.uiOCEffectThreshold)
				*result = heat->state.uiOCEffectThreshold;
			break;
		case 18:
			if (heat && heat->state.uiEnchThreshold)
				*result = heat->state.uiEnchThreshold;
			break;
		case 19:
			if (heat && heat->state.uiProjThreshold)
				*result = heat->state.uiProjThreshold;
			break;
		case 20:
			if (heat && heat->state.uiAmmoThreshold)
				*result = heat->state.uiAmmoThreshold;
			break;
		default:
			*result = 0;
			return true;
		}
		if (IsConsoleOpen())
			Console_Print("GetHeatState >> %d >> %f", type, *result);
	}
	return true;
}

bool Cmd_SetHeatState_Execute(COMMAND_ARGS)
{
	*result = 0;
	TESObjectWEAP* sourceWeap;
	Actor* sourceRef = (Actor*)thisObj;
	UInt32 type;
	float value;

	if (ExtractArgsEx(EXTRACT_ARGS_EX, &sourceWeap, &type, &value) && type > 0 && type <= 20)
	{
		const NiNodePtr sourceNode = sourceRef->Get3D();
		const TESAmmo* equippedAmmo = sourceWeap->GetEquippedAmmo(sourceRef);

		if (!sourceNode || !equippedAmmo || equippedAmmo->uiFormID) return true;

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
			heat->state.bIsOverheated = value;
			break;
		case 2:
			heat->state.fHeatPerShot = value;
			break;
		case 3:
			heat->state.fCooldownRate = value;
			break;
		case 4:
			heat->state.fHeatVal = value;
			break;
		case 5:
			heat->fx.currCol = Overcharge::UInt32toRGB(value);
			break;
		case 6:
			heat->state.uiOCEffect = value;
			break;
		case 7:
			heat->state.uiObjectEffectID = value;
			break;
		case 8:
			heat->state.uiEnchThreshold = value;
			break;
		case 9:
			heat->state.fProjectileSize = value;
			break;
		case 10:
			heat->state.fProjectileSpeed = value;
			break;
		case 11:
			heat->state.uiDamage = value;
			break;
		case 12:
			heat->state.uiCritDamage = value;
			break;
		case 13:
			heat->state.fFireRate = value;
			break;
		case 14:
			heat->state.fAccuracy = value;
			break;
		case 15:
			heat->state.uiProjectiles = value;
			break;
		case 16:
			heat->state.uiAmmoUsed = value;
			break;
		case 17:
			heat->state.uiOCEffectThreshold = value;
			break;
		case 18:
			heat->state.uiEnchThreshold = value;
			break;
		case 19:
			heat->state.uiProjThreshold = value;
			break;
		case 20:
			heat->state.uiAmmoThreshold = value;
			break;
		default:
			*result = 0;
			return true;
		}
		if (IsConsoleOpen())
			Console_Print("SetHeatState >> %d >> %f", type, *result);
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
			break;
		case 2:
			*result = config.iOverchargeEffect;
			break;
		case 3:
			resultString = config.sAnimFileName;
			g_stringInterface->Assign(PASS_COMMAND_ARGS, resultString.c_str());
			break;
		case 4:
			for (const auto& node : config.sHeatedNodes)
			resultString.append(node.c_str(), ', ');
			g_stringInterface->Assign(PASS_COMMAND_ARGS, resultString.c_str());
			break;
		case 5:
			*result = config.iWeaponType;
			break;
		case 6:
			*result = config.iMinProjectiles;
			break;
		case 7:
			*result = config.iMaxProjectiles;
			break;
		case 8:
			*result = config.iMaxAmmoUsed;
			break;
		case 9:
			*result = config.iOverchargeEffectThreshold;
			break; 
		case 10:
			*result = config.iAddAmmoThreshold;
			break;
		case 11:
			*result = config.iAddProjectileThreshold;
			break;
		case 12:
			*result = config.iObjectEffectThreshold;
			break;
		case 13:
			*result = config.iMinDamage;
			break;
		case 14:
			*result = config.iMaxDamage;
			break;
		case 15:
			*result = config.iMinCritDamage;
			break;
		case 16:
			*result = config.iMaxCritDamage;
			break;
		case 17:
			*result = config.iMinProjectileSpeedPercent;
			break;
		case 18:
			*result = config.iMaxProjectileSpeedPercent;
			break;
		case 19:
			*result = config.iMinProjectileSizePercent;
			break;
		case 20:
			*result = config.iMaxProjectileSizePercent;
			break;
		case 21:
			*result = config.iObjectEffectID;
			break;
		case 22:
			*result = config.iMinColor;
			break;
		case 23:
			*result = config.iMaxColor;
			break;
		case 24:
			*result = config.fHeatPerShot;
			break;
		case 25:
			*result = config.fCooldownPerSecond;
			break;
		case 26:
			*result = config.fMinFireRate;
			break;
		case 27:
			*result = config.fMaxFireRate;
			break;
		case 28:
			*result = config.fMinAccuracy;
			break;
		case 29:
			*result = config.fMaxAccuracy;
			break;
		default:
			*result = 0;
			return true;
		}
		if (type <= 4 && IsConsoleOpen())
		{
			Console_Print("GetHeatState >> %d >> %f", type, *result);
		}
		else if (type > 4 && IsConsoleOpen())
			Console_Print("GetHeatState >> %d >> %f", type, *result);
	}
	return true;
}
