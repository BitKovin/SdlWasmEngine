#pragma once

#include <map>
#include <string>

// Item type defines what happens when the item is selected
enum class InventoryItemType
{
	MainWeapon,      // Equips to main weapon slot only
	OffhandWeapon,   // Equips to offhand slot only
	DualWeapon,      // Equips to both main and offhand slots
	CustomLogic      // Runs custom logic when selected (for future items like consumables, tools, etc.)
};

struct ItemDbEntry
{
	std::string itemID;
	std::string itemName;
	std::string description;
	std::string iconPath;
	std::string modelPath;
	
	bool destroyOnUse = false; // Whether the item should be removed from inventory when used (for consumables, etc.)
	int maxStackSize = 1; // Maximum stack size for this item (for stackable items)

	std::string weaponClassName;
	std::string weaponOffhandClassName; // For dual weapons or offhand items
	std::string interactionEntityClassname; // For items that spawn an entity when used (like consumables, etc.)
	
	InventoryItemType itemType = InventoryItemType::MainWeapon;
};

class ItemsDataBase
{
public:

	static void LoadItemsDataBase();

	static ItemDbEntry GetItemData(const std::string& itemID);

private:

	static void loadFromCSV(const std::string& csvText);

	static inline std::map<std::string, ItemDbEntry> itemsMap;

};

