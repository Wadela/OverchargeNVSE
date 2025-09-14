#include "OverchargeConfig.hpp"

namespace Overcharge
{
	case_insensitive_set extraModels{};
	case_insensitive_set definedModels{};

	std::unordered_map<UInt64, const HeatConfiguration>	weaponDataMap;
	OverchargeSettings g_OCSettings;

	void LoadConfigMain(const std::string& filePath)
	{
		CSimpleIniA ini;
		ini.SetUnicode();

		if (ini.LoadFile(filePath.c_str()) < 0)
		{
			Log() << std::format("Failed to load settings from '{}'", filePath);
			return;
		}

		//Global
		g_OCSettings.iEnableVisualEffects = ini.GetLongValue("Global", "iEnableVisualEffects", 1);
		g_OCSettings.iEnableGameplayEffects = ini.GetLongValue("Global", "iEnableGameplayEffects", 1);
		g_OCSettings.bEnableCustomAnimations = ini.GetBoolValue("Global", "bEnableCustomAnimations", true);
		g_OCSettings.bEnableCustomMeshes = ini.GetBoolValue("Global", "bEnableCustomMeshes", true);
		g_OCSettings.bEnableCustomSounds = ini.GetBoolValue("Global", "bEnableCustomSounds", true);

		//User Interface
		g_OCSettings.iHUDIndicator = ini.GetLongValue("User Interface", "iHUDIndicator", 1);
		g_OCSettings.fHUDScale = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDScale", 100.0));
		g_OCSettings.fHUDOffsetX = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDOffsetX", 0.0));
		g_OCSettings.fHUDOffsetY = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDOffsetY", 0.0));

		//Experimental
		g_OCSettings.bEnableAshPiles = ini.GetBoolValue("Experimental", "bEnableAshPiles", true);
		g_OCSettings.bEnableScriptedEffects = ini.GetBoolValue("Experimental", "bEnableScriptedEffects", true);

		//Load all ash pile entries
		if (g_OCSettings.iEnableVisualEffects == true)
		{
			for (int i = 0;; ++i)
			{
				std::string key = "sAshPile" + std::to_string(i);
				const char* val = ini.GetValue("Experimental", key.c_str(), nullptr);

				if (!val)
					break;

				definedModels.emplace(val);
			}
		}
	}

	void InitConfigModelPaths(TESObjectWEAP* rWeap)
	{
		if (!rWeap || !rWeap->projectile || !rWeap->impactDataSet) return;

		definedModels.insert(rWeap->projectile->kMuzzleFlash.kModel.StdStr());
		definedModels.insert(rWeap->projectile->kModel.StdStr());

		for (auto impact : rWeap->impactDataSet->impactDatas)
		{
			if (impact)
			definedModels.insert(impact->kModel.StdStr());
		}
	}

