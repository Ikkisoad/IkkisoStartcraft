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

        // Loop through offensive units (Zerglings, Hydralisks, etc.)
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord && !unit->getType().isBuilding()) {
                // Validate if unit is healthy
				auto possibleTarget = Units::GetNearestThreateningEnemyUnitOrFlee(unit);
                if (BasesTools::GetEnemyBasePosition() != BWAPI::Positions::None && !possibleTarget) {
                    Micro::SmartFleeUntilHealed(unit, possibleTarget); // Call the new flee function
                } else {
                    attack(); // Run the attack method
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
                    Units::AttackNearestEnemyUnit(unit);
                }
            }
        }
    }
}