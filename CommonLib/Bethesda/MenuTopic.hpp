#pragma once

#include "BSString.hpp"
#include "BSSimpleList.hpp"

class TESQuest;
class TESTopicInfo;
class TESTopic;
class DialogueResponse;

struct MenuTopic {
	BSString						strDialogue;
	bool							byte8;
	bool							byte9;
	bool							byteA;
	BSSimpleList<DialogueResponse*> kResponses;
	TESQuest*						pQuest;
	TESTopicInfo*					pTopicInfo;
	TESTopic*						pTopic;
	DWORD							dword20;
	bool							byte24;
	bool							bNotSaidOnce;
	TESTopic*						pTopic28;
};

ASSERT_SIZE(MenuTopic, 0x2C);