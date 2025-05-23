#pragma once
#include <BWAPI.h>

class Units {
public:
    static BWAPI::Unit GetNearestEnemyUnit(BWAPI::Unit unit);
    static BWAPI::Unit GetNearestThreateningEnemyUnitOrFlee(BWAPI::Unit unit);
    // Attacks the nearest enemy unit to the given unit, if any
    static bool AttackNearestEnemyUnit(BWAPI::Unit unit);
    static bool AttackNearestNonLethalEnemyUnit(BWAPI::Unit unit);
    static void Attack(BWAPI::Unit unit, BWAPI::Position pos);
    std::vector<BWAPI::Unit> GetNearbyEnemyUnits(BWAPI::Unit unit, int radius);
    static bool isLethalTo(BWAPI::Unit myUnit, BWAPI::Unit enemy);
};
