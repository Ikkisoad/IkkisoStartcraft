#include "Units.h"
#include <BWAPI.h>
#include "micro.h"
#include <limits>
#include <vector>

// Helper function to determine if a unit is offensive
static bool isOffensiveUnit(BWAPI::Unit unit) {
    if (!unit) return false;
    // Not a worker, not a building, and can attack
    return !unit->getType().isWorker() &&
           !unit->getType().isBuilding() &&
           (unit->getType().groundWeapon() != BWAPI::WeaponTypes::None ||
            unit->getType().airWeapon() != BWAPI::WeaponTypes::None);
}

// Returns the nearest enemy unit to the given unit, prioritizing offensive > worker > building
BWAPI::Unit Units::GetNearestEnemyUnit(BWAPI::Unit unit) {
    if (!unit || !unit->exists()) return nullptr;

    std::vector<BWAPI::Unit> offensiveUnits;
    std::vector<BWAPI::Unit> workerUnits;
    std::vector<BWAPI::Unit> buildingUnits;

    for (auto enemyUnit : BWAPI::Broodwar->getAllUnits()) {
        if (!enemyUnit->exists()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->self()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->neutral()) continue;
        if (!enemyUnit->isVisible()) continue;

        if (isOffensiveUnit(enemyUnit)) {
            offensiveUnits.push_back(enemyUnit);
        } else if (enemyUnit->getType().isWorker()) {
            workerUnits.push_back(enemyUnit);
        } else if (enemyUnit->getType().isBuilding()) {
            buildingUnits.push_back(enemyUnit);
        }
    }

    auto getNearest = [&](const std::vector<BWAPI::Unit>& units) -> BWAPI::Unit {
        BWAPI::Unit nearest = nullptr;
        int minDistance = std::numeric_limits<int>::max();
        for (auto u : units) {
            int distance = unit->getDistance(u);
            if (distance < minDistance) {
                minDistance = distance;
                nearest = u;
            }
        }
        return nearest;
    };

    if (!offensiveUnits.empty())
        return getNearest(offensiveUnits);
    if (!workerUnits.empty())
        return getNearest(workerUnits);
    if (!buildingUnits.empty())
        return getNearest(buildingUnits);

    return nullptr;
}

// Returns the nearest enemy unit to the given unit, prioritizing offensive > worker > building,
// but if the unit's HP is less than or equal to the enemy's attack damage, returns nullptr (should flee)
BWAPI::Unit Units::GetNearestThreateningEnemyUnitOrFlee(BWAPI::Unit unit) {
    if (!unit || !unit->exists()) return nullptr;

    std::vector<BWAPI::Unit> offensiveUnits;
    std::vector<BWAPI::Unit> workerUnits;
    std::vector<BWAPI::Unit> buildingUnits;

    for (auto enemyUnit : BWAPI::Broodwar->getAllUnits()) {
        if (!enemyUnit->exists()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->self()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->neutral()) continue;
        if (!enemyUnit->isVisible()) continue;

        if (isOffensiveUnit(enemyUnit)) {
            offensiveUnits.push_back(enemyUnit);
        } else if (enemyUnit->getType().isWorker()) {
            workerUnits.push_back(enemyUnit);
        } else if (enemyUnit->getType().isBuilding()) {
            buildingUnits.push_back(enemyUnit);
        }
    }

    auto getNearest = [&](const std::vector<BWAPI::Unit>& units) -> BWAPI::Unit {
        BWAPI::Unit nearest = nullptr;
        int minDistance = std::numeric_limits<int>::max();
        for (auto u : units) {
            int distance = unit->getDistance(u);
            if (distance < minDistance) {
                minDistance = distance;
                nearest = u;
            }
        }
        return nearest;
    };

    BWAPI::Unit target = nullptr;
    if (!offensiveUnits.empty())
        target = getNearest(offensiveUnits);
    else if (!workerUnits.empty())
        target = getNearest(workerUnits);
    else if (!buildingUnits.empty())
        target = getNearest(buildingUnits);

    if (target) {
        // Get the enemy's ground weapon damage (simplified: only first attack, not upgrades)
        int enemyDamage = 0;
        BWAPI::WeaponType weapon = target->getType().groundWeapon();
        if (weapon != BWAPI::WeaponTypes::None) {
            enemyDamage = weapon.damageAmount();
        }
        // If our unit's HP is less than or equal to the enemy's attack, we should flee
        if (unit->getHitPoints() <= enemyDamage && enemyDamage > 0) {
            return nullptr;
        }
    }

    return target;
}

