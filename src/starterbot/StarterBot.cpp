#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"
#include "buildorders/4Pool.h"
#include "../../visualstudio/BasesTools.h"
#include "../../visualstudio/src/starterbot/BuildOrder.h"
#include "Factory/BuildOrderFactory.h"
#include "micro.h"
#include "stats/stats.h"
#include <random>

StarterBot::StarterBot()
{
    
}

BuildOrder* currentBuildOrder;

int unusedSupplyAccepted = 1;
int mineralsFrame = 0;
BWAPI::Unit scout;

// Called when the bot starts!
void StarterBot::onStart()
{
    std::string oponentName = "";
    for (auto player : BWAPI::Broodwar->getPlayers()) {
        if (player != BWAPI::Broodwar->self() && player != BWAPI::Broodwar->neutral()) {
            oponentName = player->getName();
        }
    }

    auto strategy = Stats::readStrategy("strats-v0", oponentName, BWAPI::Broodwar->mapHash());


    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(0, sizeof(BuildOrderType));
    int x = distX(rng);

    BWAPI::Broodwar->printf("Random number: %i and size of enum %i", x, sizeof(BuildOrderType));

    BasesTools::Initialize();
    mineralsFrame = 0;
    // Select the build order type you want to use
    BuildOrderType selectedBuildOrder = static_cast<BuildOrderType>(x);
    currentBuildOrder = BuildOrderFactory::Create(selectedBuildOrder);
    BWAPI::Broodwar->printf("Strategy selected: %s", currentBuildOrder->GetName());

    scout = getAvailableUnit(BWAPI::UnitTypes::Zerg_Overlord);

    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(0); //32
    BWAPI::Broodwar->setFrameSkip(0); //0
    
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();

    //Send first scout
    Tools::Scout(scout);
    BasesTools::FindExpansions();

    currentBuildOrder->onStart();
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    if (BWAPI::BWAPI_isDebug) {
        BasesTools::DrawEnemyBases(BWAPI::Colors::Orange);
        BasesTools::DrawAllBases(BWAPI::Colors::Yellow);
        Tools::DrawUnitHealthBars();
        drawDebugInformation();
    }
    BasesTools::VerifyEnemyBases();
    m_mapTools.onFrame();
    buildAdditionalSupply();

    currentBuildOrder->Execute();
}

bool StarterBot::buildBuilding(BWAPI::UnitType building, int limitAmount = 0, BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation()) {
    if (Tools::IsQueued(building) || Tools::IsReady(building) && limitAmount != 0) {
        return true;
    }

    if (BWAPI::Broodwar->self()->minerals() <= building.mineralPrice()) {
        return false;
    }

    if (limitAmount != 0 && Tools::CountUnitsOfType(building, BWAPI::Broodwar->self()->getUnits(), false) >= limitAmount) {
        return false;
    }

    return Tools::BuildBuilding(building, desiredPos);
}

BWAPI::Unit StarterBot::getAvailableUnit(BWAPI::UnitType unitType) {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits) {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType() == unitType && unit->isIdle()) {
            // Get the closest mineral to this worker unit
            return unit;
        }
    }
}

// Train more workers so we can gather more income
bool StarterBot::trainUnit(BWAPI::UnitType unit) {
    const BWAPI::Unit myDepot = Tools::GetDepot();

    // if we have a valid depot unit and it's currently not training something, train a worker
    // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
    if (myDepot) {
        return myDepot->train(unit);
    }
    return false;
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything
    if (unusedSupply >= unusedSupplyAccepted && unusedSupply != 1) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = trainUnit(supplyProviderType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
    }
}

// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Ikkrius\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner)
{
    std::string oponentName = "";
    for (auto player : BWAPI::Broodwar->getPlayers()) {
        if (player != BWAPI::Broodwar->self() && player != BWAPI::Broodwar->neutral()) {
            oponentName = player->getName();
        }
    }
    Stats::LogGameStats(oponentName, BWAPI::Broodwar->mapHash(), currentBuildOrder->GetName(), isWinner);
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";

    // At end of match
    Stats::updateWinRateFile("strats-v0", oponentName, BWAPI::Broodwar->mapHash(), currentBuildOrder->GetName(), isWinner);
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
    if (supplyProviderType == unit->getType()) {
        unusedSupplyAccepted--;
    }
    BasesTools::OnUnitDestroyed(unit);
    // Remove enemy base if the destroyed unit was an enemy building at a known base position
    if (unit->getPlayer() != BWAPI::Broodwar->self() &&
        unit->getPlayer() != BWAPI::Broodwar->neutral() &&
        unit->getType().isBuilding())
    {
        // Check if this building was at a known enemy base position (within a reasonable distance)
        BasesTools::RemoveEnemyBasePosition(unit->getPosition());
    }
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
	
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text) 
{ 
    if (text == "m")
    {
        m_mapTools.toggleDraw();
    }
    if (text == "a") { //Attack
        attack();
    }
    if (text == "d") { //Defend
    }
    if (text == "e") { //Expand
    }
    if (text == "1") { //
        BWAPI::Broodwar->setLocalSpeed(0); //32
    }
    if (text == "2") { //
        BWAPI::Broodwar->setLocalSpeed(8); //32
    }
    if (text == "3") { //
        BWAPI::Broodwar->setLocalSpeed(32); //32
    }
    if (text == "4") { //
        BWAPI::Broodwar->setLocalSpeed(128); //32
    }
}

void StarterBot::attack() {
    BWAPI::Position enemyBase = BasesTools::GetEnemyBasePosition();
    if (enemyBase == BWAPI::Positions::None) {
        // fallback: attack nearest enemy unit as before
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
                BWAPI::Unit nearestEnemy = nullptr;
                int minDistance = std::numeric_limits<int>::max();
                for (auto enemyUnit : BWAPI::Broodwar->getAllUnits()) {
                    if (enemyUnit->getPlayer() != BWAPI::Broodwar->self() && enemyUnit->getPlayer() != BWAPI::Broodwar->neutral()) {
                        int distance = unit->getDistance(enemyUnit);
                        if (distance < minDistance) {
                            minDistance = distance;
                            nearestEnemy = enemyUnit;
                        }
                    }
                }
                if (nearestEnemy) {
                    unit->attack(nearestEnemy->getPosition());
                }
            }
        }
    } else {
        // attack the known enemy base position
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
                unit->attack(enemyBase);
            }
        }
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
    currentBuildOrder->OnUnitCreate(unit);
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
	currentBuildOrder->onUnitComplete(unit);
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 
    // If the unit belongs to the enemy and is a building
    if (unit->getPlayer() != BWAPI::Broodwar->self() &&
        unit->getPlayer() != BWAPI::Broodwar->neutral() &&
        unit->getType().isBuilding())
    {
        // Check if the building is close to any known base position
        const auto& basePositions = BasesTools::GetBWEMBases();
        bool foundBase = false;
        for (const auto& basePos : basePositions) {
            if (unit->getPosition().getApproxDistance(basePos) < 1280) {
                BasesTools::SetEnemyBasePosition(basePos);
                foundBase = true;
                break;
            }
        }
        // If not close to any known base, use the building's position
        if (!foundBase) {
            BasesTools::SetEnemyBasePosition(unit->getPosition());
        }
    }
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{ 
	
}