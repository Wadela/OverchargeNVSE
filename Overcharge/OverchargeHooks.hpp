#pragma once

//Overcharge
#include "NifOverride.hpp"

//Gamebryo
#include <NiPoint3.hpp>
#include <NiPointLight.hpp>
#include <NiPSysVolumeEmitter.hpp>
#include <NiPSysMeshEmitter.hpp>

//Bethesda
#include <BSInputManager.hpp>
#include <BSTempEffectParticle.hpp>
#include <TESEffectShader.hpp>
#include <MuzzleFlash.hpp>
#include <MagicItemForm.hpp>
#include "PlayerCharacter.hpp"
#include "MiddleHighProcess.hpp"
#include "BGSSaveLoadGame.hpp"

//NVSE
#include <SafeWrite.hpp>

//#include <tracy/Tracy.hpp>

namespace Overcharge
{
	inline bool IsGamePaused()
	{
		bool isMainOrPauseMenuOpen = *(Menu**)0x11DAAC0; // g_startMenu, credits to lStewieAl
		auto* console = ConsoleManager::GetSingleton();

		return isMainOrPauseMenuOpen || console->IsConsoleOpen();
	}

	constexpr int NPC_UPDATE_THROTTLE = 15;

	extern std::vector<std::shared_ptr<HeatData>>		playerOCWeapons;
	extern std::vector<std::shared_ptr<HeatData>>		activeOCWeapons;
	extern std::vector<BSPSysSimpleColorModifierPtr>	colorModifiers;

	extern std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>	activeInstances;

	inline std::vector<OCBlock> ObjsFromStrings(
		const HeatConfiguration* data,
		const NiAVObjectPtr& sourceNode,
		bool isPlayer1st = false)
	{
		std::vector<OCBlock> blocks;
		if (!sourceNode || !data) return blocks;

		blocks.reserve(data->sHeatedNodes.size());

		if (isPlayer1st)
		OCTranslate = sourceNode->GetObjectByName("##OCTranslate");

		for (auto& it : data->sHeatedNodes)
		{
			if (!it.nodeName) continue;

			if (NiAVObjectPtr block = sourceNode->GetObjectByName(it.nodeName))
			{
				NiMaterialPropertyPtr mat = (it.flags & OCXColor)
					? NiMaterialProperty::CreateObject()
					: nullptr;

				blocks.emplace_back(it.flags, mat, block);

				if (isPlayer1st && (it.flags & OCXParticle))
				{
					if (NiNode* node = block->NiDynamicCast<NiNode>())
					{
						TraverseNiNode<NiParticleSystem>(node, [&](NiParticleSystemPtr psys) {
							if (!ContainsValue(worldSpaceParticles, psys))
								worldSpaceParticles.emplace_back(psys);
							});
					}
				}
			}
		}
		return blocks;
	}

	inline const HeatConfiguration* GetHeatConfig(const UInt32 weaponID, const UInt32 ammoID)
	{
		UInt64 configKey = MakeHashKey(weaponID, ammoID);
		auto it = weaponDataMap.find(configKey);
		if (it == weaponDataMap.end()) {
			configKey = static_cast<UInt64>(weaponID);
			it = weaponDataMap.find(configKey);
		}
		return (it != weaponDataMap.end()) ? &it->second : nullptr;
	}

	inline std::shared_ptr<HeatData> GetActiveHeat(UInt32 actorID, UInt32 weaponID)
	{
		auto player = PlayerCharacter::GetSingleton();
		const auto& vec = (player && actorID == player->uiFormID) ? playerOCWeapons : activeOCWeapons;

		auto it = std::find_if(vec.begin(), vec.end(),
			[&](const std::shared_ptr<HeatData>& heat) {
				return heat && heat->rActor && heat->rWeap &&
					heat->rActor->uiFormID == actorID &&
					heat->rWeap->uiFormID == weaponID;
			});

		return (it != vec.end()) ? *it : nullptr;
	}

