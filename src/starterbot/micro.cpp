#include "micro.h"
#include "Units.h"
#include "BWAPI.h" // Ensure this header is included for TILE_SIZE definition

void Micro::SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target)
{
    if (!attacker || !target) return;
    if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount()) return;
    if (attacker->getLastCommand().getTarget() == target) return;
    attacker->attack(target);
}

void Micro::SmartMove(BWAPI::Unit unit, BWAPI::Position position)
{
    if (!unit) return;
    if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount()) return;
    if (unit->getLastCommand().getTargetPosition() == position) return;

    // Check if the position is a walkable tile
    int tileX = position.x / 32;
    int tileY = position.y / 32;
    if (!BWAPI::Broodwar->isWalkable(tileX * 4, tileY * 4)) return;

    unit->move(position);
    BWAPI::Broodwar->drawCircleMap(BWAPI::Position(position), 64, BWAPI::Colors::Green, true);
}

void Micro::SmartKiteTarget(BWAPI::Unit rangedUnit, BWAPI::Unit target)
{
    if (!rangedUnit || !target) return;
    int weaponRange = rangedUnit->getType().groundWeapon().maxRange();
    if (rangedUnit->getDistance(target) > weaponRange)
    {
        SmartMove(rangedUnit, target->getPosition());
    }
    else
    {
        BWAPI::Position fleePosition = rangedUnit->getPosition() - (target->getPosition() - rangedUnit->getPosition());
        SmartMove(rangedUnit, fleePosition);
    }
}

void Micro::SmartFleeUntilHealed(BWAPI::Unit meleeUnit, BWAPI::Unit enemyUnit) {
    if (!meleeUnit) return;

    if (!enemyUnit) {
        enemyUnit = Units::GetNearestEnemyUnit(meleeUnit);
        if (!enemyUnit) return;
    }
    if (meleeUnit->getType().groundWeapon().maxRange() > 32 || enemyUnit->getType().groundWeapon().maxRange() > 32 || enemyUnit->getType().isBuilding()) return; // Only for melee vs melee

    // Calculate flee direction (opposite of enemy), fixed length (2 tiles)
    BWAPI::Position myPos = meleeUnit->getPosition();
    BWAPI::Position enemyPos = enemyUnit->getPosition();
    int dx = myPos.x - enemyPos.x;
    int dy = myPos.y - enemyPos.y;
    double length = std::sqrt(dx * dx + dy * dy);
    const int FLEE_DISTANCE = 2 * 32; // 2 tiles

    int fleeX = myPos.x;
    int fleeY = myPos.y;
    if (length > 0.0) {
        fleeX += static_cast<int>(FLEE_DISTANCE * dx / length);
        fleeY += static_cast<int>(FLEE_DISTANCE * dy / length);
    }
    BWAPI::Position fleeVector(fleeX, fleeY);
    SmartMove(meleeUnit, fleeVector);
}