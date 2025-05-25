#include "BuildOrderTools.h"
#include "../Tools.h"
#include "../micro.h"

void BuildOrderTools::PoolAllIn(const BWAPI::Unitset& myUnits) {
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