#pragma once

#include "Menu.hpp"

class LevelUpMenu : public Menu
{
public:
	LevelUpMenu();
	~LevelUpMenu();

	enum Buttons
	{
		kLevelUp_Reset = 0x6,
		kLevelUp_Done = 0x7,
		kLevelUp_Back = 0x8,
		kLevelUp_Skill = 0xB,
		kLevelUp_Perk = 0xC,
		kLevelUp_Minus = 0xD,
		kLevelUp_Plus = 0xE,
	};

	enum Pages
	{
		kSkillSelection = 0,
		kPerkSelection = 1,
		kCloseMenu = 999  // (any value >= 2 could work)
	};

	uint32_t						currentPage; // 0 for skills, 1 for perks
	TileText* tileTitle;
	TileImage* tileSkills;
	TileImage* tilePerks;
	TileImage* tileSelectionIcon;
	TileText* tileSelectionText;
	TileText* tilePointCounter;
	TileImage* tileBtnReset;
	TileImage* tileBtnContinue;
	TileImage* tileBtnBack;
	TileImage* tileStatsIconBadge;
	uint32_t						numAssignedSkillPoints;
	uint32_t						numAssignedPerks;
	uint32_t						numSkillPointsToAssign;
	uint32_t						numPerksToAssign;
	ListBox<Index>		skillListBox;
	ListBox<BGSPerk>			perkListBox;
	TList<BGSPerk>				availablePerks; // perks to show in the perk listBox

	void SetCurrentPage(Pages newPage) { ThisCall(0x785830, this, newPage); }

	static LevelUpMenu* GetSingleton() { return *reinterpret_cast<LevelUpMenu**>(0x11D9FDC); }
};

ASSERT_SIZE(LevelUpMenu, 0xCC);