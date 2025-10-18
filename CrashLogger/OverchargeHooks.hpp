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

//NVSE
#include <SafeWrite.hpp>

//#include <tracy/Tracy.hpp>

namespace Overcharge
{
	constexpr int NPC_UPDATE_THROTTLE = 15;

	extern std::vector<std::shared_ptr<HeatData>>		playerOCWeapons;
	extern std::vector<std::shared_ptr<HeatData>>		activeOCWeapons;
	extern std::vector<BSPSysSimpleColorModifierPtr>	colorModifiers;

	extern std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>	activeInstances;

	inline std::vector<std::pair<UInt32, NiAVObjectPtr>> ObjsFromStrings(const HeatConfiguration* data, const NiAVObjectPtr& sourceNode, bool isPlayer1st = false)
	{
		std::vector<std::pair<UInt32, NiAVObjectPtr>> blocks;
		if (sourceNode && data)
		{
			for (auto& it : data->sHeatedNodes)
			{
				UInt32 flags = it.flags;
				if (!it.nodeName) continue;
				const NiFixedString& nodeName = it.nodeName;

				if (NiAVObjectPtr block = sourceNode->GetObjectByName(nodeName))
				{
					blocks.emplace_back(flags, block);
					if (isPlayer1st && flags & OCXParticle && block->IsNiType<NiNode>())
					{
						TraverseNiNode<NiParticleSystem>(static_cast<NiNode*>(block.m_pObject), [&](NiParticleSystemPtr psys) {
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

	inline void InitializeHeatData(std::shared_ptr<HeatData>& heat, const HeatConfiguration* config)
	{
		auto player = PlayerCharacter::GetSingleton();
		if (!player || !heat || !heat->rActor || !heat->rWeap) return; 
		UInt32 actorID = heat->rActor->uiFormID;

		if (actorID == player->uiFormID) {
			NiAVObjectPtr player1st = player->GetNode(1);
			NiAVObjectPtr player3rd = player->GetNode(0);
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

		float actorSkLvl = heat->rActor->GetActorValueF(ActorValue::Index(heat->rWeap->weaponSkill));
		float reqSkill = heat->rWeap->skillRequirement;
		float percentScaling = g_OCSettings.fSkillLevelScaling ? g_OCSettings.fSkillLevelScaling : 0.30f; //default 35% if not in config
		float baseHeatPerShot = heat->config->fHeatPerShot;
		float baseCooldownRate = heat->config->fCooldownPerSecond;

		if (!actorSkLvl || !reqSkill || !percentScaling || !baseHeatPerShot || !baseCooldownRate) return;

		if (reqSkill > 0.0f) {
			float skillRatio = actorSkLvl / reqSkill;
			float finalMultiplier = 1.0f + percentScaling * (1.0f - skillRatio);

			float minMult = 1.0f - percentScaling;
			float maxMult = 1.0f + percentScaling;
			finalMultiplier = std::clamp(finalMultiplier, minMult, maxMult);

			heat->state.fHeatPerShot = baseHeatPerShot * finalMultiplier;
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
			heat->config = config;
			return heat;
		}

		heat = std::make_shared<HeatData>(config);
		heat->rActor = rActor;
		heat->rWeap = rWeap;
		InitializeHeatData(heat, config);

		return heat;
	}

	inline void UpdateOverchargeShot(HeatState& st, float frameTime, const HeatConfiguration* config)
	{
		if (st.uiOCEffect & OCEffects_Overheat) return;

		SInt32 attackHeld = 0;
		SInt32 attackDepressed = 0;
		SInt32 attackPressed = 0;
		auto inputManager = BSInputManager::GetSingleton();
		if (inputManager)
		{
			attackHeld = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Held);
			attackDepressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Depressed);
			attackPressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Pressed);
		}

		if (config->iOverchargeEffect & OCEffects_Overcharge && attackPressed) st.uiTicksPassed = 0;

		float timePassed = st.uiTicksPassed * frameTime;

		auto player = PlayerCharacter::GetSingleton();
		auto node1st = player->GetNode(1);
		auto trm = node1st->GetObjectByName("##trm");
		static float shakeTime = 0.0f;
		static float shakeBlend = 0.0f; 

		if (config->iOverchargeEffect & OCEffects_Overcharge && attackHeld)
		{
			if (!(st.uiOCEffect & OCEffects_Overheat))
				st.uiOCEffect |= OCEffects_Overcharge;

			shakeTime += frameTime * 8.5f;
			shakeBlend = (std::min)(shakeBlend + frameTime * 2.0f, 1.0f);

			const float freq1 = 6.0f;
			const float freq2 = 12.0f;

			float heatFactor = std::clamp(st.fHeatVal / 100.0f, 0.0f, 1.0f);
			float shakeAmp = 0.08f * shakeBlend * heatFactor;

			float jitterX = (sinf(shakeTime * freq1) + 0.4f * cosf(shakeTime * freq2)) * shakeAmp;
			float jitterY = (cosf(shakeTime * freq1) + 0.1f * sinf(shakeTime * freq2)) * shakeAmp;

			trm->m_kLocal.m_Translate.x = jitterX;
			trm->m_kLocal.m_Translate.z = jitterY;
		}
		else if (attackDepressed && st.uiOCEffect & OCEffects_Overcharge)
		{
			st.uiOCEffect &= ~OCEffects_Overcharge;
			inputManager->SetUserAction(BSInputManager::Attack, BSInputManager::Pressed);

			trm->m_kLocal.m_Translate.x = 0.0f;
			trm->m_kLocal.m_Translate.y = 0.0f;
			shakeTime = 0.0f;
		}

		if ((st.uiOCEffect & OCEffects_Overcharge)
			&& !(st.uiOCEffect & OCEffects_Overheat)
			&& st.fHeatVal <= HOT_THRESHOLD)
		{
			st.fHeatVal = (std::min)(99.0f, st.fHeatVal + frameTime * (3 * st.fHeatPerShot));
		}

		if ((st.uiOCEffect & OCEffects_Overcharge) &&
			timePassed >= CHARGE_THRESHOLD &&
			st.fHeatVal >= st.uiOCEffectThreshold)
		{
			st.uiOCEffect |= OCEffects_AltProjectile;
		}
	}

	inline void UpdateChargeDelay(HeatState& st, float frameTime, float timePassed, const HeatConfiguration* config)
	{
		if (st.uiOCEffect & OCEffects_Overheat) return;

		SInt32 attackHeld = 0;
		SInt32 attackDepressed = 0;
		SInt32 attackPressed = 0;
		auto inputManager = BSInputManager::GetSingleton();
		if (inputManager)
		{
			attackHeld = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Held);
			attackDepressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Depressed);
			attackPressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Pressed);
		}

		auto player = PlayerCharacter::GetSingleton();
		auto node1st = player->GetNode(1);
		auto trm = node1st->GetObjectByName("##trm");
		static float shakeTime = 0.0f;
		static float shakeBlend = 0.0f;

		if (config->iOverchargeEffect & OCEffects_ChargeDelay && attackHeld)
		{
			if (!(st.uiOCEffect & OCEffects_Overheat))
			{
				if (st.fHeatVal < st.uiOCEffectThreshold)
					st.uiOCEffect |= OCEffects_ChargeDelay;
				else st.uiOCEffect &= ~OCEffects_ChargeDelay;
			}
		}
		else if (attackDepressed && st.uiOCEffect & OCEffects_ChargeDelay)
		{
			st.uiOCEffect &= ~OCEffects_ChargeDelay;
			trm->m_kLocal.m_Translate.x = 0.0f;
			trm->m_kLocal.m_Translate.y = 0.0f;
			shakeTime = 0.0f;
		}

		if ((st.uiOCEffect & OCEffects_ChargeDelay)
			&& !(st.uiOCEffect & OCEffects_Overheat)
			&& st.fHeatVal <= HOT_THRESHOLD)
		{
			shakeTime += frameTime * 8.5f;
			shakeBlend = (std::min)(shakeBlend + frameTime * 2.0f, 1.0f);

			const float freq1 = 6.0f;
			const float freq2 = 12.0f;

			float shakeAmp = 0.08f * shakeBlend;

			float jitterX = (sinf(shakeTime * freq1) + 0.4f * cosf(shakeTime * freq2)) * shakeAmp;
			float jitterY = (cosf(shakeTime * freq1) + 0.1f * sinf(shakeTime * freq2)) * shakeAmp;

			trm->m_kLocal.m_Translate.x = jitterX;
			trm->m_kLocal.m_Translate.y = jitterY;

			st.fHeatVal = (std::min)(99.0f, st.fHeatVal + frameTime * (3 * st.fHeatPerShot));
			st.uiTicksPassed = 0;
		}
	}

	inline void UpdateHeatFX(std::shared_ptr<HeatData>& heat, float frameTime)
	{
		auto& st = heat->state;
		auto& fx = heat->fx;
		auto& cfg = heat->config;

		fx.currCol = SmoothColorShift(st.fHeatVal, cfg->iMinColor, cfg->iMaxColor);

		if (fx.targetBlocks.empty()) return;

		float heatPercent = st.fHeatVal / 100.0f;

		for (const auto& node : fx.targetBlocks)
		{
			if (!node.second) continue;

			UInt32 effectFlags = 0;
			if (node.first & OCXOnOverheat)   effectFlags |= OCEffects_Overheat;
			if (node.first & OCXOnOvercharge) effectFlags |= OCEffects_Overcharge;
			if (node.first & OCXOnDelay)      effectFlags |= OCEffects_ChargeDelay;
			if (node.first & OCXOnAltProj)    effectFlags |= OCEffects_AltProjectile;

			const bool onEffect = (effectFlags == 0) || (st.uiOCEffect & effectFlags);

			if ((node.first & OCXParticle) && node.second->IsNiType<NiNode>()) {
				TraverseNiNode<NiParticleSystem>(
					static_cast<NiNode*>(node.second.m_pObject),
					[&](NiParticleSystem* psys) {
						if (auto ctlr = psys->GetControllers())
						{
							if (node.first & OCXColor)
							SetEmissiveColor(psys, fx.currCol, fx.fxMatProp);
							if (onEffect) ctlr->Start();
							else ctlr->Stop();
						}
					});
			}

			if (!onEffect) continue;

			if (node.first & OCXColor)
				SetEmissiveColor(node.second.m_pObject, fx.currCol, fx.objMatProp);

			if (node.first & (OCXRotateX | OCXRotateY | OCXRotateZ)) {
				ApplyFixedRotation(node.second, heatPercent,
					node.first & OCXRotateX,
					node.first & OCXRotateY,
					node.first & OCXRotateZ);
			}

			if (node.first & (OCXSpinX | OCXSpinY | OCXSpinZ)) {
				ApplyFixedSpin(node.second, heatPercent, frameTime,
					node.first & OCXSpinX,
					node.first & OCXSpinY,
					node.first & OCXSpinZ);
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

	void ClearOCWeapons();
	void UpdateActiveOCWeapons();
	void UpdatePlayerOCWeapons();
}