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
    //TODO If in enemy base area, attack nearest enemy unit
    if (enemyBase == BWAPI::Positions::None) {
		const auto unexploredStartingPosition = BasesTools::FindUnexploredStarterPosition();
        // fallback: attack nearest enemy unit as before
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
                Units::Attack(unit, unexploredStartingPosition);
            }
        }
    } else {
        // attack the known enemy base position
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
                if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount()/* || !unit->isIdle()*/) { continue; }
                
                if (BasesTools::IsAreaEnemyBase(unit->getPosition())) {
                    Units::AttackNearestEnemyUnit(unit);
                //} else if (unit->isIdle()) {
                //    Units::Attack(unit, enemyBase);
                } else if (unit->isIdle()) {
                    if (!Units::AttackNearestEnemyUnit(unit)) {
						Micro::ScoutAndWander(unit);
                    }
                }
            }
        }
    }
}