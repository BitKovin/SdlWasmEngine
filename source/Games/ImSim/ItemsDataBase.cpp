#include "ItemsDataBase.h"

#include <Helpers/CSVParser.h>

#include <FileSystem/FileSystem.h>

#include <algorithm> // for std::replace
#include <string>    // for std::stoi, though already included indirectly

#include <Logger.hpp>

void ItemsDataBase::LoadItemsDataBase()
{
	auto files = FileSystemEngine::GetFilesInPath("GameData/tables/items/");

	for (auto& file : files)
	{
		loadFromCSV("GameData/tables/items/" + file);
	}

	//itemsMap = itemsMap;

}

ItemDbEntry ItemsDataBase::GetItemData(const std::string& itemID)
{
	if (itemsMap.count(itemID))
		return itemsMap[itemID];
	else
	{
		Logger::Log("Item ID not found in database: " + itemID);
		return ItemDbEntry(); // Return empty entry if not found
	}
}

void ItemsDataBase::loadFromCSV(const std::string& csvText)
{

	std::string text = FileSystemEngine::ReadFile(csvText);

	text.erase(0, text.find_first_not_of("\r\n"));
	text.erase(text.find_last_not_of("\r\n") + 1);


	CSVParser parser(text);

	for (size_t row = 1; row < parser.rowCount(); ++row) {

		ItemDbEntry entry;

		entry.itemID = parser.at(row, 0).trimmed();
		entry.itemName = parser.at(row, 1).trimmed();
		entry.description = parser.at(row, 2).trimmed();
		entry.iconPath = parser.at(row, 3).trimmed();
		entry.modelPath = parser.at(row, 4).trimmed();

		std::string destroyStr = parser.at(row, 5).trimmed();
		entry.destroyOnUse = (destroyStr == "TRUE" || destroyStr == "true" || destroyStr == "1" || destroyStr == "t");

		std::string maxStr = parser.at(row, 6).trimmed();
		entry.maxStackSize = std::stoi(maxStr);

		entry.weaponClassName = parser.at(row, 7).trimmed();
		entry.weaponOffhandClassName = parser.at(row, 8).trimmed();

		std::string offhandCompatStr = parser.at(row, 9).trimmed();
		entry.offhandCompatible = (offhandCompatStr == "TRUE" || offhandCompatStr == "true" || offhandCompatStr == "1" || offhandCompatStr == "t");
		entry.interactionEntityClassname = parser.at(row, 10).trimmed();

		if (entry.weaponOffhandClassName.empty() == false)
		{
			if (entry.weaponClassName.empty())
			{
				entry.itemType = InventoryItemType::OffhandWeapon;
			}
			else
			{
				entry.itemType = InventoryItemType::DualWeapon;
			}
		}

		if (entry.interactionEntityClassname.empty() == false)
		{
			entry.itemType = InventoryItemType::CustomLogic;
		}

		itemsMap[entry.itemID] = entry;

	}
}