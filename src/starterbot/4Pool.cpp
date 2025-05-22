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

void FourPool::Execute() {
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }

    if (zerglingRush) {
        Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Zergling);

        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord) {
                Micro::ScoutAndWander(unit);
                continue;
            }
            if (!unit->getType().isWorker() && !unit->getType().isBuilding()) {
                // Only micro if the unit is not busy with something else
                if (unit->isMorphing() || unit->isBurrowed() || unit->isLoaded()) continue;

				//TODO Units still targeting lethal units
                // If there are enemies nearby, micro; otherwise, attack
                Units unitsInstance;
                auto enemies = unitsInstance.GetNearbyEnemyUnits(unit, 640);
                if (!enemies.empty()) {
                    Micro::SmartAvoidLethalAndAttackNonLethal(unit);
                } else {
                    if (!BasesTools::IsAreaEnemyBase(unit->getPosition())) {
                        attack();
                    }
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
            // Skip if in danger from a lethal enemy
            Units unitsInstance;
            auto enemies = unitsInstance.GetNearbyEnemyUnits(unit, 640);
            bool inDanger = false;
            for (auto& enemy : enemies) {
                if (Units::isLethalTo(unit, enemy)) {
                    inDanger = true;
                    break;
                }
            }
            if (inDanger) continue;

            if (enemyBase == BWAPI::Positions::None) {
                const auto unexploredStartingPosition = BasesTools::FindUnexploredStarterPosition();
                Units::Attack(unit, unexploredStartingPosition);
            } else {
                if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount()) { continue; }
                if (BasesTools::IsAreaEnemyBase(unit->getPosition())) {
                    Units::AttackNearestEnemyUnit(unit);
                } else {
                    if (!Units::AttackNearestEnemyUnit(unit)) {
                        Micro::ScoutAndWander(unit);
                    }
                }
            }
        }
    }
}