std::vector<BWAPI::Unit> Units::GetNearbyEnemyUnits(BWAPI::Unit unit, int radius) {
    std::vector<BWAPI::Unit> result;
    if (!unit) return result;

    auto enemies = BWAPI::Broodwar->enemy();
    if (!enemies) return result;

    for (auto enemyUnit : enemies->getUnits()) {
        if (!enemyUnit || !enemyUnit->exists()) continue;
        if (unit->getDistance(enemyUnit) <= radius) {
            result.push_back(enemyUnit);
        }
    }
    return result;
}

// Attacks the nearest enemy unit to the given unit, if any
bool Units::AttackNearestEnemyUnit(BWAPI::Unit unit) {
    BWAPI::Unit nearestEnemy = GetNearestEnemyUnit(unit);
    if (nearestEnemy) {
        Attack(unit, nearestEnemy->getPosition());
        return true;
    }
    return false;
}

// Attacks the nearest non-lethal enemy unit to the given unit, if any.
// If all nearby enemies are lethal, does nothing.
bool Units::AttackNearestNonLethalEnemyUnit(BWAPI::Unit unit) {
    if (!unit || !unit->exists()) return false;

    std::vector<BWAPI::Unit> nonLethalUnits;
    std::vector<BWAPI::Unit> lethalUnits;

    for (auto enemyUnit : BWAPI::Broodwar->getAllUnits()) {
        if (!enemyUnit->exists()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->self()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->neutral()) continue;
        if (!enemyUnit->isVisible()) continue;
        if (enemyUnit->getType().isBuilding()) continue;

        // Determine if this enemy is lethal to us
        BWAPI::WeaponType weapon = unit->getType().isFlyer() ? enemyUnit->getType().airWeapon() : enemyUnit->getType().groundWeapon();
        int damage = weapon.damageAmount();
        int unitHP = unit->getHitPoints() + unit->getShields();
        bool isLethal = (damage > 0 && damage * 2 >= unitHP);

        if (isLethal) {
            lethalUnits.push_back(enemyUnit);
        } else {
            nonLethalUnits.push_back(enemyUnit);
        }
    }

    auto getNearest = [&](const std::vector<BWAPI::Unit>& units) -> BWAPI::Unit {
        BWAPI::Unit nearest = nullptr;
        int minDistance = std::numeric_limits<int>::max();
        for (auto u : units) {
            int distance = unit->getDistance(u);
            if (distance < minDistance) {
                minDistance = distance;
                nearest = u;
            }
        }
        return nearest;
    };

    // Prefer attacking the nearest non-lethal enemy
    BWAPI::Unit target = getNearest(nonLethalUnits);
    if (target) {
        Micro::SmartAttackUnit(unit, target);
        return true;
    }

    // If no non-lethal enemies, do nothing (ignore lethal enemies)
    return false;
}

void Units::Attack(BWAPI::Unit unit, BWAPI::Position pos) {
	if (unit->getLastCommand().getTargetPosition() == pos) return;
    if (unit->getLastCommandFrame() < BWAPI::Broodwar->getFrameCount()) {
        unit->attack(pos);
	}
}

bool Units::isLethalTo(BWAPI::Unit myUnit, BWAPI::Unit enemy) {
    if (!myUnit || !enemy) return false;
    BWAPI::WeaponType weapon = myUnit->getType().isFlyer() ? enemy->getType().airWeapon() : enemy->getType().groundWeapon();
    int damage = weapon.damageAmount();
    int unitHP = myUnit->getHitPoints() + myUnit->getShields();
    return (damage > 0 && damage * 2 >= unitHP);
}
