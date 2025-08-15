#pragma once

#include "ValueModifierEffect.hpp"
#include "Gamebryo/NiSmartPointer.hpp"

class NiNode;
class BoltShaderProperty;

class AbsorbEffect : public ValueModifierEffect {
public:
	AbsorbEffect();
	~AbsorbEffect();

	NiPointer<NiNode>				spRootNode;
	NiPointer<NiNode>				spCasterNode;
	NiPointer<NiNode>				spTargetNode;
	NiPointer<BoltShaderProperty>	spShaderProperty;
};

ASSERT_SIZE(AbsorbEffect, 0x5C);