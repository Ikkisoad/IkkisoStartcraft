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

void FivePool::onStart() {
    builtExtraDrone = false;
}

void FivePool::Execute() {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    // Send our idle workers to mine minerals so they don't just stand there
    Micro::sendIdleWorkersToMinerals();
    // Build one extra drone at the start (5 pool)
    if (!builtExtraDrone) {
        int droneCount = Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Drone, myUnits, true);
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

    // Train zerglings when pool is done
    if (Tools::GetUnitOfType(BWAPI::UnitTypes::Zerg_Spawning_Pool)) {
        Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Zergling);

        if (BWAPI::Broodwar->self()->minerals() >= 350) {
            Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Hatchery, 0, BWAPI::Broodwar->self()->getStartLocation());
        }
    }

    Micro::BasicAttackAndScoutLoop(myUnits);
}

void FivePool::OnUnitCreate(BWAPI::Unit unit) {
    // Optionally, add extractor trick or other logic here
}

void FivePool::onUnitComplete(BWAPI::Unit unit) {
    // Optionally, add logic for when units complete
}