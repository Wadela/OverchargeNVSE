#pragma once
#include "BSExtraData.hpp"
#include "BSSimpleList.hpp"

class TESObjectREFR;

// 14
class ExtraLinkedRefChildren : public BSExtraData
{
public:
	BSSimpleList<TESObjectREFR*> kLinkedRefChildren;
};
static_assert(sizeof(ExtraLinkedRefChildren) == 0x14);