	inline void InitializeHeatFX(std::shared_ptr<HeatData>& heat, const HeatConfiguration* config)
	{
		auto player = PlayerCharacter::GetSingleton();
		if (!player || !heat || !heat->rActor || !heat->rWeap) return;
		UInt32 actorID = heat->rActor->uiFormID;

		if (actorID == player->uiFormID) {
			NiAVObjectPtr player1st = player->GetPlayerNode(1);
			NiAVObjectPtr player3rd = player->GetPlayerNode(0);
			if (player1st && player3rd) {
				auto blocks1st = ObjsFromStrings(config, player1st, true);
				auto blocks3rd = ObjsFromStrings(config, player3rd);
				blocks1st.insert(blocks1st.end(), blocks3rd.begin(), blocks3rd.end());
				heat->fx.targetBlocks = std::move(blocks1st);
			}
			if (!ContainsValue(playerOCWeapons, heat))
				playerOCWeapons.push_back(heat);
		}
		else {
			if (NiAVObjectPtr sourceNode = heat->rActor->Get3D())
				heat->fx.targetBlocks = ObjsFromStrings(config, sourceNode);
			if (!ContainsValue(activeOCWeapons, heat))
				activeOCWeapons.push_back(heat);
		}
	}

	inline void InitializeHeatData(std::shared_ptr<HeatData>& heat, const HeatConfiguration* config)
	{
		float actorSkLvl = heat->rActor->GetActorValueF(ActorValue::Index(heat->rWeap->weaponSkill));
		float reqSkill = heat->rWeap->skillRequirement;
		float percentScaling = g_OCSettings.fSkillLevelScaling ? g_OCSettings.fSkillLevelScaling : 0.25f; //default 25% if not in config
		float baseHeatPerShot = heat->config->fHeatPerShot;
		float baseCooldownRate = heat->config->fCooldownPerSecond;

		if (!actorSkLvl || !reqSkill || !percentScaling || !baseHeatPerShot || !baseCooldownRate) return;

		if (reqSkill > 0.0f) {
			float skillRatio = actorSkLvl / reqSkill;
			float finalMultiplier = 1.0f + percentScaling * (1.0f - skillRatio);

			float minMult = 1.0f - percentScaling;
			float maxMult = 1.0f + percentScaling;
			float heatMultiplier = InterpolateTowards(finalMultiplier, minMult, maxMult);
			float cooldownMultiplier = InterpolateTowards(finalMultiplier, maxMult, minMult);
			heat->state.fHeatPerShot = baseHeatPerShot * heatMultiplier;
			heat->state.fCooldownRate = baseCooldownRate * cooldownMultiplier;
		}
	}

	inline bool FadeOutAndStop(BSSoundHandle* handle, uint32_t auiMilliseconds)
	{
		if (handle->uiSoundID == -1)
			return 0;

		UInt32 uiSoundID = handle->uiSoundID;
		BSAudioManager* audioManager = BSAudioManager::GetSingleton();

		audioManager->FadeTo(uiSoundID, auiMilliseconds, 4);
	}

	inline void InitializeHeatSounds(std::shared_ptr<HeatData>& heat, const HeatConfiguration* config)
	{
		std::string_view heatSound = heat->config->sHeatSoundFile;
		if (!heatSound.empty())
			heat->fx.heatSoundHandle =
			BSWin32Audio::GetSingleton()->GetSoundHandleByFilePath(
				heatSound.data(), 0, nullptr);

		std::string_view chargeSound = heat->config->sChargeSoundFile;
		if (!chargeSound.empty())
		{
			UInt32 audioFlags = 0;
			if (chargeSound.size() >= 3)
			{
				size_t dotPos = chargeSound.rfind('.');
				size_t endPos = (dotPos != std::string_view::npos) ? dotPos : chargeSound.size();
				if (chargeSound[endPos - 3] == '_' &&
					chargeSound[endPos - 2] == 'l' &&
					chargeSound[endPos - 1] == 'p')
				{
					audioFlags = 0x10;
				}
			}
			heat->fx.chargeSoundHandle =
				BSWin32Audio::GetSingleton()->GetSoundHandleByFilePath(
					chargeSound.data(), audioFlags, nullptr);
		}
	}

