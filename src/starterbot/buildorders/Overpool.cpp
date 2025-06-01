#include "Overpool.h"
#include "../Tools.h"
#include "../../../visualstudio/BasesTools.h"
#include "../micro.h"
#include "BuildOrderTools.h"

Overpool& Overpool::Instance() {
    static Overpool instance;
    return instance;
}

void Overpool::onStart() {
    Micro::SetMode(Micro::MicroMode::Defensive);
    BasesTools::SetOurBasePosition();
}
void Overpool::Execute() {
    auto myUnits = BWAPI::Broodwar->self()->getUnits();
    if (BWAPI::Broodwar->self()->supplyTotal() == 18 && BWAPI::Broodwar->self()->supplyUsed() < 18) {
        Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Drone);
    }
    else if (BWAPI::Broodwar->self()->supplyTotal() == BWAPI::Broodwar->self()->supplyUsed() && Tools::CountUnitOfType(BWAPI::UnitTypes::Zerg_Spawning_Pool) < 1) {
        if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Overlord, myUnits, true) < 2 && Tools::GetTotalSupply() == 18){
            Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Overlord);
        }
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }
    else if (BWAPI::Broodwar->self()->supplyUsed() < 22) {
        Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Drone);
    }
    else if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Zergling, myUnits, true) < 6) {
        Tools::MorphLarva(BWAPI::UnitTypes::Zerg_Zergling);
    }
    else if (BWAPI::Broodwar->self()->supplyUsed() == 28 && BWAPI::Broodwar->self()->supplyTotal() == 34) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Hatchery, 0, BasesTools::GetNextExpansionPosition());
    }
    else if (BWAPI::Broodwar->self()->supplyUsed() >= 26 && BWAPI::Broodwar->self()->supplyTotal() >= 34) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Hatchery, 0, BasesTools::GetNextExpansionPosition());
    }
    Micro::BasicAttackAndScoutLoop(myUnits);
}

void Overpool::OnUnitCreate(BWAPI::Unit unit) {
}

void Overpool::onUnitComplete(BWAPI::Unit unit) {
}