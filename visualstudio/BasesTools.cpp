#include "BasesTools.h"
#include <vector>  
#include <BWAPI.h>  
#include "../BWEM/bwem.h"  
#include "../src/starterbot/Tools.h"

auto& bwem = BWEM::Map::Instance();  

namespace BasesTools {  

    void BasesTools::Initialize() {
        bwem.Initialize(BWAPI::BroodwarPtr);
        bwem.EnableAutomaticPathAnalysis();
        int baseCount = bwem.BaseCount();
        Tools::print(std::to_string(baseCount));
	}

    void BasesTools::FindExpansions() {
        auto bases = bwem.GetNearestArea(BWAPI::Broodwar->self()->getStartLocation())->AccessibleNeighbours();
        for (auto& base : bases) {
            for (auto& unit : base->Bases()) {
                BWAPI::Broodwar->drawBoxScreen(0, 0, 200, 100, BWAPI::Colors::Black, true);
                BWAPI::Position pos(unit.Location());
                BWAPI::Broodwar->drawCircleMap(pos, 32, BWAPI::Colors::Red, true);
            }
        }
    }

    // Returns a vector of all base locations neighboring the given tile position  
    std::vector<const BWEM::Base*> BasesTools::GetAccessibleNeighborBases(const BWAPI::TilePosition& tilePos) {
        std::vector<const BWEM::Base*> result;  
        auto area = BWEM::Map::Instance().GetNearestArea(tilePos);  
        if (!area) return result;  
        for (auto neighbor : area->AccessibleNeighbours()) {  
            for (const auto& base : neighbor->Bases()) { // Ensure 'base' is treated as a const reference  
                result.push_back(&base); // Use the address-of operator to pass a pointer  
            }  
        }  
        return result;  
    }  

    // Draws circles on the map for all given bases  
    void BasesTools::DrawBases(const std::vector<const BWEM::Base*>& bases, BWAPI::Color color = BWAPI::Colors::Red) {
        for (const auto* base : bases) {  
            BWAPI::Position pos(base->Location());  
            BWAPI::Broodwar->drawCircleMap(pos, 32, color, true);  
        }  
    }  
}