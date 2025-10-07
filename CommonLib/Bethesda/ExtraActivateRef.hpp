#pragma once
#include "BSExtraData.hpp"
#include "BSString.hpp"
#include "BSSimpleList.hpp"

class TESObjectREFR;

// 20
class ExtraActivateRef : public BSExtraData
{
public:
	struct parentRef
	{
		TESObjectREFR* ref;
		float			delay;
	};
	BSSimpleList<parentRef*>	parentRefs;
	UInt32						flags;
	BSString					activationPromptOverride;
};
static_assert(sizeof(ExtraActivateRef) == 0x20);