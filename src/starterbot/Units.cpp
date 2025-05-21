#include "Units.h"
#include <BWAPI.h>
#include <limits>

// Attacks the nearest enemy unit to the given unit, if any
void Units::AttackNearestEnemyUnit(BWAPI::Unit unit) {
    if (!unit || !unit->exists()) return;

    BWAPI::Unit nearestEnemy = nullptr;
    int minDistance = std::numeric_limits<int>::max();

    for (auto enemyUnit : BWAPI::Broodwar->getAllUnits()) {
        if (!enemyUnit->exists()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->self()) continue;
        if (enemyUnit->getPlayer() == BWAPI::Broodwar->neutral()) continue;
        if (!enemyUnit->isVisible()) continue;

        int distance = unit->getDistance(enemyUnit);
        if (distance < minDistance) {
            minDistance = distance;
            nearestEnemy = enemyUnit;
        }
    }

    if (nearestEnemy) {
        Attack(unit, nearestEnemy->getPosition());
    }
}

void Units::Attack(BWAPI::Unit unit, BWAPI::Position pos) {
	if (unit->getLastCommand().getTargetPosition() == pos) return;
    if (unit->getLastCommandFrame() < BWAPI::Broodwar->getFrameCount()) {
        unit->attack(pos);
	}
}
