#pragma once
#include <BWAPI.h>

class Units {
public:
    // Attacks the nearest enemy unit to the given unit, if any
    static void AttackNearestEnemyUnit(BWAPI::Unit unit);
    static void Attack(BWAPI::Unit unit, BWAPI::Position pos);
};
