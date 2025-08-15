#pragma once

#include "Menu.hpp"
#include "ListBox.hpp"
#include "InventoryChanges.hpp"

class TileRect;
class TileImage;
class TileText;

class ItemModMenu : public Menu {
public:
	ItemModMenu();
	~ItemModMenu();
	union
	{
		Tile* pTiles[14];
		struct
		{
			TileRect* pTileItemName;
			TileImage* pTileItemModList;
			TileRect* pTileLeftBracket;
			TileImage* pTileItemIcon;
			TileRect* pTileItemHealth;
			TileRect* pTileItemStat;
			TileText* pTileHealthImprovementText;
			TileText* pTileStatImprovementText;
			TileText* pTileChooseItemText;
			TileRect* pTileModItemHealth;
			TileRect* pTileModItemStat;
			TileImage* pTileItemModButton;
			TileImage* pTileCancelButton;
			TileText* pTileModDesc;
		};
	};
	MenuItemEntryList kItemModList;

	static ItemModMenu* GetSingleton() { return *reinterpret_cast<ItemModMenu**>(0x11D9F54); };
	static InventoryChanges* GetTarget() { return *reinterpret_cast<InventoryChanges**>(0x11D9F58); };
};

ASSERT_SIZE(ItemModMenu, 0x90);