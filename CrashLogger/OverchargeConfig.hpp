#pragma once

#include "SimpleINILibrary.h"
#include "MainHeader.hpp"
#include "TESForm.hpp"

namespace Overcharge
{
	std::vector<std::string> SplitByDelimiter(const std::string& str, char delimiter);

	struct HeatConfiguration
	{
		UInt8 iWeaponType = 0;
		UInt8 iMinProjectiles = 0;
		UInt8 iMaxProjectiles = 0;
		UInt8 iAddProjectileThreshold = 0;
		UInt8 iMinAmmoUsed = 0;
		UInt8 iMaxAmmoUsed = 0;
		UInt8 iAddAmmoThreshold = 0;
		UInt8 iHeatEffectThreshold = 0;

		UInt16 iOverchargeFlags = 0;
		UInt16 iHeatEffectFlags = 0;
		UInt16 iMinDamage = 0;
		UInt16 iMaxDamage = 0;
		UInt16 iMinCritDamage = 0;
		UInt16 iMaxCritDamage = 0;
		UInt16 iMinProjectileSpeedPercent = 0;
		UInt16 iMaxProjectileSpeedPercent = 0;
		UInt16 iMinProjectileSizePercent = 0;
		UInt16 iMaxProjectileSizePercent = 0;

		UInt32 iMinColor = 0;
		UInt32 iMaxColor = 0;

		float fHeatPerShot = 0;
		float fCooldownPerSecond = 0;
		float fMinFireRate = 0;
		float fMaxFireRate = 0;
		float fMinAccuracy = 0;
		float fMaxAccuracy = 0;

		std::string sAnimFileName;

		std::vector<std::string> sHeatedNodes;
	};

	extern std::unordered_map<UInt64, HeatConfiguration> weaponDataMap;

	void LoadWeaponConfigs(const std::string& filePath);
}

