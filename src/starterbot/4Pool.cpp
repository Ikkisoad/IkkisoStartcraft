#include "4Pool.h"
#include "Tools.h"

// Singleton instance
FourPool& FourPool::Instance() {
    static FourPool instance;
    return instance;
}

bool zerglingRush = false;

void FourPool::Execute() {
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }

    if (zerglingRush) {
        Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Zergling);
    }
}

void FourPool::OnUnitCreate(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Extractor, 1, unit->getTilePosition());
    }
}

void FourPool::onUnitComplete(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {

    }
}