	inline std::shared_ptr<HeatData> GetOrCreateHeat(Actor* rActor)
	{
		if (!rActor) return nullptr;

		auto rWeap = rActor->GetCurrentWeapon();
		if (!rWeap) return nullptr;

		auto ammo = rWeap->GetEquippedAmmo(rActor);
		if (!ammo) return nullptr;

		auto config = GetHeatConfig(rWeap->uiFormID, ammo->uiFormID);
		if (!config) return nullptr;

		auto heat = GetActiveHeat(rActor->uiFormID, rWeap->uiFormID);
		if (heat) {
			if (heat->config != config)
			{
				heat->config = config;
				InitializeHeatData(heat, config);
			}
			return heat;
		}

		heat = std::make_shared<HeatData>(config);
		heat->rActor = rActor;
		heat->rWeap = rWeap;
		InitializeHeatData(heat, config);
		InitializeHeatFX(heat, config);
		InitializeHeatSounds(heat, config);
		return heat;
	}

	inline void DisableAiming()
	{
		auto inputManager = BSInputManager::GetSingleton();
		inputManager->SetUserAction(BSInputManager::Aim, BSInputManager::None);
		inputManager->SetUserAction(BSInputManager::ReadyItem, BSInputManager::None);
		inputManager->SetUserAction(BSInputManager::VATS_, BSInputManager::None);
	}

