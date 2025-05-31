#include "Genetic.h"
#include <BWAPI.h>
#include "../Tools.h"
#include "../micro.h"

Genetic& Genetic::Instance() {
    static Genetic instance;
    return instance;
}

Genetic::Genetic() {
    // Example: Fill with a simple build order (replace with genetic population later)
    buildOrder = {
        { 4, -1, BWAPI::UnitTypes::Zerg_Drone },
        { 5, -1, BWAPI::UnitTypes::Zerg_Drone },
        { 6, -1, BWAPI::UnitTypes::Zerg_Drone },
        { 7, -1, BWAPI::UnitTypes::Zerg_Drone },
        { 8, -1, BWAPI::UnitTypes::Zerg_Drone },
        { 9, -1, BWAPI::UnitTypes::Zerg_Spawning_Pool },
        { 10, -1, BWAPI::UnitTypes::Zerg_Overlord },
        { 12, -1, BWAPI::UnitTypes::Zerg_Hatchery },
        // Add more actions as needed
    };
}

void Genetic::onStart() {
    currentAction = 0;
    logFile.open("genetic_build_log.txt", std::ios::app);
    LogEvent("GameStart");
}

void Genetic::Execute() {
    int currentSupply = BWAPI::Broodwar->self()->supplyUsed() / 2; // supplyUsed is double in BWAPI
    int currentFrame = BWAPI::Broodwar->getFrameCount();

    // Try to execute the next action if its supply/frame condition is met
    while (currentAction < buildOrder.size()) {
        const BuildAction& action = buildOrder[currentAction];
        if ((action.supply <= currentSupply) && (action.frame == -1 || action.frame <= currentFrame)) {
            // Try to build/train/morph
            bool issued = false;
            if (action.type.isBuilding()) {
				issued = Tools::TryBuildBuilding(action.type, 1, BWAPI::Broodwar->self()->getStartLocation());
            } else if (action.type.getRace() == BWAPI::Races::Zerg && action.type.isTwoUnitsInOneEgg()) {
                issued = Tools::MorphLarva(action.type);
            } else {
                issued = Tools::TrainUnit(action.type);
            }
            if (issued) {
                LogEvent("Issued: " + action.type.getName() + " at supply " + std::to_string(currentSupply) + " frame " + std::to_string(currentFrame));
                ++currentAction;
            } else {
                // Could not issue, try again next frame
                break;
            }
        } else {
            break;
        }
    }

    Micro::BasicAttackAndScoutLoop(BWAPI::Broodwar->self()->getUnits());
}

void Genetic::LogEvent(const std::string& event) {
    if (logFile.is_open()) {
        logFile << BWAPI::Broodwar->getFrameCount() << "," << BWAPI::Broodwar->self()->supplyUsed() / 2 << "," << event << std::endl;
    }
}

void Genetic::onEnd(bool isWinner) {

}

void Genetic::OnUnitCreate(BWAPI::Unit unit) {

}
void Genetic::onUnitComplete(BWAPI::Unit unit) {

}