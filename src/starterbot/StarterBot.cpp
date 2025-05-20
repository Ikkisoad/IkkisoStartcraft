#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"
#include "../../visualstudio/BasesTools.h"

StarterBot::StarterBot()
{
    
}

int workersWanted = 9;
int lingsWanted = 0;
int unusedSupplyAccepted = 1;
BWAPI::Unit scout;

// Called when the bot starts!
void StarterBot::onStart()
{
    BasesTools::Initialize();
    scout = getAvailableUnit(BWAPI::UnitTypes::Zerg_Overlord);
    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(32);
    BWAPI::Broodwar->setFrameSkip(0);
    
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();

    //Send first scout
    Tools::Scout(scout);
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    BasesTools::FindExpansions();
    // Update our MapTools information
    m_mapTools.onFrame();

    // Send our idle workers to mine minerals so they don't just stand there
    sendIdleWorkersToMinerals();

    // Train more workers so we can gather more income
    trainAdditionalWorkers();

    // Build more supply if we are going to run out soon
    buildAdditionalSupply();

    buildOrder();
    Tools::Scout(scout);

    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

void StarterBot::buildOrder() {
    if (BWAPI::Broodwar->self()->supplyUsed() >= 9 * 2) {
        if (buildBuilding(BWAPI::UnitTypes::Zerg_Spawning_Pool, 1)) {
            workersWanted = 11;
        }
    }

    if (Tools::CountUnitsOfType(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->self()->getUnits()) < lingsWanted) {
        trainUnit(BWAPI::UnitTypes::Zerg_Zergling);
    }

    if (BWAPI::Broodwar->self()->supplyUsed() >= 15 * 2) {
        buildBuilding(BWAPI::UnitTypes::Zerg_Extractor, 0);
    }

    if (BWAPI::Broodwar->self()->minerals() > 350) {
        buildBuilding(BWAPI::UnitTypes::Zerg_Hatchery, 0);
    }
}

bool StarterBot::buildBuilding(BWAPI::UnitType building, int limitAmount = 0) {
    if (Tools::IsQueued(building) || Tools::IsReady(building) && limitAmount != 0) {
        return true;
    }

    if (BWAPI::Broodwar->self()->minerals() <= building.mineralPrice()) {
        return false;
    }

    if (limitAmount != 0 && Tools::CountUnitsOfType(building, BWAPI::Broodwar->self()->getUnits()) >= limitAmount) {
        return false;
    }

    return Tools::BuildBuilding(building);
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

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}
// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted)
    {
        // get the unit pointer to my depot
        const BWAPI::Unit myDepot = Tools::GetDepot();

        // if we have a valid depot unit and it's currently not training something, train a worker
        // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
        if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
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
    if (unusedSupply >= unusedSupplyAccepted) { return; }

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
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
    if (supplyProviderType == unit->getType()) {
        unusedSupplyAccepted--;
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
}

void StarterBot::attack() {
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits) {
        if (!unit->getType().isWorker() && unit->getType() != BWAPI::UnitTypes::Zerg_Overlord) {
            for (auto tile : BWAPI::Broodwar->getStartLocations()) {
                for (auto unitsOnTile : BWAPI::Broodwar->getUnitsOnTile(tile)) {
                    if (unitsOnTile->getPlayer() == BWAPI::Broodwar->self()) continue;
                }
                if (BWAPI::Broodwar->isExplored(tile) && !BWAPI::Broodwar->isBuildable(tile, true) || !BWAPI::Broodwar->isExplored(tile)) {
                    BWAPI::Position pos(tile);
                    auto command = unit->getLastCommand();
                    if (command.getTargetPosition() == pos) return;
                    unit->attack(pos);
                }
            }
        }
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
    if (supplyProviderType == unit->getType()) {
        if (BWAPI::Broodwar->self()->supplyUsed() == 9 && Tools::IsQueued(BWAPI::UnitTypes::Zerg_Spawning_Pool)) {
            workersWanted = 11;
        }
        unusedSupplyAccepted++;
    }

    if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
        workersWanted = 11;
        lingsWanted = 30;
    }

    if (unit->getType() == BWAPI::UnitTypes::Zerg_Extractor) {
        Tools::GatherGas(unit);
    }

    if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling) {
        attack();
    }

    if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery) {
        lingsWanted *= 2;
        buildBuilding(BWAPI::UnitTypes::Zerg_Evolution_Chamber);
    }
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 
	
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