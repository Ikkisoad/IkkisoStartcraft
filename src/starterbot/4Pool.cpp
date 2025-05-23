#include "4Pool.h"
#include "Tools.h"
#include "../../visualstudio/BasesTools.h"
#include "../../BWEM/bwem.h"  
#include "../../starterbot/Units.cpp"
#include "micro.h"

// Singleton instance
FourPool& FourPool::Instance() {
    static FourPool instance;
    return instance;
}

bool zerglingRush = false;
int healThreshold = 22; // Set the threshold for healing
//TODO When the enemy base is already know units are wandering instead of attacking it
void FourPool::Execute() {
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    if (zerglingRush/* && Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Zergling, myUnits) < 2*/) Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Zergling);

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

bool FourPool::isLethalTo(BWAPI::Unit myUnit, BWAPI::Unit enemy) {
    BWAPI::WeaponType weapon = myUnit->getType().isFlyer() ? enemy->getType().airWeapon() : enemy->getType().groundWeapon();
    int damage = weapon.damageAmount();
    int unitHP = myUnit->getHitPoints() + myUnit->getShields();
    return (damage > 0 && damage * 2 >= unitHP);
}

void FourPool::onStart() {
    zerglingRush = false;
}