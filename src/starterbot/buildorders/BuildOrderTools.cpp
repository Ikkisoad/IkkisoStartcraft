#include "BuildOrderTools.h"
#include "../Tools.h"
#include "../micro.h"
#include "../Factory/BuildOrderFactory.h"

void BuildOrderTools::PoolAllIn(const BWAPI::Unitset& myUnits) {
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        if (!Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation())) {
			Tools::BuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::Broodwar->self()->getStartLocation());
        }
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