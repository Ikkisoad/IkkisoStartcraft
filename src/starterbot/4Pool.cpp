#include "4Pool.h"
#include "Tools.h"
#include "../../visualstudio/BasesTools.h"
#include "../../BWEM/bwem.h"  

// Singleton instance
FourPool& FourPool::Instance() {
    static FourPool instance;
    return instance;
}

bool zerglingRush = false;

void FourPool::Execute() {
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }

    if (zerglingRush) {
        Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Zergling);
        attack();
    }
}

void FourPool::OnUnitCreate(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Extractor, 1, unit->getTilePosition());
    }
}

void FourPool::onUnitComplete(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
        zerglingRush = true;
    }
}

void FourPool::attack() {
    BWAPI::Position enemyBase = BasesTools::GetEnemyBasePosition();
    if (enemyBase == BWAPI::Positions::None) {
        // fallback: attack nearest enemy unit as before
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
                BWAPI::Unit nearestEnemy = nullptr;
                int minDistance = std::numeric_limits<int>::max();
                for (auto enemyUnit : BWAPI::Broodwar->getAllUnits()) {
                    if (enemyUnit->getPlayer() != BWAPI::Broodwar->self() && enemyUnit->getPlayer() != BWAPI::Broodwar->neutral()) {
                        int distance = unit->getDistance(enemyUnit);
                        if (distance < minDistance) {
                            minDistance = distance;
                            nearestEnemy = enemyUnit;
                        }
                    }
                }
                if (nearestEnemy) {
                    if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || !unit->isIdle()) { continue; }
                    unit->attack(nearestEnemy->getPosition());
                }
            }
        }
    } else {
        // attack the known enemy base position
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
                if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || !unit->isIdle()) { continue; }
                
                if (BWAPI::Broodwar->isExplored(BWAPI::TilePosition(enemyBase))) {
                    unit->attack(BasesTools::FindUnexploredStarterPosition());
                } else {
                    // If the enemy base is not visible, attack the position instead
                    unit->move(enemyBase);
				}
            }
        }
    }
}