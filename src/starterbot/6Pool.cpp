#include "6Pool.h"
#include "Tools.h"
#include "../../visualstudio/BasesTools.h"
#include "micro.h"

SixPool& SixPool::Instance() {
    static SixPool instance;
    return instance;
}

void SixPool::onStart() {
    builtSixDrones = false;
}

void SixPool::Execute() {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    // Build up to 6 drones before pool
    if (!builtSixDrones) {
        int droneCount = Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Drone, myUnits, true);
        if (droneCount < 6 && BWAPI::Broodwar->self()->minerals() >= 50) {
            Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Drone);
            return;
        }
        if (droneCount >= 6) {
            builtSixDrones = true;
        }
    }

    // Build spawning pool as soon as possible
    if (BWAPI::Broodwar->self()->minerals() >= 200) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }

    // Train zerglings when pool is done
    if (Tools::GetUnitOfType(BWAPI::UnitTypes::Zerg_Spawning_Pool)) {
        Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Zergling);
    }

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
            if (unit->isMorphing() || unit->isBurrowed() || unit->isLoaded()) continue;

            auto enemies = unit->getUnitsInRadius(640, BWAPI::Filter::IsEnemy && BWAPI::Filter::Exists);
            if (!enemies.empty()) {
                Micro::SmartAvoidLethalAndAttackNonLethal(unit);
            } else {
                if (!BasesTools::IsAreaEnemyBase(unit->getPosition(), 3)) {
                    Micro::attack();
                }
            }
        }
    }
}

void SixPool::OnUnitCreate(BWAPI::Unit unit) {
    // Optional: extractor trick or other logic
}

void SixPool::onUnitComplete(BWAPI::Unit unit) {
    // Optional: logic for completed units
}