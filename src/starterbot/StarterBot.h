#pragma once

#include "MapTools.h"

#include <BWAPI.h>

class StarterBot
{
    MapTools m_mapTools;

	// helper functions to get you started with bot programming and learn the API
	void sendIdleWorkersToMinerals();
	void trainAdditionalWorkers();
	bool trainUnit(BWAPI::UnitType supplyProviderType);
	void buildAdditionalSupply();
	void drawDebugInformation();

public:

    StarterBot();

    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void buildOrder();
	bool buildBuilding(BWAPI::UnitType building, int limitAmount, BWAPI::TilePosition desiredPos);
	BWAPI::Unit getAvailableUnit(BWAPI::UnitType unitType);
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void attack();
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);
};