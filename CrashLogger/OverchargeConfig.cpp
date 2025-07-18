#include "OverchargeConfig.hpp"

namespace Overcharge
{
	std::vector<std::string> SplitByDelimiter(const std::string& str, char delimiter)
	{
		std::vector<std::string> result;
		std::stringstream ss(str);
		std::string item;

		while (std::getline(ss, item, delimiter)) {
			size_t start = item.find_first_not_of(" \t");
			size_t end = item.find_last_not_of(" \t");
			if (start != std::string::npos && end != std::string::npos)
				result.push_back(item.substr(start, end - start + 1));
		}
		return result;
	}

	void LoadWeaponConfigs(const std::string& filePath)
	{
		for (const auto& entry : std::filesystem::directory_iterator(filePath))
		{
			if (entry.path().extension() == ".ini")
			{
				std::string weapEID = entry.path().stem().string();
				UInt32 weapFID = TESForm::GetFormIDByEdID(weapEID.c_str());
				if (!weapFID)
				{
					Log() << "Could not find editor ID: " << weapEID;
					continue;
				}

				CSimpleIniA ini;
				ini.SetUnicode();
				if (ini.LoadFile(entry.path().string().c_str()) < 0)
				{
					Log() << std::format("Failed to load config: {}", filePath);
					continue;
				}

				CSimpleIniA::TNamesDepend allSections;
				ini.GetAllSections(allSections);
				for (const auto& section : allSections)
				{
					const char* secItem = section.pItem;
					int index = std::stoi(secItem);
					HeatConfiguration config;

					std::string ammoEID = ini.GetValue(secItem, "sAmmoEditorID", "");
					UInt32 ammoFID = TESForm::GetFormIDByEdID(ammoEID.c_str());
					if (!ammoFID)
					{
						Log() << "Could not find editor ID: " << ammoEID << " in weapon " << weapEID;
						continue;
					}
					UInt64 key = MakeHashKey(weapFID, ammoFID);

					config.iWeaponType = ini.GetLongValue(secItem, "iWeaponType", 0);
					config.iMinProjectiles = ini.GetLongValue(secItem, "iMinProjectiles", 0);
					config.iMaxProjectiles = ini.GetLongValue(secItem, "iMaxProjectiles", 0);
					config.iAddProjectileThreshold = ini.GetLongValue(secItem, "iAddProjectileThreshold", 0);
					config.iMinAmmoUsed = ini.GetLongValue(secItem, "iMinAmmoUsed", 0);
					config.iMaxAmmoUsed = ini.GetLongValue(secItem, "iMaxAmmoUsed", 0);
					config.iAddAmmoThreshold = ini.GetLongValue(secItem, "iAddAmmoThreshold", 0);
					config.iHeatEffectThreshold = ini.GetLongValue(secItem, "iHeatEffectThreshold", 0);
					config.iOverchargeFlags = ini.GetLongValue(secItem, "iOverchargeFlags", 0);
					config.iHeatEffectFlags = ini.GetLongValue(secItem, "iHeatEffectFlags", 0);
					config.iMinDamage = ini.GetLongValue(secItem, "iMinDamage", 0);
					config.iMaxDamage = ini.GetLongValue(secItem, "iMaxDamage", 0);
					config.iMinCritDamage = ini.GetLongValue(secItem, "iMinCritDamage", 0);
					config.iMaxCritDamage = ini.GetLongValue(secItem, "iMaxCritDamage", 0);
					config.iMinProjectileSpeedPercent = ini.GetLongValue(secItem, "iMinProjectileSpeedPercent", 0);
					config.iMaxProjectileSpeedPercent = ini.GetLongValue(secItem, "iMaxProjectileSpeedPercent", 0);
					std::string value = ini.GetValue(secItem, "iMinColor", "0");  // default as string!
					config.iMinColor = static_cast<UInt32>(std::stoul(value, nullptr, 16));
					std::string value2 = ini.GetValue(secItem, "iMaxColor", "0");  // default as string!
					config.iMaxColor = static_cast<UInt32>(std::stoul(value2, nullptr, 16));
					config.fHeatPerShot = ini.GetDoubleValue(secItem, "fHeatPerShot", 0.0);
					config.fCooldownPerSecond = ini.GetDoubleValue(secItem, "fCooldownPerSecond", 0.0);
					config.fMinFireRate = ini.GetDoubleValue(secItem, "fMinFireRate", 0.0);
					config.fMaxFireRate = ini.GetDoubleValue(secItem, "fMaxFireRate", 0.0);
					config.fMinAccuracy = ini.GetDoubleValue(secItem, "fMinAccuracy", 0.0);
					config.fMaxAccuracy = ini.GetDoubleValue(secItem, "fMaxAccuracy", 0.0);
					config.sAnimFileName = ini.GetValue(secItem, "sAnimFileName", "");
					std::string nodes = ini.GetValue(secItem, "sHeatedNodes", "");
					config.sHeatedNodes = SplitByDelimiter(nodes, ',');

					auto [insertIt, inserted] = weaponDataMap.try_emplace(key, config);
				}
			}
		}
	}
}