	void LoadWeaponConfigs(const std::string& filePath)
	{
		for (const auto& entry : std::filesystem::directory_iterator(filePath))
		{
			if (entry.path().extension() == ".ini")
			{
				std::string weapEID = entry.path().stem().string();

				TESForm* weapForm = TESForm::GetByID(weapEID.c_str());

				if (!weapForm || weapForm->eTypeID != TESForm::kType_TESObjectWEAP)
				{
					Log() << "Could not find form from editor ID: " << weapEID;
					continue;
				}

				UInt32 weapFID = weapForm->uiFormID;
				if (!weapFID)
				{
					Log() << "Could not find form ID: " << weapEID;
					continue;
				}

				TESObjectWEAP* rWeap = reinterpret_cast<TESObjectWEAP*>(weapForm);
				InitConfigModelPaths(rWeap);

				CSimpleIniA ini;
				ini.SetUnicode();
				if (ini.LoadFile(entry.path().string().c_str()) < 0)
				{
					Log() << std::format("Failed to load config: {}", entry.path().string());
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
					config.iMinAmmoUsed = ini.GetLongValue(secItem, "iMinAmmoUsed", 0);
					config.iMaxAmmoUsed = ini.GetLongValue(secItem, "iMaxAmmoUsed", 0);

					std::string OCEffectString = ini.GetValue(secItem, "sOverchargeEffects", "");
					UInt8 OCEffects = StringToFlags(OCEffectString, ',', OCEffectNames);
					config.iOverchargeEffect = OCEffects;

					config.iOverchargeEffectThreshold = ini.GetLongValue(secItem, "iOverchargeEffectThreshold", 0);
					config.iAddAmmoThreshold = ini.GetLongValue(secItem, "iAddAmmoThreshold", 0);
					config.iAddProjectileThreshold = ini.GetLongValue(secItem, "iAddProjectileThreshold", 0);
					config.iObjectEffectThreshold = ini.GetLongValue(secItem, "iObjectEffectThreshold", 0);

					std::string OCString = ini.GetValue(secItem, "sOverchargeFlags", "");
					UInt16 OCFlags = StringToFlags(OCString, ',', OCFlagNames);
					config.iOverchargeFlags = OCFlags;

					config.iMinDamage = ini.GetLongValue(secItem, "iMinDamage", 0);
					config.iMaxDamage = ini.GetLongValue(secItem, "iMaxDamage", 0);
					config.iMinCritDamage = ini.GetLongValue(secItem, "iMinCriticalDamage", 0);
					config.iMaxCritDamage = ini.GetLongValue(secItem, "iMaxCriticalDamage", 0);
					config.iMinProjectileSpeedPercent = ini.GetLongValue(secItem, "iMinProjectileSpeedPercent", 0);
					config.iMaxProjectileSpeedPercent = ini.GetLongValue(secItem, "iMaxProjectileSpeedPercent", 0);
					config.iMinProjectileSizePercent = ini.GetLongValue(secItem, "iMinProjectileSizePercent", 0);
					config.iMaxProjectileSizePercent = ini.GetLongValue(secItem, "iMaxProjectileSizePercent", 0);

					std::string objEffect = ini.GetValue(secItem, "sObjectEffectID", "");
					if (UInt32 effectID = TESForm::GetFormIDByEdID(objEffect.c_str()))
					{
						config.iObjectEffectID = effectID;
					}
					std::string minColor = ini.GetValue(secItem, "iMinColor", "0");  
					config.iMinColor = static_cast<UInt32>(std::stoul(minColor, nullptr, 16));
					std::string maxColor = ini.GetValue(secItem, "iMaxColor", "0");  
					config.iMaxColor = static_cast<UInt32>(std::stoul(maxColor, nullptr, 16));

					config.fHeatPerShot = ini.GetDoubleValue(secItem, "fHeatPerShot", 0.0);
					config.fCooldownPerSecond = ini.GetDoubleValue(secItem, "fCooldownPerSecond", 0.0);
					config.fMinFireRate = ini.GetDoubleValue(secItem, "fMinFireRate", 0.0);
					config.fMaxFireRate = ini.GetDoubleValue(secItem, "fMaxFireRate", 0.0);
					config.fMinAccuracy = ini.GetDoubleValue(secItem, "fMinAccuracy", 0.0);
					config.fMaxAccuracy = ini.GetDoubleValue(secItem, "fMaxAccuracy", 0.0);

					config.sAnimFile = "OCAnims\\";
					std::string animFile = ini.GetValue(secItem, "sAnimationFile", "");
					if (animFile.empty())
					{
						config.sAnimFile += "Overheat" + EnumToString(rWeap->eWeaponType, OCWeapTypeNames) + ".kf";
					}
					else config.sAnimFile += animFile;

					config.sHeatedNodes = ini.GetValue(secItem, "sHeatedNodes", "");

					std::string extraNodeString = ini.GetValue(secItem, "sExtraModels", "");
					auto extraNodes = SplitByDelimiter(extraNodeString, ',');
					for (std::string extraNode : extraNodes)
					{
						extraModels.insert(extraNode);
					}

					weaponDataMap.try_emplace(key, config);
				}
			}
		}
	}
}
