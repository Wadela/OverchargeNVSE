#pragma once

#include "MainHeader.hpp"
#include "Animation.hpp"
#include "NiControllerManager.hpp"
#include "ModelLoader.hpp"

// Courtesy of Kormakur - kNVSE (https://github.com/korri123/kNVSE/blob/dbf8713d908eb10c35089dafbdfbbc4d575fed20/nvse_plugin_example/commands_animation.cpp#L1505)

namespace GameFuncs
{
	inline auto* LoadAnimation = reinterpret_cast<bool(__thiscall*)(Animation*, KFModel*, bool)>(0x490500);
	inline auto* NiControllerSequenceActivate = reinterpret_cast<bool(__thiscall*)(NiControllerSequence*, char, bool, float, float, NiControllerSequence*, bool)>(0xA34F20);
	inline auto* NiControllerManagerDeactivate = reinterpret_cast<int(__thiscall*)(NiControllerManager*, NiControllerSequence*, float)>(0x47B220);
	inline auto* TESEffectShaderStartParticleShader = reinterpret_cast<NiCamera * (__thiscall*)(TESEffectShader*, NiAVObject*, NiAVObject*, NiSourceTexture*, float)>(0x5077C0);
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

BSAnimGroupSequence* LoadAnim(Animation* animData, const char* path)
{
	auto* kfModel = ModelLoader::LoadKFModel(path);
	const auto makeAnim = [&]()->BSAnimGroupSequence*
	{
		if (kfModel && kfModel->animGroup && animData)
		{
			const auto groupId = kfModel->animGroup->usGroupID;
			if (GameFuncs::LoadAnimation(animData, kfModel, true))
			{
				if (auto* base = animData->pAnimSequenceBases->Lookup(groupId))
				{
					BSAnimGroupSequence* anim;
					if (base && ((anim = base->GetSequenceByIndex(-1))))
					{
						return anim;
					}
				}
			}
		}
	};
	const auto result = makeAnim();
	return result;
}

void PlayAnimPath(std::string path, Actor* actor)
{
	auto bIsFirstPerson = -1;

	auto* animData = GetAnimData(actor, bIsFirstPerson);

	if (const auto anim = LoadAnim(animData, path.c_str()))
	{
		if (anim->m_eState != NiControllerSequence::kState_Inactive)
		GameFuncs::NiControllerManagerDeactivate(animData->spControllerManager, anim, 0.0f);

		GameFuncs::NiControllerSequenceActivate(anim, 0, true, anim->m_fSeqWeight, 0.0f, nullptr, false);
	}
}
