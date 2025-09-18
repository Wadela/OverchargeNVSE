#include "OverchargeConfig.hpp"

namespace Overcharge
{
	case_insensitive_set extraModels{};
	case_insensitive_set definedModels{};
	std::unordered_set<NiNode*>	extraNiNodes;
	std::vector<OCXNode> OCExtraModels;

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

			for (int i = 0;; ++i)
			{
				std::string key = "sExtraModel" + std::to_string(i);
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

	inline std::string MakeFlaggedPrefix(UInt32 flags, const std::string& name, bool stripExtension = false)
	{
		if (flags == 0)
			return {};

		std::string baseName = name;
		if (stripExtension)
		{
			size_t dot = baseName.find_last_of('.');
			if (dot != std::string::npos)
				baseName = baseName.substr(0, dot);
		}

		return "[" + std::to_string(flags) + "]" + baseName;
	}

	void LoadConfigSection(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, TESObjectWEAP* rWeap, const HeatConfiguration* baseConfig = nullptr)
	{
		const HeatConfiguration& defaults = baseConfig ? *baseConfig : HeatConfiguration{};

		config.iMinProjectiles = ini.GetLongValue(secItem, "iMinProjectiles", defaults.iMinProjectiles);
		config.iMaxProjectiles = ini.GetLongValue(secItem, "iMaxProjectiles", defaults.iMaxProjectiles);
		config.iMinAmmoUsed = ini.GetLongValue(secItem, "iMinAmmoUsed", defaults.iMinAmmoUsed);
		config.iMaxAmmoUsed = ini.GetLongValue(secItem, "iMaxAmmoUsed", defaults.iMaxAmmoUsed);

		std::string OCEffectString = ini.GetValue(secItem, "sOverchargeEffects", "");
		config.iOverchargeEffect = !OCEffectString.empty()
			? StringToFlags(OCEffectString, ',', OCEffectNames)
			: defaults.iOverchargeEffect;

		config.iOverchargeEffectThreshold = ini.GetLongValue(secItem, "iOverchargeEffectThreshold", defaults.iOverchargeEffectThreshold);
		config.iAddAmmoThreshold = ini.GetLongValue(secItem, "iAddAmmoThreshold", defaults.iAddAmmoThreshold);
		config.iAddProjectileThreshold = ini.GetLongValue(secItem, "iAddProjectileThreshold", defaults.iAddProjectileThreshold);
		config.iObjectEffectThreshold = ini.GetLongValue(secItem, "iObjectEffectThreshold", defaults.iObjectEffectThreshold);

		config.iMinDamage = ini.GetLongValue(secItem, "iMinDamage", defaults.iMinDamage);
		config.iMaxDamage = ini.GetLongValue(secItem, "iMaxDamage", defaults.iMaxDamage);
		config.iMinCritDamage = ini.GetLongValue(secItem, "iMinCriticalDamage", defaults.iMinCritDamage);
		config.iMaxCritDamage = ini.GetLongValue(secItem, "iMaxCriticalDamage", defaults.iMaxCritDamage);

		config.iMinProjectileSpeedPercent = ini.GetLongValue(secItem, "iMinProjectileSpeedPercent", defaults.iMinProjectileSpeedPercent);
		config.iMaxProjectileSpeedPercent = ini.GetLongValue(secItem, "iMaxProjectileSpeedPercent", defaults.iMaxProjectileSpeedPercent);
		config.iMinProjectileSizePercent = ini.GetLongValue(secItem, "iMinProjectileSizePercent", defaults.iMinProjectileSizePercent);
		config.iMaxProjectileSizePercent = ini.GetLongValue(secItem, "iMaxProjectileSizePercent", defaults.iMaxProjectileSizePercent);

		std::string objEffect = ini.GetValue(secItem, "sObjectEffectID", "");
		config.iObjectEffectID = objEffect.empty() ? defaults.iObjectEffectID
			: TESForm::GetFormIDByEdID(objEffect.c_str());

		std::string minColor = ini.GetValue(secItem, "iMinColor", "");
		config.iMinColor = !minColor.empty() ? static_cast<UInt32>(std::stoul(minColor, nullptr, 16))
			: defaults.iMinColor;

		std::string maxColor = ini.GetValue(secItem, "iMaxColor", "");
		config.iMaxColor = !maxColor.empty() ? static_cast<UInt32>(std::stoul(maxColor, nullptr, 16))
			: defaults.iMaxColor;

		config.fHeatPerShot = ini.GetDoubleValue(secItem, "fHeatPerShot", defaults.fHeatPerShot);
		config.fCooldownPerSecond = ini.GetDoubleValue(secItem, "fCooldownPerSecond", defaults.fCooldownPerSecond);
		config.fMinFireRate = ini.GetDoubleValue(secItem, "fMinFireRate", defaults.fMinFireRate);
		config.fMaxFireRate = ini.GetDoubleValue(secItem, "fMaxFireRate", defaults.fMaxFireRate);
		config.fMinAccuracy = ini.GetDoubleValue(secItem, "fMinAccuracy", defaults.fMinAccuracy);
		config.fMaxAccuracy = ini.GetDoubleValue(secItem, "fMaxAccuracy", defaults.fMaxAccuracy);

		if (!baseConfig)
		{
			config.iWeaponType = ini.GetLongValue(secItem, "iWeaponType", defaults.iWeaponType);

			std::string OCString = ini.GetValue(secItem, "sOverchargeFlags", "");
			config.iOverchargeFlags = !OCString.empty()
				? StringToFlags(OCString, ',', OCFlagNames)
				: defaults.iOverchargeFlags;

			config.sAnimFile = "OCAnims\\";
			std::string animFile = ini.GetValue(secItem, "sAnimationFile", "");
			if (animFile.empty())
				config.sAnimFile += "Overheat" + EnumToString(rWeap->eWeaponType, OCWeapTypeNames) + ".kf";
			else
				config.sAnimFile += animFile;

			for (int i = 0;; ++i) {
				std::string blockKey = "sHeatedBlock" + std::to_string(i);
				std::string blockName = ini.GetValue(secItem, blockKey.c_str(), "");
				if (blockName.empty()) break;

				std::string flagsKey = "hNodeFlags" + std::to_string(i);
				std::string flagsVal = ini.GetValue(secItem, flagsKey.c_str(), "");
				UInt32 flags = StringToFlags(flagsVal, ',', OCXAddonNames);

				std::string blockFinal = MakeFlaggedPrefix(flags, blockName);
				if (!blockFinal.empty()) {
					if (!config.sHeatedNodes.empty())
						config.sHeatedNodes.append(", ");
					config.sHeatedNodes.append(blockFinal);
				}
			}

			for (int i = 0;; ++i) {
				std::string nodeKey = "sExtraNode" + std::to_string(i);
				std::string nodeName = ini.GetValue(secItem, nodeKey.c_str(), "");
				if (nodeName.empty()) break;

				std::string flagsKey = "xNodeFlags" + std::to_string(i);
				std::string flagsVal = ini.GetValue(secItem, flagsKey.c_str(), "");
				UInt32 flags = StringToFlags(flagsVal, ',', OCXAddonNames);

				std::string transKey = "xNodeTranslate" + std::to_string(i);
				std::string transVal = ini.GetValue(secItem, transKey.c_str(), "0,0,0");
				auto transParts = SplitByDelimiter(transVal, ',');
				NiPoint3 translate{ 0.0f, 0.0f, 0.0f };
				if (transParts.size() >= 3) {
					translate.x = std::stof(transParts[0]);
					translate.y = std::stof(transParts[1]);
					translate.z = std::stof(transParts[2]);
				}

				std::string rotKey = "xNodeRotate" + std::to_string(i);
				std::string rotVal = ini.GetValue(secItem, rotKey.c_str(), "0,0,0");
				auto rotParts = SplitByDelimiter(rotVal, ',');
				NiPoint3 rotation{ 0.0f, 0.0f, 0.0f };
				if (rotParts.size() >= 3) {
					rotation.x = std::stof(rotParts[0]);
					rotation.y = std::stof(rotParts[1]);
					rotation.z = std::stof(rotParts[2]);
				}

				std::string scaleKey = "xNodeScale" + std::to_string(i);
				float scaleVal = ini.GetDoubleValue(secItem, scaleKey.c_str(), 1.0f);

				OCXNode nodeConfig;
				nodeConfig.extraNode = "[" + std::to_string(i) + "]" + "OCExtraMeshes\\" + nodeName;
				nodeConfig.targetParent = std::make_unique<BSString>(rWeap->kModel);
				nodeConfig.xNodeTranslate = translate;
				nodeConfig.xNodeRotation = rotation;
				nodeConfig.xNodeScale = scaleVal;
				nodeConfig.flags = flags;

				OCExtraModels.push_back(std::move(nodeConfig));

				std::string flaggedName = MakeFlaggedPrefix(flags, nodeName, true);
				if (!flaggedName.empty()) {
					if (!config.sHeatedNodes.empty())
						config.sHeatedNodes.append(", ");
					config.sHeatedNodes.append(flaggedName + std::to_string(i));
				}
			}
		}
		else
		{
			config.iWeaponType = baseConfig->iWeaponType;
			config.iOverchargeFlags = baseConfig->iOverchargeFlags;
			config.sAnimFile = baseConfig->sAnimFile;
			config.sHeatedNodes = baseConfig->sHeatedNodes;
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

				HeatConfiguration baseConfig{};
				bool hasBaseConfig = false;

				if (ini.GetSection("Default"))
				{
					LoadConfigSection(ini, "Default", baseConfig, rWeap);
					hasBaseConfig = true;

					UInt64 key = MakeHashKey(weapFID, 0); 
					weaponDataMap.try_emplace(key, baseConfig);
				}

				CSimpleIniA::TNamesDepend allSections;
				ini.GetAllSections(allSections);
				for (const auto& section : allSections)
				{
					std::string secName = section.pItem;
					if (_stricmp(secName.c_str(), "Default") == 0) continue;

					HeatConfiguration config = hasBaseConfig ? baseConfig : HeatConfiguration{};
					LoadConfigSection(ini, secName.c_str(), config, rWeap);

					UInt32 ammoFID = TESForm::GetFormIDByEdID(secName.c_str());
					if (!ammoFID)
					{
						Log() << "Unable to locate Ammo Editor ID: " << secName;
						continue;
					}

					UInt64 key = MakeHashKey(weapFID, ammoFID);
					weaponDataMap.try_emplace(key, config);
				}
			}
		}
	}
}
