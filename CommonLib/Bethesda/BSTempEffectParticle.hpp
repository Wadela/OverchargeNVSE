#pragma once

#include "BSTempEffect.hpp"
#include "BSSoundHandle.hpp"
#include "NiTransform.hpp"

class TESObjectCELL;

NiSmartPointer(BSTempEffectParticle);

class BSTempEffectParticle : public BSTempEffect {
public:
	NiAVObject* 	spParticleObject; 
	const char*		pFileName;
	NiRefObjectPtr	spParentNode; 
	NiTransform		transform;
	DWORD			dword58;
	BSSoundHandle	Sound1;
	BSSoundHandle	Sound2;

	static void Spawn(TESObjectCELL* apCell, float afLifetime, const char* apParticleFilename);

	NIRTTI_ADDRESS(0x11d6b84);
};

ASSERT_SIZE(BSTempEffectParticle, 0x74);