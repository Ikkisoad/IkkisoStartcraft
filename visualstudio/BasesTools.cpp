#include "BasesTools.h"
#include <vector>  
#include <BWAPI.h>  
#include "../BWEM/bwem.h"  
#include "../src/starterbot/Tools.h"

auto& bwem = BWEM::Map::Instance();  

BWAPI::TilePosition mainBasePosition;
BWAPI::TilePosition naturalBasePosition;
BWAPI::TilePosition thirdBasePosition;
BWAPI::TilePosition fourthBasePosition;
BWAPI::TilePosition fifthBasePosition;

// Add variable to store enemy base position
BWAPI::Position enemyBasePosition = BWAPI::Positions::None;

namespace BasesTools {  

    void SetEnemyBasePosition(const BWAPI::Position& pos) {
        enemyBasePosition = pos;
    }

    BWAPI::Position GetEnemyBasePosition() {
        return enemyBasePosition;
    }

    bool BasesTools::IsAreaEnemyBase(BWAPI::Position position) {
        auto area = bwem.GetArea(BWAPI::TilePosition(position));
        return false;
    }

    void BasesTools::Initialize() {
        bwem.Initialize(BWAPI::BroodwarPtr);
        bwem.EnableAutomaticPathAnalysis();
        int baseCount = bwem.BaseCount();
        Tools::print(std::to_string(baseCount));
	}

    void BasesTools::FindExpansionsV1() {
        auto bases = bwem.GetNearestArea(BWAPI::Broodwar->self()->getStartLocation())->AccessibleNeighbours();
        for (auto& base : bases) {
            for (auto& unit : base->Bases()) {
                BWAPI::Position pos(unit.Location());
                BWAPI::Broodwar->drawCircleMap(pos, 16, BWAPI::Colors::Green, true);
            }
        }
    }

    void BasesTools::FindExpansions() {
        BWAPI::Position mainBase(BWAPI::Broodwar->self()->getStartLocation());
        mainBasePosition = BWAPI::TilePosition(mainBase);
        auto bases = bwem.GetNearestArea(BWAPI::Broodwar->self()->getStartLocation())->AccessibleNeighbours();
        for (auto& base : bases) {
            for (auto& unit : base->Bases()) {
                BWAPI::TilePosition pos(unit.Location());
                naturalBasePosition = pos;
            }
        }
		Tools::print("Main Base Position: " + std::to_string(mainBasePosition.x) + ", " + std::to_string(mainBasePosition.y));
        Tools::print("Natural Base Position: " + std::to_string(naturalBasePosition.x) + ", " + std::to_string(naturalBasePosition.y));
    }

    void BasesTools::DrawExpansions() {
        // Fix the issue by ensuring the correct type is passed to `drawCircleMap`.  
        // `drawCircleMap` expects a `BWAPI::Position` type, but `mainBasePosition` is a `BWAPI::TilePosition`.  
        // Convert `BWAPI::TilePosition` to `BWAPI::Position` using `BWAPI::Position()` constructor.
        BWAPI::Broodwar->drawCircleMap(BWAPI::Position(mainBasePosition), 32, BWAPI::Colors::Red, true);
        BWAPI::Broodwar->drawCircleMap(BWAPI::Position(naturalBasePosition), 32, BWAPI::Colors::Red, true);
        //BWAPI::Broodwar->drawCircleMap(thirdBasePosition, 32, BWAPI::Colors::Red, true);
        //BWAPI::Broodwar->drawCircleMap(fourthBasePosition, 32, BWAPI::Colors::Red, true);
        //BWAPI::Broodwar->drawCircleMap(fifthBasePosition, 32, BWAPI::Colors::Red, true);
    }

    void BasesTools::FindThirdBase() {
        auto bases = bwem.GetNearestArea(BWAPI::Broodwar->self()->getStartLocation())->AccessibleNeighbours();
        for (auto& base : bases) {
            for (auto& unit : base->Bases()) {
                BWAPI::TilePosition pos(unit.Location());
                if (!thirdBasePosition) thirdBasePosition = pos;
            }
        }
	}

    BWAPI::Position BasesTools::ReturnBasePosition(BWEM::Area area) {
        if (area.Bases().size() <= 0) {
            return BWAPI::Positions::None; // Or handle the error appropriately
        }
        for (auto& base : area.Bases()) {
            BWAPI::Position pos(base.Location());
            return pos;
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

    BWAPI::Position BasesTools::FindUnexploredStarterPosition() {
        for (const auto& tile : BWAPI::Broodwar->getStartLocations()) {
            if (tile != BWAPI::Broodwar->self()->getStartLocation() && !BWAPI::Broodwar->isExplored(tile)) {
                return BWAPI::Position(tile);
                break;
            }
        }
		return enemyBasePosition; // Return the enemy base position if no unexplored tile is found
    }

    BWAPI::TilePosition BasesTools::GetMainBasePosition() {
        return mainBasePosition;
	}

    BWAPI::TilePosition BasesTools::GetNaturalBasePosition() {
        return naturalBasePosition;
    }
    BWAPI::TilePosition BasesTools::GetThirdBasePosition() {
        return thirdBasePosition;
    }
    BWAPI::TilePosition BasesTools::GetFourthBasePosition() {
        return fourthBasePosition;
    }
    BWAPI::TilePosition BasesTools::GetFifthBasePosition() {
        return fifthBasePosition;
    }
}