#pragma once
#include <BWAPI.h>
#include "../BWEM/bwem.h"  

namespace BasesTools {
	void Initialize();
	void FindExpansionsV1();
	void FindExpansions();
	void DrawExpansions();
	void FindThirdBase();
	BWAPI::Position ReturnBasePosition(BWEM::Area area);
	std::vector<const BWEM::Base*> GetAccessibleNeighborBases(const BWAPI::TilePosition& tilePos);
	void DrawBases(const std::vector<const BWEM::Base*>& bases, BWAPI::Color color);
	BWAPI::TilePosition GetMainBasePosition();
	BWAPI::TilePosition GetNaturalBasePosition();
	BWAPI::TilePosition GetThirdBasePosition();
	BWAPI::TilePosition GetFourthBasePosition();
	BWAPI::TilePosition GetFifthBasePosition();
}