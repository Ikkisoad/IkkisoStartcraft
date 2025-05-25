#include "7Pool.h"
#include "../Tools.h"
#include "../../../visualstudio/BasesTools.h"
#include "../micro.h"
#include "BuildOrderTools.h"

SevenPool& SevenPool::Instance() {
    static SevenPool instance;
    return instance;
}

void SevenPool::onStart() {
    builtSevenDrones = false;
}

void SevenPool::Execute() {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    // Build up to 6 drones before pool
    if (!builtSevenDrones) {
        int droneCount = Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Drone, myUnits, true);
        if (droneCount < 7 && BWAPI::Broodwar->self()->minerals() >= 50) {
            Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Drone);
            return;
        }
        if (droneCount >= 7) {
            builtSevenDrones = true;
        }
    }

    BuildOrderTools::PoolAllIn(myUnits);
}

void SevenPool::OnUnitCreate(BWAPI::Unit unit) {
    // Optional: extractor trick or other logic
}

void SevenPool::onUnitComplete(BWAPI::Unit unit) {
    // Optional: logic for completed units
}