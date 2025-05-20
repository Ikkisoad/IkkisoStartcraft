#pragma once
#include <BWAPI.h>
#include "../BWEM/bwem.h"  

namespace BasesTools {
	void Initialize();
	void FindExpansions();
	std::vector<const BWEM::Base*> GetAccessibleNeighborBases(const BWAPI::TilePosition& tilePos);
	void DrawBases(const std::vector<const BWEM::Base*>& bases, BWAPI::Color color);
}