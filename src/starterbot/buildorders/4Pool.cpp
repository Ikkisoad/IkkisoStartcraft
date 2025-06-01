#include "4Pool.h"
#include "../Tools.h"
#include "../../../visualstudio/BasesTools.h"
#include "../../../BWEM/bwem.h"  
#include "../../starterbot/Units.cpp"
#include "../micro.h"
#include "BuildOrderTools.h"

// Singleton instance
FourPool& FourPool::Instance() {
    static FourPool instance;
    return instance;
}

bool zerglingRush = false;
int healThreshold = 22; // Set the threshold for healing
//TODO When the enemy base is already know units are wandering instead of attacking it
void FourPool::Execute() {
    if (BWAPI::Broodwar->self()->minerals() >= 190) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1, BWAPI::Broodwar->self()->getStartLocation());
    }
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    BuildOrderTools::PoolAllIn(myUnits);
}

void FourPool::OnUnitCreate(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
        Tools::TryBuildBuilding(BWAPI::UnitTypes::Zerg_Extractor, 1, unit->getTilePosition());
    }
}

void FourPool::onUnitComplete(BWAPI::Unit unit) {
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
        zerglingRush = true;
    }
}

bool FourPool::isLethalTo(BWAPI::Unit myUnit, BWAPI::Unit enemy) {
    BWAPI::WeaponType weapon = myUnit->getType().isFlyer() ? enemy->getType().airWeapon() : enemy->getType().groundWeapon();
    int damage = weapon.damageAmount();
    int unitHP = myUnit->getHitPoints() + myUnit->getShields();
    return (damage > 0 && damage * 2 >= unitHP);
}

void FourPool::onStart() {
    zerglingRush = false;
    Micro::SetMode(Micro::MicroMode::Aggressive);
}