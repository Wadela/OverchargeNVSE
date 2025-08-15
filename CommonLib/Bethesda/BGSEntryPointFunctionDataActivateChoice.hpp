#pragma once
#include "BGSEntryPointFunctionData.hpp"
#include "BSString.hpp"

class Script;

// 0x14
class BGSEntryPointFunctionDataActivateChoice : public BGSEntryPointFunctionData {
public:
	BGSEntryPointFunctionDataActivateChoice();
	~BGSEntryPointFunctionDataActivateChoice();

	BSString			kLabel;				// 04
	Script*				pkScript;			// 0C
	UInt32				uiFlags;			// 10
};
ASSERT_SIZE(BGSEntryPointFunctionDataActivateChoice, 0x14);