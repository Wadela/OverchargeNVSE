#pragma once

#include "MainHeader.hpp"
#include "Overcharge.hpp"
#include "OverchargeConfig.hpp"
#include "SafeWrite.hpp"
#include "NifOverride.hpp"


//Gamebryo
#include <NiPoint3.hpp>
#include <NiPointLight.hpp>
#include <NiPSysData.hpp>
#include <NiPSysVolumeEmitter.hpp>

//Bethesda
#include "BSInputManager.hpp"
#include <BGSPrimitive.hpp>
#include <BSTempEffectParticle.hpp>
#include <TESEffectShader.hpp>
#include <Animation.hpp>
#include <MuzzleFlash.hpp>
#include <EffectSetting.hpp>
#include <MagicItemForm.hpp>
#include "ModelLoader.hpp"
#include "MissileProjectile.hpp"

//#include <tracy/Tracy.hpp>

namespace Overcharge
{
	constexpr int NPC_UPDATE_THROTTLE = 15;

	extern std::vector<std::shared_ptr<HeatData>> playerOCWeapons;
	extern std::vector< std::shared_ptr<HeatData>>  activeOCWeapons;

	extern std::unordered_map<UInt64, std::shared_ptr<HeatData>>		activeWeapons;
	extern std::unordered_map<NiAVObject*, std::shared_ptr<HeatData>>	activeInstances;
	extern std::unordered_set<BSPSysSimpleColorModifier*>				colorModifiers;

	inline std::vector<std::pair<UInt32, NiAVObjectPtr>> ObjsFromStrings(const HeatConfiguration* data, const NiAVObjectPtr& sourceNode, bool isPlayer1st = false)
	{
		std::vector<std::pair<UInt32, NiAVObjectPtr>> blocks;
		if (sourceNode && data)
		{
			for (auto& it : data->sHeatedNodes)
			{
				UInt32 flags = it.flags;
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


	inline std::shared_ptr<HeatData> GetOrCreateHeat(const UInt32 actorID, const UInt32 weaponID, const NiAVObjectPtr& sourceNode, const HeatConfiguration& config)
	{
		const UInt64 key = MakeHashKey(actorID, weaponID);

		auto [it, inserted] = activeWeapons.try_emplace(key);
		if (inserted) {
			auto heat = std::make_shared<HeatData>(&config);
			heat->fx.targetBlocks = ObjsFromStrings(&config, sourceNode);
			it->second = std::move(heat);
		}
		it->second->config = &config;

		return it->second;
	}

	inline void InitializeHeatData(std::shared_ptr<HeatData>& heat, const HeatConfiguration* config)
	{
		auto player = PlayerCharacter::GetSingleton();
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
			activeOCWeapons.push_back(heat);
		}

		float actorSkLvl = heat->rActor->GetActorValueF(ActorValue::Index(heat->rWeap->weaponSkill));
		float reqSkill = heat->rWeap->skillRequirement;
		float ratio = (std::max)(actorSkLvl / reqSkill, 0.0001f);
		float smoothOff = 0.5f * std::tanh(std::log(ratio));

		heat->state.fHeatPerShot = (std::max)(0.0f, heat->state.fHeatPerShot * (1 - smoothOff));
		heat->state.fCooldownRate = (std::max)(0.0f, heat->state.fCooldownRate * (1 + smoothOff));
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


	inline void UpdateOverchargeShot(HeatState& st, float frameTime, float timePassed, const HeatConfiguration* config)
	{
		SInt32 attackHeld = 0;
		SInt32 attackDepressed = 0;
		auto inputManager = BSInputManager::GetSingleton();
		if (inputManager)
		{
			attackHeld = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Held);
			attackDepressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Depressed);
		}

		if (config->iOverchargeEffect & OCEffects_Overcharge && attackHeld)
		{
			if (!(st.uiOCEffect & OCEffects_Overheat))
				st.uiOCEffect |= OCEffects_Overcharge;
		}
		else if (attackDepressed && st.uiOCEffect & OCEffects_Overcharge)
		{
			st.uiOCEffect &= ~OCEffects_Overcharge;
			inputManager->SetUserAction(BSInputManager::Attack, BSInputManager::Pressed);
		}

		if ((st.uiOCEffect & OCEffects_Overcharge)
			&& !(st.uiOCEffect & OCEffects_Overheat)
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
		else if (st.uiOCEffect & OCEffects_AltProjectile) {
			st.uiOCEffect &= ~OCEffects_AltProjectile;
		}
	}

	inline void UpdateChargeDelay(HeatState& st, float frameTime, float timePassed, const HeatConfiguration* config)
	{
		SInt32 attackHeld = 0;
		SInt32 attackDepressed = 0;
		auto inputManager = BSInputManager::GetSingleton();
		if (inputManager)
		{
			attackHeld = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Held);
			attackDepressed = inputManager->GetUserAction(BSInputManager::Attack, BSInputManager::Depressed);
		}

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
		}

		if ((st.uiOCEffect & OCEffects_ChargeDelay)
			&& !(st.uiOCEffect & OCEffects_Overheat)
			&& st.fHeatVal <= HOT_THRESHOLD)
		{
			st.fHeatVal = (std::min)(99.0f, st.fHeatVal + frameTime * (2 * st.fHeatPerShot));
			st.uiTicksPassed = 0;
		}
	}

	void UpdateActiveOCWeapons();
	void UpdatePlayerOCWeapons();
}