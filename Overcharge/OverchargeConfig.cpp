#include "OverchargeConfig.hpp"

namespace Overcharge
{
	case_insensitive_set extraModels{};
	case_insensitive_set definedModels{};
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
		g_OCSettings.iVisualEffects = ini.GetLongValue("Global", "iEnableVisualEffects", 1);
		g_OCSettings.iGameplayEffects = ini.GetLongValue("Global", "iEnableGameplayEffects", 1);
		g_OCSettings.bMeshes = ini.GetBoolValue("Global", "bEnableCustomMeshes", true);
		g_OCSettings.bAnimations = ini.GetBoolValue("Global", "bEnableCustomAnimations", true);
		g_OCSettings.bSounds = ini.GetBoolValue("Global", "bEnableCustomSounds", true);
		g_OCSettings.fSkillLevelScaling = static_cast<float>(ini.GetDoubleValue("User Interface", "fSkillLevelScaling", 0.35));

		//User Interface
		g_OCSettings.iHUDIndicator = ini.GetLongValue("User Interface", "iHUDIndicator", 1);
		g_OCSettings.fHUDScale = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDScale", 100.0));
		g_OCSettings.fHUDOffsetX = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDOffsetX", 0.0));
		g_OCSettings.fHUDOffsetY = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDOffsetY", 0.0));

