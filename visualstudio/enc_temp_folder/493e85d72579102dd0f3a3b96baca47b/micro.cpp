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
    BWAPI::Broodwar->drawCircleMap(target->getPosition(), 3, BWAPI::Colors::Red, true);
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

//TODO avoid offensive unit and to attack other units if possible
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
    if (!scout->isIdle()) return;

    // First, try to scout the nearest unexplored starting location
    const auto& startLocations = BWAPI::Broodwar->getStartLocations();
    BWAPI::TilePosition nearestUnexplored;
    int minDist = std::numeric_limits<int>::max();
    bool foundUnexplored = false;
    for (const auto& startLoc : startLocations)
    {
        if (!BWAPI::Broodwar->isExplored(startLoc))
        {
            const int dist = scout->getDistance(BWAPI::Position(startLoc));
            if (dist < minDist)
            {
                minDist = dist;
                nearestUnexplored = startLoc;
                foundUnexplored = true;
            }
        }
    }
    if (foundUnexplored)
    {
        SmartMove(scout, BWAPI::Position(nearestUnexplored));
        return;
    }

    // Then, try to scout unexplored bases
    const auto& basePositions = BasesTools::GetBWEMBases();
    for (const auto& pos : basePositions)
    {
        const BWAPI::TilePosition tilePos(pos);
        if (!BWAPI::Broodwar->isExplored(tilePos))
        {
            SmartMove(scout, pos);
            return;
        }
    }

    // If all bases and start locations are explored, wander randomly
    const int mapWidth = BWAPI::Broodwar->mapWidth() * 32;
    const int mapHeight = BWAPI::Broodwar->mapHeight() * 32;
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(0, mapWidth - 1);
    std::uniform_int_distribution<int> distY(0, mapHeight - 1);

    for (int tries = 0; tries < 10; ++tries)
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

