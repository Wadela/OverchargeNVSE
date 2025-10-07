#pragma once

#include "BSExtraData.hpp"

class TESObjectREFR;

struct PortalLinkedRefData {
	TESObjectREFR* pRef[2];
};

class ExtraPortalRefData : public BSExtraData {
public:
	PortalLinkedRefData* pData;
};

ASSERT_SIZE(ExtraPortalRefData, 0x10);