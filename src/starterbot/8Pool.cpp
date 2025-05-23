#include "8Pool.h"
#include "Tools.h"
#include "../../visualstudio/BasesTools.h"
#include "micro.h"

EightPool& EightPool::Instance() {
    static EightPool instance;
    return instance;
}

void EightPool::onStart() {
    builtEightDrones = false;
}

void EightPool::Execute() {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    // Build up to 8 drones before pool
    if (!builtEightDrones) {
        int droneCount = Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Drone, myUnits, true);
        if (droneCount < 8 && BWAPI::Broodwar->self()->minerals() >= 50) {
            Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Drone);
            return;
        }
        if (droneCount >= 8) {
            builtEightDrones = true;
        }
    }

    // Build spawning pool as soon as possible
    if (BWAPI::Broodwar->self()->minerals() >= 200) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }

    // Train zerglings when pool is done
    if (Tools::GetUnitOfType(BWAPI::UnitTypes::Zerg_Spawning_Pool)) {
        Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Zergling);

        if (BWAPI::Broodwar->self()->minerals() >= 350) {
            Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Hatchery, 0, BWAPI::Broodwar->self()->getStartLocation());
        }
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

void EightPool::OnUnitCreate(BWAPI::Unit unit) {
    // Optional: extractor trick or other logic
}

void EightPool::onUnitComplete(BWAPI::Unit unit) {
    // Optional: logic for completed units
}