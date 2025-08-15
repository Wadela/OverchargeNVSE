#pragma once

#include "BSExtraData.hpp"
#include "BSSimpleList.hpp"

class FACTION_RANK;

class ExtraFactionChanges : public BSExtraData {
public:
	ExtraFactionChanges();
	virtual ~ExtraFactionChanges();

	BSSimpleList<FACTION_RANK*>* pFactionChanges;

	EXTRADATATYPE(FACTIONCHANGES);
};

ASSERT_SIZE(ExtraFactionChanges, 0x10);