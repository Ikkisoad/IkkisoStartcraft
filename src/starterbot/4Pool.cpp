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

			//TODO Units still targeting lethal units
            // If there are enemies nearby, micro; otherwise, attack
            Units unitsInstance;
            auto enemies = unitsInstance.GetNearbyEnemyUnits(unit, 640);
            if (!enemies.empty()) {
                Micro::SmartAvoidLethalAndAttackNonLethal(unit);
            } else {
                if (!BasesTools::IsAreaEnemyBase(unit->getPosition(), 3)) {
                    attack();
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

void FourPool::attack() {
    const BWAPI::Position enemyBase = BasesTools::GetEnemyBasePosition();
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    for (auto& unit : myUnits) {
        if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
            // If there are enemies nearby, use micro logic
            Units unitsInstance;
            auto enemies = unitsInstance.GetNearbyEnemyUnits(unit, 640);
            if (!enemies.empty()) {
                Micro::SmartAvoidLethalAndAttackNonLethal(unit);
                continue;
            }

            if (enemyBase == BWAPI::Positions::None) {
                Micro::ScoutAndWander(unit);
            } else {
                if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount()) { continue; }
                if (BasesTools::IsAreaEnemyBase(unit->getPosition(), 3)) {
                    Units::AttackNearestNonLethalEnemyUnit(unit);
                } else {
                    unit->attack(enemyBase);
                }
            }
        }
    }
}