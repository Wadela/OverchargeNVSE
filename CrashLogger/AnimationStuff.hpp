#pragma once

#include "MainHeader.hpp"

#include "Animation.hpp"
#include "NiControllerManager.hpp"
#include <shared_mutex>
#include "ModelLoader.hpp"
#include "NiTextKeyExtraData.hpp"

// Hash function for std::pair
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator()(const std::pair<T1, T2>& p) const {
		const auto hash1 = std::hash<T1>()(p.first);
		const auto hash2 = std::hash<T2>()(p.second);
		return hash_combine(hash1, hash2);
	}

private:
	// A better way to combine hashes
	static std::size_t hash_combine(std::size_t seed, std::size_t hash) {
		return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
	}
};

// Custom equality comparator
struct pair_equal {
	template <class T1, class T2>
	bool operator()(const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs) const {
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}
};

std::string_view AddStringToPool(const std::string_view str)
{
	auto* handle = NiGlobalStringTable::AddString(str.data());
	return { handle, str.size() };
}

struct BSAnimationContext
{
	NiPointer<BSAnimGroupSequence> anim;
	AnimSequenceBase* base;

	BSAnimationContext(BSAnimGroupSequence* anim, AnimSequenceBase* base) : anim(anim), base(base)
	{
	}
};

template <typename T_Data>
using NiTAnimMap = NiTPointerMap <UInt16, T_Data>; 

using GameAnimMap = NiTAnimMap<AnimSequenceBase*>;

namespace GameFuncs
{
	inline auto* LoadAnimation = reinterpret_cast<bool(__thiscall*)(Animation*, KFModel*, bool)>(0x490500);
	inline auto* NiTPointerMap_RemoveKey = reinterpret_cast<bool(__thiscall*)(void*, UInt16)>(0x49C250);
	inline auto* NiTPointerMap_Init = reinterpret_cast<GameAnimMap * (__thiscall*)(GameAnimMap*, int numBuckets)>(0x49C050);
	inline auto* NiControllerSequenceActivate = reinterpret_cast<bool(__thiscall*)(NiControllerSequence*, char, bool, float, float, NiControllerSequence*, bool)>(0xA34F20);
	inline auto* NiControllerSequenceDeactivate = reinterpret_cast<int(__thiscall*)(NiControllerManager*, NiControllerSequence*, float)>(0x47B220);
}

std::shared_mutex g_loadCustomAnimationMutex;


typedef void* (*_FormHeap_Allocate)(UInt32 size);
extern const _FormHeap_Allocate FormHeap_Allocate;

const _FormHeap_Allocate FormHeap_Allocate = (_FormHeap_Allocate)0x00401000;
GameAnimMap* CreateGameAnimMap(UInt32 bucketSize)
{
	// see 0x48F9F1
	auto* alloc = static_cast<GameAnimMap*>(FormHeap_Allocate(0x10));
	return GameFuncs::NiTPointerMap_Init(alloc, bucketSize);
}


thread_local GameAnimMap* s_customMap = nullptr;

// intentional const char*, anim paths are pooled and their pointers remain consistent throughout lifetime
std::unordered_map<std::pair<const char*, Animation*>, BSAnimationContext, pair_hash, pair_equal> g_cachedAnimMap;

enum eAnimSequence
{
	kSequence_None = -0x1,
	kSequence_Idle = 0x0,
	kSequence_Movement = 0x1,
	kSequence_LeftArm = 0x2,
	kSequence_LeftHand = 0x3,
	kSequence_Weapon = 0x4,
	kSequence_WeaponUp = 0x5,
	kSequence_WeaponDown = 0x6,
	kSequence_SpecialIdle = 0x7,
	kSequence_Death = 0x14,
};

