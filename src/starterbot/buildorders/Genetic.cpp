#include "Genetic.h"
#include <BWAPI.h>
#include <random>
#include <fstream>
#include <sstream>
#include <map>
#include "../Tools.h"
#include "../micro.h"

struct BuildOrderResult {
    int games = 0;
    int wins = 0;
    int totalScore = 0;
};

Genetic& Genetic::Instance() {
    static Genetic instance;
    return instance;
}

Genetic::Genetic() {
    // Add all Zerg buildings
    for (auto& type : BWAPI::UnitTypes::allUnitTypes()) {
        if (type.getRace() == BWAPI::Races::Zerg && type.isBuilding() && type != BWAPI::UnitTypes::Zerg_Larva && type != BWAPI::UnitTypes::Zerg_Egg) {
            buildOrder.push_back({ 4, -1, ActionType::Building, type, BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None });
        }
        if (type.getRace() == BWAPI::Races::Zerg && !type.isBuilding()) {
            buildOrder.push_back({ 4, -1, ActionType::Unit, type, BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None });
        }
    }
    // Add all Zerg upgrades
    for (auto& upgrade : BWAPI::UpgradeTypes::allUpgradeTypes()) {
        if (upgrade.getRace() == BWAPI::Races::Zerg) {
            buildOrder.push_back({ 4, -1, ActionType::Upgrade, BWAPI::UnitTypes::None, upgrade, BWAPI::TechTypes::None });
        }
    }
    // Add all Zerg techs
    for (auto& tech : BWAPI::TechTypes::allTechTypes()) {
        if (tech.getRace() == BWAPI::Races::Zerg) {
            buildOrder.push_back({ 4, -1, ActionType::Tech, BWAPI::UnitTypes::None, BWAPI::UpgradeTypes::None, tech });
        }
    }
}

void Genetic::onStart() {
    AnalyzeBuildOrderResults();
    currentAction = 0;
    logFile.open("genetic_build_log.txt", std::ios::app);
    LogEvent("GameStart");
}

void Genetic::Execute() {
    int currentSupply = BWAPI::Broodwar->self()->supplyUsed() / 2;
    int currentFrame = BWAPI::Broodwar->getFrameCount();

    if (buildOrder.empty()) return;

    // Randomly select an action to try
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<size_t> dist(0, buildOrder.size() - 1);
    size_t actionIndex = dist(rng);
    const BuildAction& action = buildOrder[actionIndex];

    bool issued = false;
    switch (action.actionType) {
        case ActionType::Building:
            issued = Tools::TryBuildBuilding(action.unitType, 1, BWAPI::Broodwar->self()->getStartLocation());
            break;
        case ActionType::Unit:
            issued = Tools::MorphLarva(action.unitType);
            break;
        case ActionType::Upgrade:
            issued = Tools::ResearchUpgrade(action.upgradeType);
            break;
        case ActionType::Tech:
            issued = Tools::ResearchTech(action.techType);
            break;
    }

    if (issued) {
        LogEvent("Issued: " + action.unitType.getName() + " (or upgrade/tech) at supply " + std::to_string(currentSupply) + " frame " + std::to_string(currentFrame));
    }

    Micro::BasicAttackAndScoutLoop(BWAPI::Broodwar->self()->getUnits());
}

void Genetic::LogEvent(const std::string& event) {
    if (logFile.is_open()) {
        logFile << BWAPI::Broodwar->getFrameCount() << "," << BWAPI::Broodwar->self()->supplyUsed() / 2 << "," << event << std::endl;
    }
}

void Genetic::onEnd(bool isWinner) {
    if (logFile.is_open()) {
        int unitScore = BWAPI::Broodwar->self()->getUnitScore();
        int killScore = BWAPI::Broodwar->self()->getKillScore();
        int buildingScore = BWAPI::Broodwar->self()->getBuildingScore();
        int razingScore = BWAPI::Broodwar->self()->getRazingScore();
        int customScore = BWAPI::Broodwar->self()->getCustomScore();

        logFile << "GameEnd,"
                << (isWinner ? "Win" : "Loss") << ","
                << "UnitScore:" << unitScore << ","
                << "KillScore:" << killScore << ","
                << "BuildingScore:" << buildingScore << ","
                << "RazingScore:" << razingScore << ","
                << "CustomScore:" << customScore
                << std::endl;
    }
}

void Genetic::AnalyzeBuildOrderResults() {
    std::ifstream infile("genetic_build_log.txt");
    std::string line;
    std::map<std::string, BuildOrderResult> buildOrderStats;
    std::string currentBuildOrder;

    while (std::getline(infile, line)) {
        if (line.find("GameStart") != std::string::npos) {
            currentBuildOrder.clear();
        } else if (line.find("Issued:") != std::string::npos) {
            currentBuildOrder += line + ";";
        } else if (line.find("GameEnd") != std::string::npos) {
            // Parse result
            bool win = line.find("Win") != std::string::npos;
            int score = 0;
            size_t pos = line.find("CustomScore:");
            if (pos != std::string::npos) {
                std::istringstream iss(line.substr(pos + 12));
                iss >> score;
            }
            auto& result = buildOrderStats[currentBuildOrder];
            result.games++;
            if (win) result.wins++;
            result.totalScore += score;
        }
    }

    // Output summary (could be to a file or console)
    for (const auto& [order, result] : buildOrderStats) {
        double avgScore = result.games ? (double)result.totalScore / result.games : 0.0;
        std::cout << "BuildOrder: " << order << "\n"
                  << "  Games: " << result.games
                  << "  Wins: " << result.wins
                  << "  AvgScore: " << avgScore << "\n";
    }
}

void Genetic::OnUnitCreate(BWAPI::Unit unit) {

}
void Genetic::onUnitComplete(BWAPI::Unit unit) {

}