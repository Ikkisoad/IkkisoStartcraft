#include "BasesTools.h"
#include <vector>  
#include <algorithm>
#include <BWAPI.h>  
#include "../BWEM/bwem.h"  
#include "../src/starterbot/Tools.h"

auto& bwem = BWEM::Map::Instance();  

BWAPI::TilePosition mainBasePosition;
BWAPI::TilePosition naturalBasePosition;
BWAPI::TilePosition thirdBasePosition;
BWAPI::TilePosition fourthBasePosition;
BWAPI::TilePosition fifthBasePosition;
//TODO remove destroyed bases
// Store all known enemy base positions
std::vector<BWAPI::Position> enemyBasePositions;

// Store all base positions for easy reference
static std::vector<BWAPI::Position> allBasePositions;

namespace BasesTools {  

    // Adds a new enemy base position if not already present
    void SetEnemyBasePosition(const BWAPI::Position& pos) {
        if (pos == BWAPI::Positions::None) return;
        auto it = std::find(enemyBasePositions.begin(), enemyBasePositions.end(), pos);
        if (it == enemyBasePositions.end()) {
            enemyBasePositions.push_back(pos);
        }
    }

    // Returns the first known enemy base position, or BWAPI::Positions::None if none exist
    BWAPI::Position GetEnemyBasePosition() {
        if (!enemyBasePositions.empty()) {
            return enemyBasePositions.front();
        }
        return BWAPI::Positions::None;
    }

    // Removes a specific enemy base position if it exists
    void RemoveEnemyBasePosition(const BWAPI::Position pos) {
        auto it = std::remove(enemyBasePositions.begin(), enemyBasePositions.end(), pos);
        if (it != enemyBasePositions.end()) {
            enemyBasePositions.erase(it, enemyBasePositions.end());
        }
    }

    // Returns all known enemy base positions
    const std::vector<BWAPI::Position>& GetAllEnemyBasePositions() {
        return enemyBasePositions;
    }

    bool BasesTools::IsAreaEnemyBase(BWAPI::Position position) {
        auto area = bwem.GetArea(BWAPI::TilePosition(position));
        if (!area) return false;

        for (auto& unit : BWAPI::Broodwar->getUnitsOnTile(BWAPI::TilePosition(position))) {
            if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) && unit->getType().isBuilding() && unit->exists()) {
                return true;
            }
        }
        return false;
    }

    void BasesTools::CacheBWEMBases() {
        allBasePositions.clear();
        for (auto area : bwem.Areas()) {
            for (auto& base : area.Bases()) {
                allBasePositions.push_back(BWAPI::Position(base.Location()));
            }
        }
    }

    const std::vector<BWAPI::Position>& BasesTools::GetAllBasePositions() {
        return allBasePositions;
    }

    void BasesTools::Initialize() {
        bwem.Initialize(BWAPI::BroodwarPtr);
        bwem.EnableAutomaticPathAnalysis();
        int baseCount = bwem.BaseCount();
        Tools::print(std::to_string(baseCount));
        enemyBasePositions.clear();
        mainBasePosition = BWAPI::TilePositions::None;
        naturalBasePosition = BWAPI::TilePositions::None;
        thirdBasePosition = BWAPI::TilePositions::None;
        fourthBasePosition = BWAPI::TilePositions::None;
        fifthBasePosition = BWAPI::TilePositions::None;
        CacheBWEMBases(); // <-- Add this line to cache base positions on initialization
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
            return BWAPI::Positions::None;
        }
        for (auto& base : area.Bases()) {
            BWAPI::Position pos(base.Location());
            return pos;
        }
	}

    std::vector<const BWEM::Base*> BasesTools::GetAccessibleNeighborBases(const BWAPI::TilePosition& tilePos) {
        std::vector<const BWEM::Base*> result;  
        auto area = BWEM::Map::Instance().GetNearestArea(tilePos);  
        if (!area) return result;  
        for (auto neighbor : area->AccessibleNeighbours()) {  
            for (const auto& base : neighbor->Bases()) {  
                result.push_back(&base);  
            }  
        }  
        return result;  
    }  

    void BasesTools::DrawBases(const std::vector<const BWEM::Base*>& bases, BWAPI::Color color) {
        for (const auto* base : bases) {  
            BWAPI::Position pos(base->Location());  
            BWAPI::Broodwar->drawCircleMap(pos, 32, color, true);  
        }  
    }

    BWAPI::Position BasesTools::FindUnexploredStarterPosition() {
        BWAPI::Position pos = BWAPI::Position(0, 0);
        for (const auto& tile : BWAPI::Broodwar->getStartLocations()) {
            if (tile != BWAPI::Broodwar->self()->getStartLocation() && !BWAPI::Broodwar->isExplored(tile)) {
                if (pos == BWAPI::Position(0, 0)) {
                    pos = BWAPI::Position(tile);
                    continue;
                } 
                auto testedTile = BWAPI::Position(tile);
                BWAPI::Position mainBase = BWAPI::Position(mainBasePosition);
                int testedDist = testedTile.getApproxDistance(mainBase);
                int posDist = pos.getApproxDistance(mainBase);
                if (testedDist < posDist) {
                    pos = testedTile;
                }
            }
        }
        // Return the first known enemy base position if no unexplored tile is found
        return (pos != BWAPI::Position(0, 0)) ? pos : GetEnemyBasePosition();
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

    std::vector<BWAPI::Position> BasesTools::GetBWEMBases() {
        std::vector<BWAPI::Position> basePositions;
        for (auto area : bwem.Areas()) {
            for (auto& base : area.Bases()) {
                BWAPI::Position pos(base.Location());
                basePositions.push_back(pos);
            }
        }
        return basePositions;
    }

    BWAPI::Position BasesTools::GetNearestBasePosition(const BWAPI::Position& position) {
        auto bases = GetBWEMBases();
        BWAPI::Position nearestBase = BWAPI::Positions::None;
        int minDistance = std::numeric_limits<int>::max();
        for (const auto& base : bases) {
            int distance = position.getApproxDistance(base);
            if (distance < minDistance) {
                minDistance = distance;
                nearestBase = base;
            }
        }
        return nearestBase;
    }

    void BasesTools::DrawAllBases(BWAPI::Color color) {
        for (const auto& pos : allBasePositions) {
            BWAPI::Broodwar->drawCircleMap(pos, 10, color, true);
        }
    }
}