	inline void UpdateOverchargeShot(std::shared_ptr<HeatData> inst, float frameTime)
	{
		SInt32 attackHeld = 0;
		SInt32 attackDepressed = 0;
		SInt32 attackPressed = 0;
		HeatState& st = inst->state;
		const HeatConfiguration* config = inst->config;

		auto inputManager = BSInputManager::GetSingleton();
		if (inputManager)
		{
			attackHeld = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Held);
			attackDepressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Depressed);
			attackPressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Pressed);
		}

		if (config->iOverchargeEffect & OCEffects_Overcharge && attackPressed)
			st.uiTicksPassed = 0;

		float timePassed = st.uiTicksPassed * frameTime;
		static float shakeTime = 0.0f;
		static float shakeBlend = 0.0f; 
		static float currentVolume = 0.0f;

		if (config->iOverchargeEffect & OCEffects_Overcharge && attackHeld)
		{
			if (!(st.uiOCEffect & OCEffects_Overheat))
				st.uiOCEffect |= OCEffects_Overcharge;

			if (OCTranslate)
			{
				shakeTime += frameTime * 8.5f;
				shakeBlend = (std::min)(shakeBlend + frameTime * 2.0f, 1.0f);
				const float freq1 = 6.0f;
				const float freq2 = 11.0f;
				float heatFactor = std::clamp(st.fHeatVal / 100.0f, 0.0f, 1.0f);
				float shakeAmp = 0.08f * shakeBlend * heatFactor;
				float jitterX = (sinf(shakeTime * freq1) + 0.45f * cosf(shakeTime * freq2)) * shakeAmp;
				float jitterY = (cosf(shakeTime * freq1) + 0.15f * sinf(shakeTime * freq2)) * shakeAmp;
				OCTranslate->m_kLocal.m_Translate.x = jitterX;
				OCTranslate->m_kLocal.m_Translate.z = jitterY;
			}

			float targetVolume = st.fHeatVal / 100.0f;
			float rampSpeed = 2.0f;
			currentVolume += (targetVolume - currentVolume) * std::clamp(frameTime * rampSpeed, 0.0f, 1.0f);

			inst->fx.chargeSoundHandle.SetVolume(currentVolume);
			if (!inst->fx.chargeSoundHandle.IsPlaying())
				inst->fx.chargeSoundHandle.FadeInPlay(50);
		}
		else if (attackDepressed && st.uiOCEffect & OCEffects_Overcharge)
		{
			st.uiOCEffect &= ~OCEffects_Overcharge;
			inputManager->SetUserAction(BSInputManager::Attack, BSInputManager::Pressed);
			FadeOutAndStop(&inst->fx.chargeSoundHandle, 100);
			shakeTime = 0.0f;
			currentVolume = 0.0f;

			if (OCTranslate) {
				OCTranslate->m_kLocal.m_Translate.x = 0.0f;
				OCTranslate->m_kLocal.m_Translate.y = 0.0f;
			}
		}

		if ((st.uiOCEffect & OCEffects_Overcharge)
			&& !(st.uiOCEffect & OCEffects_Overheat)
			&& timePassed >= COOLDOWN_DELAY
			&& st.fHeatVal <= HOT_THRESHOLD)
		{
			st.fHeatVal = (std::min)(99.0f, st.fHeatVal + frameTime * (2 * st.fHeatPerShot));

		}

		if ((st.uiOCEffect & OCEffects_Overcharge) &&
			timePassed >= CHARGE_THRESHOLD &&
			st.fHeatVal >= st.uiOCEffectThreshold)
		{
			st.uiOCEffect |= OCEffects_AltProjectile;
		}
	}

	inline void UpdateChargeDelay(std::shared_ptr<HeatData> inst, float frameTime)
	{
		SInt32 attackHeld = 0;
		SInt32 attackDepressed = 0;
		SInt32 attackPressed = 0;
		HeatState& st = inst->state;
		const HeatConfiguration* config = inst->config;

		auto inputManager = BSInputManager::GetSingleton();
		if (inputManager)
		{
			attackHeld = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Held);
			attackDepressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Depressed);
			attackPressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Pressed);
		}

		static float shakeTime = 0.0f;
		static float shakeBlend = 0.0f;

		if ((config->iOverchargeEffect & OCEffects_ChargeDelay) && attackPressed)
			st.uiTicksPassed = 0;

		if ((config->iOverchargeEffect & OCEffects_ChargeDelay) && attackHeld)
		{
			if (!(st.uiOCEffect & OCEffects_Overheat))
			{
				if (st.uiTicksPassed == 0)
					st.fTargetVal = st.fHeatVal + st.uiOCEffectThreshold;

				if (st.fHeatVal <= st.fTargetVal)
				{
					st.uiOCEffect |= OCEffects_ChargeDelay;
				}
				else
				{
					st.uiOCEffect &= ~OCEffects_ChargeDelay;
					FadeOutAndStop(&inst->fx.chargeSoundHandle, 100);
					st.fTargetVal = -1.0f;
					shakeTime = 0.0f;

					if (OCTranslate) {
						OCTranslate->m_kLocal.m_Translate.x = 0.0f;
						OCTranslate->m_kLocal.m_Translate.y = 0.0f;
					}

				}
			}
		}
		else if (attackDepressed && (st.uiOCEffect & OCEffects_ChargeDelay))
		{
			st.uiOCEffect &= ~OCEffects_ChargeDelay;
			FadeOutAndStop(&inst->fx.chargeSoundHandle, 100);
			st.fTargetVal = -1.0f;
			shakeTime = 0.0f;

			if (OCTranslate) {
				OCTranslate->m_kLocal.m_Translate.x = 0.0f;
				OCTranslate->m_kLocal.m_Translate.y = 0.0f;
			}
		}

		if ((st.uiOCEffect & OCEffects_ChargeDelay)
			&& !(st.uiOCEffect & OCEffects_Overheat)
			&& st.fHeatVal <= HOT_THRESHOLD)
		{
			if (OCTranslate) {
				shakeTime += frameTime * 8.5f;
				shakeBlend = (std::min)(shakeBlend + frameTime * 2.0f, 1.0f);
				const float freq1 = 6.0f, freq2 = 12.0f;
				float shakeAmp = 0.08f * shakeBlend;
				float jitterX = (sinf(shakeTime * freq1) + 0.4f * cosf(shakeTime * freq2)) * shakeAmp;
				float jitterY = (cosf(shakeTime * freq1) + 0.1f * sinf(shakeTime * freq2)) * shakeAmp;
				OCTranslate->m_kLocal.m_Translate.x = jitterX;
				OCTranslate->m_kLocal.m_Translate.y = jitterY;
			}
			if (!inst->fx.chargeSoundHandle.IsPlaying())
				inst->fx.chargeSoundHandle.FadeInPlay(50);

			st.fHeatVal = (std::min)(99.0f, st.fHeatVal + frameTime * (2 * st.fHeatPerShot));
		}
	}

	inline void UpdateHeatFX(std::shared_ptr<HeatData>& heat, float frameTime)
	{
		auto& st = heat->state;
		auto& fx = heat->fx;
		auto& cfg = heat->config;

		if (st.fHeatVal <= 0 && st.uiTicksPassed <= 0)
			InitializeHeatFX(heat, cfg);

		fx.currCol = SmoothColorShift(st.fHeatVal, cfg->iMinColor, cfg->iMaxColor);

		if (fx.targetBlocks.empty()) return;

		float heatPercent = st.fHeatVal / 100.0f;

		for (const auto& node : fx.targetBlocks)
		{
			if (!node.target) continue;

			UInt32 effectFlags = 0;
			if (node.OCXFlags & OCXOnOverheat)   effectFlags |= OCEffects_Overheat;
			if (node.OCXFlags & OCXOnOvercharge) effectFlags |= OCEffects_Overcharge;
			if (node.OCXFlags & OCXOnDelay)      effectFlags |= OCEffects_ChargeDelay;
			if (node.OCXFlags & OCXOnAltProj)    effectFlags |= OCEffects_AltProjectile;
			bool hasEffectFlags = node.OCXFlags & (OCXOnOverheat | OCXOnOvercharge | OCXOnDelay | OCXOnAltProj | OCXOnHolster);
			bool onEffect = (effectFlags == 0) || (st.uiOCEffect & effectFlags);
			if (node.OCXFlags & OCXOnHolster && heat->rActor)
				onEffect = onEffect && heat->rActor->IsWeaponDrawn();

			bool isNegative = node.OCXFlags & OCXNegative;

			if ((node.OCXFlags & OCXParticle) && node.target->IsNiType<NiNode>()) {
				TraverseNiNode<NiParticleSystem>(
					static_cast<NiNode*>(node.target.m_pObject),
					[&](NiParticleSystem* psys) {
						if (auto ctlr = psys->GetControllers())
						{
							if (onEffect) ctlr->Start();
							else ctlr->Stop();
						}
					});
			}

			if ((node.OCXFlags & OCXCull))
			{
				bool isCulled = node.target->GetAppCulled();
				bool shouldCull = true;

				if (hasEffectFlags)
					shouldCull = !onEffect;

				if (isCulled != shouldCull)
					node.target->SetAppCulled(shouldCull);
			}

			if (!onEffect) continue;

			if (node.OCXFlags & OCXColor)
			{
				if (node.target->IsNiType<NiNode>())
				{
					TraverseNiNode<NiGeometry>(
						static_cast<NiNode*>(node.target.m_pObject),
						[&](NiGeometry* geom) {
							SetEmissiveColor(geom, fx.currCol, node.matProp);
						});

					TraverseNiNode<NiLight>(
						static_cast<NiNode*>(node.target.m_pObject),
						[&](NiLight* light) {
							light->SetDiffuseColor(fx.currCol);
						});
				}
				else if (node.target->IsNiType<NiGeometry>())
					SetEmissiveColor(node.target.m_pObject, fx.currCol, node.matProp);
			}

			if ((node.OCXFlags & OCXFlicker))
			{
				if (st.fHeatVal <= 0 && st.uiTicksPassed <= 0)
					st.fStartingVal = node.matProp->m_fEmitMult;

				ApplyFlicker(node.matProp->m_fEmitMult, st.fStartingVal, 
					frameTime, st.uiTicksPassed, isNegative);
			}

			if (node.OCXFlags & (OCXRotateX | OCXRotateY | OCXRotateZ)) {
				ApplyFixedRotation(node.target, heatPercent,
					node.OCXFlags & OCXRotateX,
					node.OCXFlags & OCXRotateY,
					node.OCXFlags & OCXRotateZ,
					isNegative);
			}
			if (node.OCXFlags & (OCXSpinX | OCXSpinY | OCXSpinZ)) {
				ApplyFixedSpin(node.target, heatPercent, frameTime,
					node.OCXFlags & OCXSpinX,
					node.OCXFlags & OCXSpinY,
					node.OCXFlags & OCXSpinZ,
					isNegative);
			}

		}
	}


	inline void InitPerks()
	{
		auto overclocker = TESForm::GetByID("OCPerkOverclocker");
		if (overclocker && overclocker->eTypeID == TESForm::kType_BGSPerk)
			OCPerkOverclocker = static_cast<BGSPerk*>(overclocker);

		auto voltageReg = TESForm::GetByID("OCPerkVoltageRegulator");
		if (voltageReg && voltageReg->eTypeID == TESForm::kType_BGSPerk)
			OCPerkVoltageRegulator = static_cast<BGSPerk*>(voltageReg);

		auto galRelativ = TESForm::GetByID("OCPerkGalvanicRelativist");
		if (galRelativ && galRelativ->eTypeID == TESForm::kType_BGSPerk)
			OCPerkGalvanicRelativist = static_cast<BGSPerk*>(galRelativ);

		auto cirBender = TESForm::GetByID("OCPerkCircuitBender");
		if (cirBender && cirBender->eTypeID == TESForm::kType_BGSPerk)
			OCPerkCircuitBender = static_cast<BGSPerk*>(cirBender);

		auto critMass = TESForm::GetByID("OCPerkCriticalMass");
		if (critMass && critMass->eTypeID == TESForm::kType_BGSPerk)
			OCPerkCriticalMass = static_cast<BGSPerk*>(critMass);

		auto coolantLeak = TESForm::GetByID("OCPerkCoolantLeak");
		if (coolantLeak && coolantLeak->eTypeID == TESForm::kType_BGSPerk)
			OCPerkCoolantLeak = static_cast<BGSPerk*>(coolantLeak);
	}

	inline void UpdatePerks(std::shared_ptr<HeatData> data)
	{
		if (!data || !data->rActor) return;

		auto& st = data->state;

		if (data->rActor->GetPerkRank(OCPerkOverclocker, 0))
		{
			if (st.fHeatVal > 60)
			{
				st.uiCritDamage *= 1.1;
			}
		}
		if (data->rActor->GetPerkRank(OCPerkVoltageRegulator, 0))
		{
			int rng = rand();
			if (st.fHeatVal < 50.0f && (rng % 100) < 25)
			{
				st.uiAmmoUsed = 0;
			}
		}
		if (data->rActor->GetPerkRank(OCPerkGalvanicRelativist, 0))
		{
			if (st.fHeatVal > 35.0f && st.fHeatVal < 65.0f)
			{
				st.uiDamage *= 1.2;
			}
			else st.uiDamage *= 0.9;
		}
		if (data->rActor->GetPerkRank(OCPerkCircuitBender, 0))
		{
			if (st.uiOCEffect & OCEffects_AltProjectile && (rand() % 100) < 50)
			{
				st.fHeatVal = (std::max)(0.0f, st.fHeatVal - st.fHeatPerShot);
			}
		}
		if (data->rActor->GetPerkRank(OCPerkCriticalMass, 0))
		{
			if (st.IsHot() && !(st.uiOCEffect & OCEffects_SelfDamage))
			{
				st.uiOCEffect |= OCEffects_SelfDamage;
				TESForm* effect = TESForm::GetByID("EnchFlamerEffect");
				if (effect) data->rActor->CastSpellImmediate(reinterpret_cast<MagicItemForm*>(effect), 0, data->rActor, 1, 0);
				st.bCanOverheat = false;
			}
			else if (!st.IsHot() && st.uiOCEffect & OCEffects_SelfDamage)
			{
				st.uiOCEffect &= ~OCEffects_SelfDamage;
			}
		}
		if (data->rActor->GetPerkRank(OCPerkCoolantLeak, 0))
		{
			if (st.fHeatVal >= 70.0f)
			{
				st.fHeatVal = 100.0f;
			}
		}
	}

	void InitHooks();
	void ClearOCWeapons();
	void RefreshPlayerOCWeapons();
	void UpdateActiveOCWeapons();
	void UpdatePlayerOCWeapons();
}