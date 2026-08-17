#pragma once


#include <string>

#include "LevelSaveData.h"


using namespace std;

class LevelSaveSystem
{
public:

	static LevelSaveData SaveLevelToData();
	static void LoadLevelFromData(LevelSaveData data);
	static void SaveLevelToFile(const string& saveName);
	static void LoadLevelFromFile(const string& saveName);

	static LevelSaveData pendingSave;

	static void InitPersistence();

private:

	static inline string saveDataPath = "SaveData/saves/";

};

