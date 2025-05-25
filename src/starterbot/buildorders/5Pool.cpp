#include "5Pool.h"
#include "../Tools.h"
#include "../../../visualstudio/BasesTools.h"
#include "../micro.h"
#include "BuildOrderTools.h"

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

    BuildOrderTools::PoolAllIn(myUnits);

}

void FivePool::OnUnitCreate(BWAPI::Unit unit) {
    // Optionally, add extractor trick or other logic here
}

void FivePool::onUnitComplete(BWAPI::Unit unit) {
    // Optionally, add logic for when units complete
}