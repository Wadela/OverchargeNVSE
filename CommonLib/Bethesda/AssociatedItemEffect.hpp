#pragma once

#include "ActiveEffect.hpp"

class TESObject;

// 0x4C
class AssociatedItemEffect : public ActiveEffect
{
public:
	AssociatedItemEffect();
	~AssociatedItemEffect();

	TESObject* pkItem;	// 48 - creature, armor, weapon
};
ASSERT_SIZE(AssociatedItemEffect, 0x4C);