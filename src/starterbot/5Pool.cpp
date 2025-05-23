#include "5Pool.h"
#include "Tools.h"
#include "../../visualstudio/BasesTools.h"
#include "micro.h"

FivePool& FivePool::Instance() {
    static FivePool instance;
    return instance;
}

FivePool::FivePool() : builtExtraDrone(false) {}
//FivePool::FivePool() : zerglingRush(false) {}

void FivePool::Execute() {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    // Build one extra drone at the start (5 pool)
    if (!builtExtraDrone) {
        int droneCount = Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Drone, myUnits);
        if (droneCount < 5 && BWAPI::Broodwar->self()->minerals() >= 50) {
            Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Drone);
            return;
        }
        if (droneCount >= 5) {
            builtExtraDrone = true;
        }
    }

    // Now proceed with 4 pool logic
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }

    if (Tools::GetUnitOfType(BWAPI::UnitTypes::Zerg_Spawning_Pool)/* && Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Zergling, myUnits) < 2*/) Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Zergling);

    for (auto& unit : myUnits) {
        if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord) {
            Micro::ScoutAndWander(unit);
            continue;
        }
        if (unit->getType().isWorker()) {
            Micro::SmartGatherMinerals(unit);
            continue;
        }
        if (!unit->getType().isBuilding()) {
            // Only micro if the unit is not busy with something else
            if (unit->isMorphing() || unit->isBurrowed() || unit->isLoaded()) continue;

            // Use spatial queries for efficiency
            auto enemies = unit->getUnitsInRadius(640, BWAPI::Filter::IsEnemy && BWAPI::Filter::Exists);
            if (!enemies.empty()) {
                // Check for nearest offensive enemy unit
                BWAPI::Unit nearestOffensive = nullptr;
                int minDist = 645;
                int enemyCount = 0;
                for (auto enemy : enemies) {
                    if (!enemy || !enemy->exists()) continue;
                    // Offensive: can attack, not worker, not building
                    if (enemy->getType().canAttack() && !enemy->getType().isWorker() && !enemy->getType().isBuilding()) {
                        int dist = unit->getDistance(enemy);
                        if (dist < minDist) {
                            minDist = dist;
                            nearestOffensive = enemy;
                        }
                        enemyCount++;
                    }
                }
                // Count nearby allies (excluding buildings and workers)
                int allyCount = 0;
                auto alliesNearby = unit->getUnitsInRadius(96, BWAPI::Filter::IsAlly && !BWAPI::Filter::IsWorker && !BWAPI::Filter::IsBuilding);
                allyCount = static_cast<int>(alliesNearby.size());
                // If we have at least 2x as many allies as offensive enemies, attack the nearest offensive enemy
                if (nearestOffensive && allyCount >= enemyCount * 2 && enemyCount > 0 && false) {
                    Micro::SmartAttackUnit(unit, nearestOffensive);
                } else {
                    Micro::SmartAvoidLethalAndAttackNonLethal(unit);
                }
            } else {
                if (!BasesTools::IsAreaEnemyBase(unit->getPosition(), 3)) {
                    Micro::attack();
                }
            }
        }
    }
}

void FivePool::OnUnitCreate(BWAPI::Unit unit) {
    // Optionally, add extractor trick or other logic here
}

void FivePool::onUnitComplete(BWAPI::Unit unit) {
    // Optionally, add logic for when units complete
}