#pragma once

#include <BWAPI.h>

namespace Micro
{
    void SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
    void SmartMove(BWAPI::Unit unit, BWAPI::Position position);
    void SmartKiteTarget(BWAPI::Unit rangedUnit, BWAPI::Unit target);
    void SmartFleeUntilHealed(BWAPI::Unit meleeUnit, BWAPI::Unit enemyUnit);
    void ScoutAndWander(BWAPI::Unit scout);
    void SmartAvoidLethalAndAttackNonLethal(BWAPI::Unit unit);
}