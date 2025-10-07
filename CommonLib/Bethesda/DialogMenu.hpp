#pragma once

#include "Menu.hpp"
#include "ListBox.hpp"
#include "BSSoundHandle.hpp"

class TESTopic;
class TESTopicInfo;
class ImageSpaceModifierInstance;
class MenuTopicManager;
class Actor;

class DialogMenu : public Menu {
public:
	DialogMenu();
	~DialogMenu();

	int							eStage;
	int							field_2C;
	Tile*						pTiles[4];
	ListBox<SInt32>				topicList;
	MenuTopicManager*			pMenuTopicManager;
	BSSoundHandle				sound74;
	TESObjectREFR*				pTargetRef;
	char						cText84[128];
	bool						bHasResponses;
	float						fTime;
	int							lastMenuClickTime;
	BSSimpleList<TESTopicInfo*>	infos110;
	BSSimpleList<TESTopicInfo*>	infos118;
	BSSimpleList<TESTopicInfo*>	infos120;
	float						fBlurStrength;
	ImageSpaceModifierInstance* pImageSpaceDOF;
	bool						bWasPlayerThirdPerson;
	UInt32						unk134;
	bool						bIsBarterMenuShown;
	bool						bShouldShowHelpMessage;

	static DialogMenu* GetSingleton();
};
ASSERT_SIZE(DialogMenu, 0x13C);