std::optional<BSAnimationContext> LoadCustomAnimation(std::string_view path, Animation* animData)
{
	const auto key = std::make_pair(path.data(), animData);
	{
		std::shared_lock lock(g_loadCustomAnimationMutex);
		if (const auto iter = g_cachedAnimMap.find(key); iter != g_cachedAnimMap.end())
		{
			return iter->second;
		}
	}

	const auto tryCreateAnimation = [&]() -> std::optional<BSAnimationContext>
		{
			auto* kfModel = ModelLoader::LoadKFModel(path.data());
			if (kfModel && kfModel->animGroup && animData)
			{
				const auto groupId = kfModel->animGroup->usGroupID;

				if (auto* base = animData->pAnimSequenceBases->Lookup(groupId))
				{
					// fix memory leak, can't previous anim in map since it might be blending
					auto* anim = base->GetSequenceByIndex(-1);
					if (anim && _stricmp(anim->m_kName, path.data()) == 0)
						return BSAnimationContext(anim, base);
					GameFuncs::NiTPointerMap_RemoveKey(animData->pAnimSequenceBases, groupId);
				}
				const auto oldMovementQueuedGroupId = animData->usNextGroups[kSequence_Movement];
				if (GameFuncs::LoadAnimation(animData, kfModel, false))
				{
					if (animData->usNextGroups[kSequence_Movement] != oldMovementQueuedGroupId)

						// hack fix for hack fix at 0x4946D2
						animData->usNextGroups[kSequence_Movement] = oldMovementQueuedGroupId;
					if (auto* base = animData->pAnimSequenceBases->Lookup(groupId))
					{
						BSAnimGroupSequence* anim;
						if (base && ((anim = base->GetSequenceByIndex(-1))))
						{
							auto iter = g_cachedAnimMap.emplace(key, BSAnimationContext(anim, base));
							return iter.first->second;
						}
					}
				}

			}
			return std::nullopt;
		};

	std::unique_lock lock(g_loadCustomAnimationMutex);
	auto* defaultMap = animData->pAnimSequenceBases;

	if (!s_customMap)
		s_customMap = CreateGameAnimMap(1);

	animData->pAnimSequenceBases = s_customMap;
	const auto result = tryCreateAnimation();
	animData->pAnimSequenceBases = defaultMap;
	s_customMap->Clear();
	lock.unlock();

	return result;
}


BSAnimGroupSequence* FindOrLoadAnim(Animation* animData, const char* path)
{
	if (!animData || !animData->spControllerManager)
		return nullptr;

	if (auto* anim = animData->spControllerManager->m_kIndexMap.Lookup(path))
		return static_cast<BSAnimGroupSequence*>(anim);
	const std::string_view pooledPath = AddStringToPool(ToLower(path));
	const auto& ctx = LoadCustomAnimation(pooledPath, animData);
	if (ctx)
		return ctx->anim;
	return nullptr;
}

Animation* GetAnimData(Actor* actor, int firstPerson)
{
	const auto player = PlayerCharacter::GetSingleton();

	if (!actor || !actor->pkBaseProcess)
		return nullptr;

	switch (firstPerson)
	{
	case -1:
		if (actor != player)
			return actor->GetAnimData();
		return !player->IsThirdPerson() ? player->pFirstPersonArms : player->pkBaseProcess->GetAnimData();
	case 0:
		return actor->pkBaseProcess->GetAnimData();
	case 1:
	default:
		if (actor == player)
			return player->pFirstPersonArms;
		return actor->GetAnimData();
	}
}


void PlayAnimPath(std::string path, Actor* actor)
{
	auto bIsFirstPerson = -1;

	auto* animData = GetAnimData(actor, bIsFirstPerson);

	if (const auto anim = FindOrLoadAnim(animData, path.c_str()))
	{
		if (anim->m_eState != NiControllerSequence::kState_Inactive)
			GameFuncs::NiControllerSequenceDeactivate(animData->spControllerManager, anim, 0.0f);

		const auto easeInTime = 0.0f;
		GameFuncs::NiControllerSequenceActivate(anim, 0, true, anim->m_fSeqWeight, easeInTime, nullptr, false);
	}
}
