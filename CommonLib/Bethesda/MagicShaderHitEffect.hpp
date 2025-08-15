#pragma once

#include "MagicHitEffect.hpp"
#include "ParticleShaderProperty.hpp"
#include "BSSimpleArray.hpp"

class TextureEffectData;
class TESBoundObject;
class TESEffectShader;

class MagicShaderHitEffect : public MagicHitEffect {
public:
	MagicShaderHitEffect();
	~MagicShaderHitEffect();

	bool											 unk28;
	TESBoundObject*									 pBoundObject;
	TESEffectShader*								 effectShader;
	float											 time34;				// 34	Init'd to float
	BSSimpleArray<NiPointer<ParticleShaderProperty>> kShaderProperties;
	NiNode*											 shaderNode;
	bool											 isVisible;
	BSSimpleArray<NiPointer<NiAVObject>>			 objects;
	float											 flt60;
	float											 flt64;
	TextureEffectData*								 effectProperty;
};

ASSERT_SIZE(MagicShaderHitEffect, 0x6C);