#include "6Pool.h"
#include "../Tools.h"
#include "../../../visualstudio/BasesTools.h"
#include "../micro.h"
#include "BuildOrderTools.h"

SixPool& SixPool::Instance() {
    static SixPool instance;
    return instance;
}

void SixPool::onStart() {
    builtSixDrones = false;
    Micro::SetMode(Micro::MicroMode::Aggressive);
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
    BuildOrderTools::PoolAllIn(myUnits);
}

void SixPool::OnUnitCreate(BWAPI::Unit unit) {
    // Optional: extractor trick or other logic
}

void SixPool::onUnitComplete(BWAPI::Unit unit) {
    // Optional: logic for completed units
}