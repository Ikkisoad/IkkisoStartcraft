#include "micro.h"
#include "Units.h"
#include "BWAPI.h" // Ensure this header is included for TILE_SIZE definition
#include <random>
#include "../../BasesTools.h"
#include "Tools.h"

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

    // Only check walkability for ground units
    if (!unit->getType().isFlyer()) {
        int tileX = position.x / 32;
        int tileY = position.y / 32;
        if (!BWAPI::Broodwar->isWalkable(tileX * 4, tileY * 4)) return;
    }

    unit->move(position);
    BWAPI::Broodwar->drawCircleMap(BWAPI::Position(position), 5, BWAPI::Colors::Green, true);
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
    const int FLEE_DISTANCE = 32; // 1 tiles

    int fleeX = myPos.x;
    int fleeY = myPos.y;
    if (length > 0.0) {
        fleeX += static_cast<int>(FLEE_DISTANCE * dx / length);
        fleeY += static_cast<int>(FLEE_DISTANCE * dy / length);
    }
    BWAPI::Position fleeVector(fleeX, fleeY);

    // Check if the flee position is walkable
    int tileX = fleeVector.x / 32;
    int tileY = fleeVector.y / 32;
    bool isWalkable = meleeUnit->getType().isFlyer() || BWAPI::Broodwar->isWalkable(tileX * 4, tileY * 4);

    if (!isWalkable) {
        // Try to the right (perpendicular to flee direction)
        int perpDx = -dy;
        int perpDy = dx;
        double perpLength = std::sqrt(perpDx * perpDx + perpDy * perpDy);
        const int SIDE_STEP = 32; // 1 tile to the side

        if (perpLength > 0.0) {
            // Try right
            int rightX = fleeX + static_cast<int>(SIDE_STEP * perpDx / perpLength);
            int rightY = fleeY + static_cast<int>(SIDE_STEP * perpDy / perpLength);
            int rightTileX = rightX / 32;
            int rightTileY = rightY / 32;
            if (BWAPI::Broodwar->isWalkable(rightTileX * 4, rightTileY * 4)) {
                fleeVector = BWAPI::Position(rightX, rightY);
            } else {
                // Try left
                int leftX = fleeX - static_cast<int>(SIDE_STEP * perpDx / perpLength);
                int leftY = fleeY - static_cast<int>(SIDE_STEP * perpDy / perpLength);
                int leftTileX = leftX / 32;
                int leftTileY = leftY / 32;
                if (BWAPI::Broodwar->isWalkable(leftTileX * 4, leftTileY * 4)) {
                    fleeVector = BWAPI::Position(leftX, leftY);
                }
                // If neither is walkable, fallback to original fleeVector (will fail in SmartMove)
            }
        }
    }

    SmartMove(meleeUnit, fleeVector);
}

void Micro::ScoutAndWander(BWAPI::Unit scout)
{
    if (!scout) return;

    // If the unit is not idle, let it finish its current command
    if (!scout->isIdle()) return;

    // 1. Try to find an unexplored start location
    BWAPI::Position unexplored = BasesTools::FindUnexploredStarterPosition();
    if (unexplored != BWAPI::Positions::None && !BWAPI::Broodwar->isExplored(BWAPI::TilePosition(unexplored)))
    {
        SmartMove(scout, unexplored);
        return;
    }

    // 2. If all start locations are explored, wander to a random walkable position
    int mapWidth = BWAPI::Broodwar->mapWidth() * 32;
    int mapHeight = BWAPI::Broodwar->mapHeight() * 32;
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(0, mapWidth - 1);
    std::uniform_int_distribution<int> distY(0, mapHeight - 1);

    for (int tries = 0; tries < 10; ++tries) // Try up to 10 times to find a walkable tile
    {
        int x = distX(rng);
        int y = distY(rng);
        BWAPI::Position pos(x, y);
        int tileX = x / 32;
        int tileY = y / 32;
        if (scout->getType().isFlyer() || BWAPI::Broodwar->isWalkable(tileX * 4, tileY * 4))
        {
            SmartMove(scout, pos);
            return;
        }
    }
}