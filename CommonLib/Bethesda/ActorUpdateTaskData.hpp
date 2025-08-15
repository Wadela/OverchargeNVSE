#pragma once
#include "MobileObjectTaskletData.hpp"

class Actor;

// 0x3C
class ActorUpdateTaskData : public MobileObjectTaskletData {
public:
	UInt32 uiRunCount;

	static void __fastcall ExecuteTaskEx(ActorUpdateTaskData* apThis, void*, Actor* apActor);
};
ASSERT_SIZE(ActorUpdateTaskData, 0x3C);