void Micro::SmartAvoidLethalAndAttackNonLethal(BWAPI::Unit unit)
{
    if (!unit) return;

    Units unitsInstance;
    auto enemies = unitsInstance.GetNearbyEnemyUnits(unit, 640);

    // First, flee if any lethal enemy is in range
    BWAPI::Unit closestLethal = nullptr;
    int minLethalDist = std::numeric_limits<int>::max();

    for (auto enemy : enemies)
    {
        if (!enemy || !enemy->exists()) continue;

        BWAPI::WeaponType weapon = unit->getType().isFlyer() ? enemy->getType().airWeapon() : enemy->getType().groundWeapon();
        int damage = weapon.damageAmount();
        int range = weapon.maxRange();
        if (range <= 0) range = 32; // fallback for melee

        int unitHP = unit->getHitPoints() + unit->getShields();
        bool isLethal = (damage > 0 && damage >= unitHP);

        int dist = unit->getDistance(unit->getType().isFlyer() ? enemy->getPosition() : enemy->getPosition());
        const int RANGE_BUFFER = 64;
        if (isLethal && dist <= range + RANGE_BUFFER) {
            if (dist < minLethalDist) {
                minLethalDist = dist;
                closestLethal = enemy;
            }
        }
    }

    if (closestLethal) {
        // Flee from the closest lethal enemy in range, but always slightly to the right or left
        BWAPI::Position myPos = unit->getPosition();
        BWAPI::Position lethalPos = closestLethal->getPosition();
        int dx = myPos.x - lethalPos.x;
        int dy = myPos.y - lethalPos.y;
        double length = std::sqrt(dx * dx + dy * dy);

        const int FLEE_DISTANCE = 32;
        const int SIDE_STEP = 16; // Slightly to the side (half a tile)
        int fleeX = myPos.x;
        int fleeY = myPos.y;
        if (length > 0.0) {
            fleeX += static_cast<int>(FLEE_DISTANCE * dx / length);
            fleeY += static_cast<int>(FLEE_DISTANCE * dy / length);

            // Perpendicular vector (right: -dy, dx)
            int perpDx = -dy;
            int perpDy = dx;
            double perpLength = std::sqrt(perpDx * perpDx + perpDy * perpDy);
            if (perpLength > 0.0) {
                // Alternate between right and left based on unit id for determinism
                int side = (unit->getID() % 2 == 0) ? 1 : -1;
                fleeX += static_cast<int>(side * SIDE_STEP * perpDx / perpLength);
                fleeY += static_cast<int>(side * SIDE_STEP * perpDy / perpLength);
            }
        }
        BWAPI::Position fleeVector(fleeX, fleeY);

        // Validate if the tile is walkable, if not flee directly to the right or left
        int tileX = fleeVector.x / 32;
        int tileY = fleeVector.y / 32;
        // Validate if the tile is walkable and not occupied by any unit
        bool isOccupied = false;
        for (const auto& u : BWAPI::Broodwar->getAllUnits()) {
            if (!u || !u->exists()) continue;
            // Use a small radius to check for overlap (8 pixels)
            if (u->getPosition().getDistance(BWAPI::Position(fleeVector)) < 32) {
                isOccupied = true;
                break;
            }
        }
        bool isWalkable = unit->getType().isFlyer() || (BWAPI::Broodwar->isWalkable(tileX * 4, tileY * 4) && !isOccupied);
        if (!isWalkable) {
            if (!isWalkable) {
                // If not walkable, try to flee strictly perpendicular (left/right or up/down)
                // Determine if horizontal or vertical flee is more open
                int absDx = std::abs(dx);
                int absDy = std::abs(dy);
                const int SIDE_ONLY_STEP = 32; // 1 tile

                // Try horizontal (left/right)
                int leftX = myPos.x - SIDE_ONLY_STEP;
                int leftY = myPos.y;
                int rightX = myPos.x + SIDE_ONLY_STEP;
                int rightY = myPos.y;
                bool leftWalkable = BWAPI::Broodwar->isWalkable((leftX / 32) * 4, (leftY / 32) * 4);
                bool rightWalkable = BWAPI::Broodwar->isWalkable((rightX / 32) * 4, (rightY / 32) * 4);

                // Try vertical (up/down)
                int upX = myPos.x;
                int upY = myPos.y - SIDE_ONLY_STEP;
                int downX = myPos.x;
                int downY = myPos.y + SIDE_ONLY_STEP;
                bool upWalkable = BWAPI::Broodwar->isWalkable((upX / 32) * 4, (upY / 32) * 4);
                bool downWalkable = BWAPI::Broodwar->isWalkable((downX / 32) * 4, (downY / 32) * 4);

                // Prefer the direction perpendicular to the main flee vector
                if (absDx > absDy) {
                    // Flee up or down
                    if (upWalkable) {
                        fleeVector = BWAPI::Position(upX, upY);
                    } else if (downWalkable) {
                        fleeVector = BWAPI::Position(downX, downY);
                    } else if (leftWalkable) {
                        fleeVector = BWAPI::Position(leftX, leftY);
                    } else if (rightWalkable) {
                        fleeVector = BWAPI::Position(rightX, rightY);
                    }
                    // else fallback to original fleeVector (will fail in SmartMove)
                } else {
                    // Flee left or right
                    if (leftWalkable) {
                        fleeVector = BWAPI::Position(leftX, leftY);
                    } else if (rightWalkable) {
                        fleeVector = BWAPI::Position(rightX, rightY);
                    } else if (upWalkable) {
                        fleeVector = BWAPI::Position(upX, upY);
                    } else if (downWalkable) {
                        fleeVector = BWAPI::Position(downX, downY);
                    }
                    // else fallback to original fleeVector (will fail in SmartMove)
                }
            }
            // Try to the right (perpendicular to flee direction)
            int perpDx = -dy;
            int perpDy = dx;
            double perpLength = std::sqrt(perpDx * perpDx + perpDy * perpDy);
            const int SIDE_ONLY_STEP = 32; // 1 tile to the side
            if (perpLength > 0.0) {
                // Try right
                int rightX = myPos.x + static_cast<int>(SIDE_ONLY_STEP * perpDx / perpLength);
                int rightY = myPos.y + static_cast<int>(SIDE_ONLY_STEP * perpDy / perpLength);
                int rightTileX = rightX / 32;
                int rightTileY = rightY / 32;
                if (BWAPI::Broodwar->isWalkable(rightTileX * 4, rightTileY * 4)) {
                    fleeVector = BWAPI::Position(rightX, rightY);
                } else {
                    // Try left
                    int leftX = myPos.x - static_cast<int>(SIDE_ONLY_STEP * perpDx / perpLength);
                    int leftY = myPos.y - static_cast<int>(SIDE_ONLY_STEP * perpDy / perpLength);
                    int leftTileX = leftX / 32;
                    int leftTileY = leftY / 32;
                    if (BWAPI::Broodwar->isWalkable(leftTileX * 4, leftTileY * 4)) {
                        fleeVector = BWAPI::Position(leftX, leftY);
                    }
                    // If neither is walkable, fallback to original fleeVector (will fail in SmartMove)
                }
            }
        }

        SmartMove(unit, fleeVector);
        return;
    }

    // --- Target selection with priority system ---
    BWAPI::Unit bestTarget = nullptr;
    int bestPriority = 100;
    int minDist = std::numeric_limits<int>::max();
    bool bestTargetIsLethal = true; // Start with lethal as worst

    for (auto enemy : enemies)
    {
        if (!enemy || !enemy->exists()) continue;

        // Assign priority: 0 = offensive, 1 = worker, 2 = building, 3 = other
        int priority = 3;
        if (enemy->getType().canAttack() && !enemy->getType().isWorker() && !enemy->getType().isBuilding())
            priority = 0;
        else if (enemy->getType().isWorker())
            priority = 1;
        else if (enemy->getType().isBuilding())
            priority = 2;

        BWAPI::WeaponType weapon = enemy->getType().groundWeapon();
        if (unit->getType().isFlyer()) weapon = enemy->getType().airWeapon();
        int damage = weapon.damageAmount();
        int unitHP = unit->getHitPoints() + unit->getShields();
        bool isLethal = (damage > 0 && damage >= unitHP);

        int dist = unit->getDistance(enemy);

        // Prefer non-lethal, then higher priority, then closer
        if ((!isLethal && bestTargetIsLethal) ||
            (isLethal == bestTargetIsLethal && priority < bestPriority) ||
            (isLethal == bestTargetIsLethal && priority == bestPriority && dist < minDist))
        {
            bestTarget = enemy;
            bestPriority = priority;
            minDist = dist;
            bestTargetIsLethal = isLethal;
        }
    }

    if (!bestTarget) return;

    SmartAttackUnit(unit, bestTarget);
}

