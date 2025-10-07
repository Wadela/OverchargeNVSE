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
#include <BGSPrimitive.hpp>
#include <BSTempEffectParticle.hpp>
#include <TESEffectShader.hpp>
#include <Animation.hpp>
#include <MuzzleFlash.hpp>
#include <EffectSetting.hpp>
#include <MagicItemForm.hpp>
#include "ModelLoader.hpp"

//#include <tracy/Tracy.hpp>

namespace Overcharge
{
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

	inline std::shared_ptr<HeatData> GetActiveHeat(const UInt32 actorID, const UInt32 weaponID)
	{
		auto it = activeWeapons.find(MakeHashKey(actorID, weaponID));
		return (it != activeWeapons.end()) ? it->second : nullptr;
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

	inline std::shared_ptr<HeatData> GetOrCreateHeat(Actor* rActor, const HeatConfiguration& config)
	{
		auto player = PlayerCharacter::GetSingleton();
		UInt32 actorID = rActor->uiFormID;
		TESObjectWEAP* rWeap = rActor->GetCurrentWeapon();
		const UInt64 key = MakeHashKey(actorID, rWeap->uiFormID);

		auto [it, inserted] = activeWeapons.try_emplace(key);
		if (inserted) 
		{
			auto heat = std::make_shared<HeatData>(&config);
			if (actorID == player->uiFormID)
			{
				NiAVObjectPtr player1st = player->GetNode(1);
				NiAVObjectPtr player3rd = player->GetNode(0);
				if (player1st && player3rd)
				{
					auto blocks1st = ObjsFromStrings(&config, player1st, true);
					auto blocks3rd = ObjsFromStrings(&config, player3rd);
					blocks1st.insert(blocks1st.end(), blocks3rd.begin(), blocks3rd.end());
					heat->fx.targetBlocks = blocks1st;
				}
			}
			else 
			{
				if (NiAVObjectPtr sourceNode = rActor->Get3D())
				heat->fx.targetBlocks = ObjsFromStrings(&config, sourceNode);
			}
			it->second = std::move(heat);

			float actorSkLvl = rActor->GetActorValueF(ActorValue::Index(rWeap->weaponSkill));
			float reqSkill = rWeap->skillRequirement;         
			float ratio = actorSkLvl / reqSkill;
			ratio = (std::max)(ratio, 0.0001f);
			float smoothOffset = 0.5f * std::tanh(std::log(ratio));

			float heatPerShot = it->second->state.fHeatPerShot * (1 - smoothOffset);
			float cooldownRate = it->second->state.fCooldownRate * (1 + smoothOffset);
			it->second->state.fHeatPerShot = (std::max)(0.0f, heatPerShot);
			it->second->state.fCooldownRate = (std::max)(0.0f, cooldownRate);
		}
		it->second->config = &config;

		return it->second;
	}

	void WeaponCooldown();
}