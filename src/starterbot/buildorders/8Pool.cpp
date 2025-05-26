#include "8Pool.h"
#include "../Tools.h"
#include "../../../visualstudio/BasesTools.h"
#include "../micro.h"
#include "BuildOrderTools.h"

EightPool& EightPool::Instance() {
    static EightPool instance;
    return instance;
}

void EightPool::onStart() {
    builtEightDrones = false;
    builtExtractor = false;
    builtSpawningPool = false;
}

void EightPool::Execute() {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    // Build up to 8 drones before pool
    if (!builtEightDrones) {
        const int droneCount = Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Drone, myUnits, true);
        if (droneCount < 8 && BWAPI::Broodwar->self()->minerals() >= 50) {
            Tools::TrainUnit(BWAPI::UnitTypes::Zerg_Drone);
            return;
        }
        if (droneCount >= 8) {
            builtEightDrones = true;
        }
    }
    else {

        if (BWAPI::Broodwar->self()->gas() >= 100) {
            for (auto unit : myUnits) {
                if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
                    unit->upgrade(BWAPI::UpgradeTypes::Metabolic_Boost);
                }
                else if (unit->getType().isWorker() && unit->isGatheringGas()) {
					unit->gather(unit->getClosestUnit(BWAPI::Filter::IsMineralField));
                }
			}
		}

    }
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        if (!Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation())) {
            Tools::BuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::Broodwar->self()->getStartLocation());
        }
    }

    if (builtSpawningPool) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Extractor, 1, BWAPI::Broodwar->self()->getStartLocation());
        if (BWAPI::Broodwar->self()->supplyTotal() == BWAPI::Broodwar->self()->supplyUsed()) Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Overlord);
    }

    // Train zerglings when pool is done
    if (Tools::GetUnitOfType(BWAPI::UnitTypes::Zerg_Spawning_Pool)) {
        Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Zergling);

        if (builtExtractor && BWAPI::Broodwar->self()->minerals() >= 350) {
            Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Hatchery, 0, BWAPI::Broodwar->self()->getStartLocation());
        }
    }
    Micro::BasicAttackAndScoutLoop(myUnits);
}

void EightPool::OnUnitCreate(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Extractor, 1, BWAPI::Broodwar->self()->getStartLocation());
	}
}

void EightPool::onUnitComplete(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Extractor) {
        builtExtractor = true;
        int count = 0;
        for (auto drone : BWAPI::Broodwar->self()->getUnits()) {
            if (drone->getType() == BWAPI::UnitTypes::Zerg_Drone && count < 3) {
                drone->gather(unit);
                count++;
            }
        }
    }
}