void Micro::SmartGatherMinerals(BWAPI::Unit drone)
{
    if (!drone || !drone->exists()) return;
    if (!drone->isCompleted() || drone->isConstructing()) return;
    if (!drone->getType().isWorker()) return;

    // If the drone is already performing a move command, let it finish its current command
    if (drone->getOrder() == BWAPI::Orders::Move) return;
    // Prevent issuing gather if already gathering minerals
    if (drone->getOrder() == BWAPI::Orders::MiningMinerals ||
        drone->getOrder() == BWAPI::Orders::Harvest1 ||
        drone->getOrder() == BWAPI::Orders::Harvest2)
    {
        return;
    }

    // If holding minerals, return to the closest hatchery
    if (drone->isCarryingMinerals())
    {
        // Find the closest hatchery
        BWAPI::Unit closestHatchery = nullptr;
        int minDist = std::numeric_limits<int>::max();
        for (auto& unit : BWAPI::Broodwar->self()->getUnits())
        {
            if (!unit->exists()) continue;
            if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery ||
                unit->getType() == BWAPI::UnitTypes::Zerg_Lair ||
                unit->getType() == BWAPI::UnitTypes::Zerg_Hive)
            {
                int dist = drone->getDistance(unit);
                if (dist < minDist)
                {
                    minDist = dist;
                    closestHatchery = unit;
                }
            }
        }

        if (closestHatchery)
        {
            const int RETURN_RADIUS = 32; // 1 tile
            const int distToHatchery = drone->getDistance(closestHatchery);

            if (distToHatchery < RETURN_RADIUS)
            {
                // If close enough, return cargo
                if (drone->getOrder() != BWAPI::Orders::ReturnMinerals)
                {
                    drone->returnCargo();
                }
            }
            else
            {
                // If not close enough, move to hatchery
                if (drone->getOrderTarget() != closestHatchery || drone->getOrder() != BWAPI::Orders::Move)
                {
                    drone->move(closestHatchery->getPosition());
                }
            }
        }
        return;
    }

    // If not holding minerals, find the closest mineral patch
    BWAPI::Unit closestMineral = nullptr;
    int minDist = std::numeric_limits<int>::max();
    for (auto& mineral : BWAPI::Broodwar->getMinerals())
    {
        if (!mineral->exists() || mineral->getResources() <= 0) continue;
        int dist = drone->getDistance(mineral);
        if (dist < minDist)
        {
            minDist = dist;
            closestMineral = mineral;
        }
    }

    if (closestMineral)
    {
        // If close enough, gather
        if (drone->getDistance(closestMineral) < 64) // 2 tiles
        {
            // Only issue gather if the last command was not already a gather command
            if (drone->getLastCommand().getType() != BWAPI::UnitCommandTypes::Gather) {
                if (drone->getOrder() != BWAPI::Orders::MiningMinerals &&
                    drone->getOrder() != BWAPI::Orders::Harvest1 &&
                    drone->getOrder() != BWAPI::Orders::Harvest2)
                {
                    drone->gather(closestMineral);
                }
            }
        }
        else
        {
            // Move to mineral patch if not already moving there
            if (drone->getOrderTarget() != closestMineral || drone->getOrder() != BWAPI::Orders::Move)
            {
                drone->move(closestMineral->getPosition());
            }
        }
    }
}