		//Load all extra model entries (i.e. Ash Piles)
		for (int i = 0;; ++i)
		{
			std::string key = "sExtraMesh" + std::to_string(i);
			const char* val = ini.GetValue("Extra", key.c_str(), nullptr);

			if (!val)
				break;

			definedModels.emplace(val);
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

	template <typename TMinMax>
	inline void ParseRange(std::string_view sv, TMinMax& minVal, TMinMax& maxVal, UInt8* threshold = nullptr)
	{
		auto parts = SplitByDelimiter(sv, ' ');

		if (!parts.empty())
			std::from_chars(parts[0].data(), parts[0].data() + parts[0].size(), minVal);

		if (parts.size() >= 2)
			std::from_chars(parts[1].data(), parts[1].data() + parts[1].size(), maxVal);

		if (threshold && parts.size() >= 3)
			*threshold = static_cast<UInt8>(ParseDelimitedData(parts[2], '(', ')'));
	}

	template <typename TMinMax>
	inline void ParseRangeFromINI(
		CSimpleIniA& ini, const char* sec, const char* key,
		const TMinMax& defaultMin, const TMinMax& defaultMax,
		TMinMax& minVal, TMinMax& maxVal,
		const UInt8* defaultThreshold = nullptr, UInt8* threshold = nullptr)
	{
		if (std::string_view val = ini.GetValue(sec, key, ""); !val.empty())
			ParseRange(val, minVal, maxVal, threshold);
		else
		{
			minVal = defaultMin;
			maxVal = defaultMax;
			if (threshold && defaultThreshold) 
				*threshold = *defaultThreshold;
		}
	}

	inline std::array<float, 3> ParseNiTransform(std::string_view str, char component)
	{
		std::array<float, 3> values = { 0.f, 0.f, 0.f };
		auto start = str.find(component);
		if (start == std::string_view::npos) return values;

		auto open = str.find('(', start);
		auto close = str.find(')', open);
		if (open == std::string_view::npos || close == std::string_view::npos || close <= open) return values;

		std::string_view sub = str.substr(open + 1, close - open - 1);
		auto parts = SplitByDelimiter(sub, ',');
		for (size_t i = 0; i < parts.size() && i < 3; ++i)
			std::from_chars(parts[i].data(), parts[i].data() + parts[i].size(), values[i]);

		return values;
	}

	void LoadConfigSection(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, TESObjectWEAP* rWeap, const HeatConfiguration* baseConfig = nullptr)
	{
		const HeatConfiguration& defaults = baseConfig ? *baseConfig : HeatConfiguration{};

		config.fHeatPerShot = ini.GetDoubleValue(secItem, "fHeatPerShot", defaults.fHeatPerShot);
		config.fCooldownPerSecond = ini.GetDoubleValue(secItem, "fCooldownPerSecond", defaults.fCooldownPerSecond);

		if (std::string_view OCEffectString = ini.GetValue(secItem, "sOverchargeEffects", ""); !OCEffectString.empty())
		{
			auto parts = SplitByDelimiter(OCEffectString, '(');
			if (!parts.empty())
			{
				auto flags = StringToFlags(parts[0], ' ', OCEffectNames);
				config.iOverchargeEffect = flags;
			}
			if (parts.size() >= 2)
				config.iOverchargeEffectThreshold = static_cast<UInt8>(ParseDelimitedData(parts[1], '\0', ')'));
		}

		if (std::string_view objEffect = ini.GetValue(secItem, "sObjectEffect", ""); !objEffect.empty())
		{
			auto parts = SplitByDelimiter(objEffect, '(');
			if (!parts.empty())
			{
				config.iObjectEffectID = parts[0].empty()
					? defaults.iObjectEffectID
					: TESForm::GetFormIDByEdID(parts[0].data());
			}
			if (parts.size() >= 2)
				config.iObjectEffectThreshold = static_cast<UInt8>(ParseDelimitedData(parts[1], '\0', ')'));
		}

		if (std::string_view altProj = ini.GetValue(secItem, "sAlternateProjectile", ""); !altProj.empty())
			config.iAltProjectileID = TESForm::GetFormIDByEdID(altProj.data());
		else config.iAltProjectileID = defaults.iAltProjectileID;

		ParseRangeFromINI(ini, secItem, "iAmmoConsumption", 
			defaults.iMinAmmoUsed, defaults.iMaxAmmoUsed, 
			config.iMinAmmoUsed, config.iMaxAmmoUsed, 
			&defaults.iAddAmmoThreshold, &config.iAddAmmoThreshold);

		ParseRangeFromINI(ini, secItem, "iProjectileCount", 
			defaults.iMinProjectiles, defaults.iMaxProjectiles,
			config.iMinProjectiles, config.iMaxProjectiles,
			&defaults.iAddProjectileThreshold, &config.iAddProjectileThreshold);

		ParseRangeFromINI(ini, secItem, "iDamage",
			defaults.iMinDamage, defaults.iMaxDamage,
			config.iMinDamage, config.iMaxDamage);

		ParseRangeFromINI(ini, secItem, "iCriticalDamage",
			defaults.iMinCritDamage, defaults.iMaxCritDamage,
			config.iMinCritDamage, config.iMaxCritDamage);

		ParseRangeFromINI(ini, secItem, "iProjectileSpeed",
			defaults.iMinProjectileSpeedPercent, defaults.iMaxProjectileSpeedPercent,
			config.iMinProjectileSpeedPercent, config.iMaxProjectileSpeedPercent);

		ParseRangeFromINI(ini, secItem, "iProjectileSize",
			defaults.iMinProjectileSizePercent, defaults.iMaxProjectileSizePercent,
			config.iMinProjectileSizePercent, config.iMaxProjectileSizePercent);

		ParseRangeFromINI(ini, secItem, "fFireRate",
			defaults.fMinFireRate, defaults.fMaxFireRate,
			config.fMinFireRate, config.fMaxFireRate);

		ParseRangeFromINI(ini, secItem, "fAccuracy",
			defaults.fMinAccuracy, defaults.fMaxAccuracy,
			config.fMinAccuracy, config.fMaxAccuracy);

		std::string_view colorVal = ini.GetValue(secItem, "iColor", "");
		if (!colorVal.empty()) {
			auto parts = SplitByDelimiter(colorVal, ' ');

			if (parts.size() >= 1)
			std::from_chars(parts[0].data(), parts[0].data() + parts[0].size(), config.iMinColor, 16);

			else config.iMinColor = defaults.iMinColor;

			if (parts.size() >= 2)
			std::from_chars(parts[1].data(), parts[1].data() + parts[1].size(), config.iMaxColor, 16);
			else config.iMaxColor = defaults.iMaxColor;
		}
		else 
		{
			config.iMinColor = defaults.iMinColor;
			config.iMaxColor = defaults.iMaxColor;
		}

		if (!baseConfig)
		{
			ini.GetLongValue(secItem, "iWeaponType", 0);

			std::string_view OCString{ ini.GetValue(secItem, "sOverchargeFlags", "") };
			config.iOverchargeFlags = !OCString.empty()
				? StringToFlags(OCString, ' ', OCFlagNames)
				: defaults.iOverchargeFlags;

			char animFileBuffer[128];
			std::string_view animFile = ini.GetValue(secItem, "sAnimationFile", "");
			std::string_view weapType = EnumToString(rWeap->eWeaponType, OCWeapTypeNames);
			if (animFile.empty()) {
				std::snprintf(animFileBuffer, sizeof(animFileBuffer), "OCAnims\\Overheat%.*s.kf",
					(int)weapType.size(), weapType.data());
			}
			else {
				std::snprintf(animFileBuffer, sizeof(animFileBuffer), "OCAnims\\%.*s",
					(int)animFile.size(), animFile.data());
			}
			config.sAnimFile = animFileBuffer;

			char blockKey[32];
			char xNodeKey[32];
			char xNodePath[128];
			char xTransKey[128];
			char tempName[64];

			for (int i = 0;; ++i)
			{
				std::snprintf(blockKey, sizeof(blockKey), "sHeatedBlock%d", i);
				std::string_view blockVal = ini.GetValue(secItem, blockKey, "");
				if (blockVal.empty()) break;

				auto parts = SplitByDelimiter(blockVal, '(');
				std::string_view nodeName = Trim(parts[0]);
				std::snprintf(tempName, sizeof(tempName), "%.*s", nodeName.size(), nodeName.data());

				UInt16 flags = 0;
				if (parts.size() >= 2)
				{
					std::string_view flagsStr = parts[1];

					if (!flagsStr.empty() && flagsStr.back() == ')')
						flagsStr.remove_suffix(1);

					flags = StringToFlags(flagsStr, ' ', OCXAddonNames);
				}

				NiFixedString fixed(rWeap->kModel.c_str());
				HeatedNode hNode({ 0xFFFF, flags, NiFixedString(tempName) });
				OCExtraModels.push_back({ fixed, hNode, 0.0f, (0, 0, 0), (0,0,0) });

				config.sHeatedNodes.push_back({ static_cast<UInt16>(i), flags, tempName});

				for (auto& modModel : rWeap->kModModels)
				{
					if (NiFixedString modFixed = modModel.kModel.c_str())
					{
						OCExtraModels.push_back({ modFixed, hNode, 0.0f, (0, 0, 0), (0,0,0) });
					}
				}
			}

			for (int i = 0;; ++i)
			{
				std::snprintf(xNodeKey, sizeof(xNodeKey), "sExtraNode%d", i);
				std::string_view nodeVal = ini.GetValue(secItem, xNodeKey, "");
				if (nodeVal.empty()) break;

				auto parts = SplitByDelimiter(nodeVal, '(');
				std::string_view nodeName = Trim(parts[0]);

				UInt16 flags = 0;
				if (parts.size() >= 2)
				{
					std::string_view flagsStr = parts[1];
					if (!flagsStr.empty() && flagsStr.back() == ')')
						flagsStr.remove_suffix(1);

					flags = StringToFlags(flagsStr, ' ', OCXAddonNames);
				}

				std::snprintf(xTransKey, sizeof(xTransKey), "xNodeTransform%d", i);
				std::string_view transVal = ini.GetValue(secItem, xTransKey, "");

				std::array<float, 3> translateVals = ParseNiTransform(transVal, 'T');
				NiPoint3 translate{ translateVals[0], translateVals[1], translateVals[2] };

				std::array<float, 3> rotateVals = ParseNiTransform(transVal, 'R');
				NiPoint3 rotate{ rotateVals[0], rotateVals[1], rotateVals[2] };

				std::array<float, 3> scaleVals = ParseNiTransform(transVal, 'S');
				float scale = scaleVals[0] != 0.f ? scaleVals[0] : 1.0f;

				std::snprintf(xNodePath, sizeof(xNodePath), "OCExtraMeshes\\%.*s", nodeName.size(), nodeName.data());

				HeatedNode hNode({ static_cast<UInt16>(i), flags, NiFixedString(xNodePath) });
				NiFixedString fixed(rWeap->kModel.c_str());

				OCExtraModels.push_back({ fixed, hNode, scale, translate, rotate });

				for (auto& modModel : rWeap->kModModels)
				{
					if (NiFixedString modFixed = modModel.kModel.c_str())
					{
						OCExtraModels.push_back({ modFixed, hNode, scale, translate, rotate });
					}
				}

				const char* dot = std::strrchr(nodeName.data(), '.');
				size_t len = dot ? static_cast<size_t>(dot - nodeName.data()) : nodeName.size();
				std::snprintf(tempName, sizeof(tempName), "%.*s%d", (int)len, nodeName.data(), i);
				config.sHeatedNodes.push_back({ static_cast<UInt16>(i), flags, tempName });
			}
		}
		else
		{
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
					const char* secName = section.pItem;
					if (_stricmp(secName, "Default") == 0) continue;

					HeatConfiguration config = hasBaseConfig ? baseConfig : HeatConfiguration{};
					LoadConfigSection(ini, secName, config, rWeap);

					UInt32 ammoFID = TESForm::GetFormIDByEdID(secName);
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
