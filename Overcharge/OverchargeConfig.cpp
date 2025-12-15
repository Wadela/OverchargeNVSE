#include "OverchargeConfig.hpp"

namespace Overcharge
{
	case_insensitive_set definedModels{};
	std::vector<OCXNode> OCExtraModels;
	std::vector<std::pair<NiFixedString, NiFixedString>> OCDeferredModels;
	std::unordered_map<UInt64, const HeatConfiguration>	weaponDataMap;
	OverchargeSettings g_OCSettings;

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
		if (open == std::string_view::npos) return values;
		std::string_view sub = str.substr(open + 1);

		auto parts = SplitByDelimiter(sub, ',');
		for (size_t i = 0; i < parts.size() && i < 3; ++i) {
			std::string tmp(parts[i]);
			values[i] = std::strtof(tmp.c_str(), nullptr);
		}

		return values;
	}

	void LoadConfigMain(const std::string& filePath)
	{
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetMultiKey();

		if (ini.LoadFile(filePath.c_str()) < 0)
		{
			Log() << std::format("Failed to load settings from '{}'", filePath);
			return;
		}

		//Global
		g_OCSettings.bMeshes = ini.GetBoolValue("Global", "bEnableMeshes", true);
		g_OCSettings.bAnimations = ini.GetBoolValue("Global", "bEnableAnimations", true);
		g_OCSettings.bSounds = ini.GetBoolValue("Global", "bEnableSounds", true);
		g_OCSettings.bStats = ini.GetBoolValue("Global", "bEnableStats", true);
		g_OCSettings.bMechanics = ini.GetBoolValue("Global", "bEnableMechanics", true);
		g_OCSettings.bVFX = ini.GetBoolValue("Global", "bEnableVFX", true);
		g_OCSettings.bOverheatLockout = ini.GetBoolValue("Global", "bOverheatLockout", true);
		g_OCSettings.fSkillLevelScaling = static_cast<float>(ini.GetDoubleValue("User Interface", "fSkillLevelScaling", 0.25));

		//User Interface
		g_OCSettings.iHUDStyle = ini.GetLongValue("User Interface", "iHUDStyle", 1);
		g_OCSettings.fHUDScale = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDScale", 100.0));
		g_OCSettings.fHUDOffsetX = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDOffsetX", 0.0));
		g_OCSettings.fHUDOffsetY = static_cast<float>(ini.GetDoubleValue("User Interface", "fHUDOffsetY", 0.0));

		//Load all extra mesh entries (i.e. Ash Piles)
		CSimpleIniA::TNamesDepend extraMeshes;
		ini.GetAllValues("Extra", "sExtraMesh", extraMeshes);
		for (auto& xMesh : extraMeshes)
		{
			std::string_view xMeshPath = xMesh.pItem;
			if (!xMeshPath.empty() && g_OCSettings.bVFX)
				definedModels.emplace(xMeshPath);
		}
	}

	void InitConfigModelPaths(TESObjectWEAP* rWeap)
	{
		if (!rWeap || !rWeap->projectile || !rWeap->impactDataSet || !g_OCSettings.bVFX) return;

		definedModels.insert(rWeap->projectile->kMuzzleFlash.kModel.StdStr());
		definedModels.insert(rWeap->projectile->kModel.StdStr());

		for (auto& impact : rWeap->impactDataSet->impactDatas) {
			if (impact) definedModels.insert(impact->kModel.StdStr());
		}
	}

	void ParseGameData(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, const HeatConfiguration& defaults)
	{
		if (g_OCSettings.bStats) {
			ParseRangeFromINI(ini, secItem, "iAmmoConsumption",
				defaults.iMinAmmoUsed, defaults.iMaxAmmoUsed,
				config.iMinAmmoUsed, config.iMaxAmmoUsed,
				&defaults.iAddAmmoThreshold, &config.iAddAmmoThreshold);

			ParseRangeFromINI(ini, secItem, "iProjectileCount",
				defaults.iMinProjectiles, defaults.iMaxProjectiles,
				config.iMinProjectiles, config.iMaxProjectiles,
				&defaults.iAddProjectileThreshold, &config.iAddProjectileThreshold);

			ParseRangeFromINI(ini, secItem, "fDamage",
				defaults.fMinDamage, defaults.fMaxDamage,
				config.fMinDamage, config.fMaxDamage);

			ParseRangeFromINI(ini, secItem, "fCriticalDamage",
				defaults.fMinCritDamage, defaults.fMaxCritDamage,
				config.fMinCritDamage, config.fMaxCritDamage);

			ParseRangeFromINI(ini, secItem, "fProjectileSpeed",
				defaults.fMinProjectileSpeed, defaults.fMaxProjectileSpeed,
				config.fMinProjectileSpeed, config.fMaxProjectileSpeed);

			ParseRangeFromINI(ini, secItem, "fProjectileSize",
				defaults.fMinProjectileSize, defaults.fMaxProjectileSize,
				config.fMinProjectileSize, config.fMaxProjectileSize);

			ParseRangeFromINI(ini, secItem, "fFireRate",
				defaults.fMinFireRate, defaults.fMaxFireRate,
				config.fMinFireRate, config.fMaxFireRate);

			ParseRangeFromINI(ini, secItem, "fSpread",
				defaults.fMinSpread, defaults.fMaxSpread,
				config.fMinSpread, config.fMaxSpread);
		}

		if (!g_OCSettings.bMechanics) return;

		if (std::string_view objEffect = ini.GetValue(secItem, "sObjectEffect", ""); !objEffect.empty())
		{
			auto parts = SplitByDelimiter(objEffect, '(');
			if (!parts.empty())
			{
				if (parts[0].empty())
				{
					config.iObjectEffectID = defaults.iObjectEffectID;
				}
				else
				{
					char tempName[128]; 
					std::snprintf(tempName, sizeof(tempName), "%.*s", (int)parts[0].size(), parts[0].data());
					config.iObjectEffectID = TESForm::GetFormIDByEdID(tempName);
				}
			}
			if (parts.size() >= 2)
				config.iObjectEffectThreshold = static_cast<UInt8>(ParseDelimitedData(parts[1], '\0', ')'));
		}

		if (std::string_view altProj = ini.GetValue(secItem, "sAlternateProjectile", ""); !altProj.empty())
			config.iAltProjectileID = TESForm::GetFormIDByEdID(altProj.data());
		else config.iAltProjectileID = defaults.iAltProjectileID;
	}

	void ParseOverchargeData(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, const HeatConfiguration& defaults)
	{
		if (std::string_view OCFlagString = ini.GetValue(secItem, "sOverchargeFlags", ""); !OCFlagString.empty()) 
		{
			auto OCFlags = StringToFlags(OCFlagString, ' ', OCFlagNames);
			config.iOverchargeFlags = OCFlags;
		}

		config.fHeatPerShot = ini.GetDoubleValue(secItem, "fHeatPerShot", defaults.fHeatPerShot);
		config.fCooldownPerSecond = ini.GetDoubleValue(secItem, "fCooldownPerSecond", defaults.fCooldownPerSecond);

		if (g_OCSettings.bMechanics)
		{
			if (std::string_view OCEffectString = ini.GetValue(secItem, "sOverchargeEffects", ""); !OCEffectString.empty())
			{
				auto parts = SplitByDelimiter(OCEffectString, '(');
				if (!parts.empty()) {
					auto flags = StringToFlags(parts[0], ' ', OCEffectNames);
					config.iOverchargeEffect = flags;
				}
				if (parts.size() >= 2)
					config.iOverchargeEffectThreshold = static_cast<UInt8>(ParseDelimitedData(parts[1], '\0', ')'));
			}
		}

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
	}

	void ParseAssetData(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, TESObjectWEAP* rWeap)
	{
		char buffer[256];
		char keyBuffer[32];
		char nodeBuffer[128];

		if (g_OCSettings.bSounds) {
			if (std::string_view hSound = ini.GetValue(secItem, "sOverheatSFX", ""); !hSound.empty()) {
				std::snprintf(buffer, sizeof(buffer), "Sound\\OCSounds\\%.*s", (int)hSound.size(), hSound.data());
				config.sHeatSoundFile = buffer;
			}
			if (std::string_view cSound = ini.GetValue(secItem, "sChargeSFX", ""); !cSound.empty()) {
				std::snprintf(buffer, sizeof(buffer), "Sound\\OCSounds\\%.*s", (int)cSound.size(), cSound.data());
				config.sChargeSoundFile = buffer;
			}
		}
		std::string_view animFile = ini.GetValue(secItem, "sOverheatAnimation", "");
		std::string_view weapType = EnumToString(rWeap->eWeaponType, OCWeapTypeNames);
		if (animFile.empty()) {
			std::snprintf(buffer, sizeof(buffer), "OCAnims\\Overheat%.*s.kf", (int)weapType.size(), weapType.data());
		}
		else {
			std::snprintf(buffer, sizeof(buffer), "OCAnims\\%.*s", (int)animFile.size(), animFile.data());
		}

		if (g_OCSettings.bAnimations) config.sAnimFile = buffer;
		if (!g_OCSettings.bMeshes) return;
		CSimpleIniA::TNamesDepend nodeValues;
		ini.GetAllValues(secItem, "OCNode", nodeValues);

		int i = 0;
		for (auto it = nodeValues.begin(); it != nodeValues.end(); ++it, ++i)
		{
			std::string_view nodeLine = it->pItem;
			if (nodeLine.empty()) continue;

			auto parts = SplitByDelimiter(nodeLine, ')');
			if (parts.empty()) continue;

			std::string_view nodeMain = Trim(parts[0]);
			auto mainParts = SplitByDelimiter(nodeMain, '(');
			if (mainParts.empty()) continue;

			std::string_view nodeNameView = Trim(mainParts[0]);
			std::string_view flagNameView = (mainParts.size() >= 2) ? Trim(mainParts[1]) : std::string_view{};

			UInt32 flags = 0;
			if (!flagNameView.empty())
				flags = StringToFlags(flagNameView, ' ', OCXAddonNames);

			std::snprintf(nodeBuffer, sizeof(nodeBuffer), "%.*s", (int)nodeNameView.size(), nodeNameView.data());

			bool isAttachment = (strlen(nodeBuffer) >= 4)
				&& std::string_view(nodeBuffer + strlen(nodeBuffer) - 4, 4) == ".nif";

			NiPoint3 translate{ 0.f, 0.f, 0.f };
			NiPoint3 rotate{ 0.f, 0.f, 0.f };
			float scale = 1.0f;
			UInt32 indexForConfig;
			const char* finalName = nullptr;

			if (isAttachment)
			{
				std::snprintf(buffer, sizeof(buffer), "OCExtraMeshes\\%s", nodeBuffer);
				finalName = buffer;
				indexForConfig = static_cast<UInt32>(i);
				for (size_t p = 1; p < parts.size(); ++p)
				{
					std::string_view seg = Trim(parts[p]);
					if (seg.empty()) continue;

					char type = seg[0];
					if (type != 'T' && type != 'R' && type != 'S') continue;

					auto vals = ParseNiTransform(seg, type);
					if (type == 'T')      translate = { vals[0], vals[1], vals[2] };
					else if (type == 'R') rotate = { vals[0], vals[1], vals[2] };
					else if (type == 'S') scale = (vals[0] != 0.f ? vals[0] : 1.0f);
				}
			}
			else
			{
				finalName = nodeBuffer;
				scale = 0.0f;
				translate = { 0,0,0 };
				rotate = { 0,0,0 };
				indexForConfig = INVALID_U32;
			}

			HeatedNode hNode({ indexForConfig, flags, NiFixedString(finalName) });
			NiFixedString fixed(rWeap->kModel.c_str());
			OCExtraModels.push_back({ fixed, hNode, scale, translate, rotate });

			for (auto& modModel : rWeap->kModModels) {
				if (!(modModel.kModel.GetLength() == 0)) {
					NiFixedString modFixed = modModel.kModel.c_str();
					OCExtraModels.push_back({ modFixed, hNode, scale, translate, rotate });
				}
			}
			if (isAttachment) {
				const char* dot = std::strrchr(nodeBuffer, '.');
				size_t len = dot ? static_cast<size_t>(dot - nodeBuffer) : strlen(nodeBuffer);
				std::snprintf(nodeBuffer, sizeof(nodeBuffer), "%.*s%d", (int)len, nodeBuffer, i);
			}

			config.sHeatedNodes.push_back({ static_cast<UInt32>(i), flags, nodeBuffer });
		}
	}

	void LoadConfigSection(CSimpleIniA& ini, const char* secItem, HeatConfiguration& config, TESObjectWEAP* rWeap, const HeatConfiguration* baseConfig = nullptr)
	{
		const HeatConfiguration& defaults = baseConfig ? *baseConfig : HeatConfiguration{};

		ParseGameData(ini, secItem, config, defaults);
		ParseOverchargeData(ini, secItem, config, defaults);
		if (!baseConfig) 
			ParseAssetData(ini, secItem, config, rWeap);
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
				ini.SetMultiKey();
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
