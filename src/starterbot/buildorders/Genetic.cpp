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

    // Add MicroMode action
    buildOrder.push_back({ 10, -1, ActionType::MicroMode, BWAPI::UnitTypes::None, BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None, Micro::MicroMode::Aggressive });
    buildOrder.push_back({ 10, -1, ActionType::MicroMode, BWAPI::UnitTypes::None, BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None, Micro::MicroMode::Defensive });
    buildOrder.push_back({ 10, -1, ActionType::MicroMode, BWAPI::UnitTypes::None, BWAPI::UpgradeTypes::None, BWAPI::TechTypes::None, Micro::MicroMode::Neutral });
}

void Genetic::onStart() {
    Micro::SetMode(Micro::MicroMode::Aggressive);
    AnalyzeBuildOrderResults();
    currentAction = 0;
    logFile.open("genetic_build_log.txt", std::ios::app);
    LogEvent("GameStart");
    MutateBuildOrder(buildOrder);
}

void Genetic::Execute() {
    int currentSupply = BWAPI::Broodwar->self()->supplyUsed() / 2;
    int currentFrame = BWAPI::Broodwar->getFrameCount();

    if (buildOrder.empty()) return;

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
        case ActionType::MicroMode:
            Micro::SetMode(action.microMode);
            issued = true;
            break;
    }

    Micro::BasicAttackAndScoutLoop(BWAPI::Broodwar->self()->getUnits());
}

void Genetic::LogEvent(const std::string& event) {
    if (logFile.is_open()) {
        logFile << BWAPI::Broodwar->getFrameCount() << "," << BWAPI::Broodwar->self()->supplyUsed() / 2 << "," << event << std::endl;
    }
}

void Genetic::onEnd(bool isWinner) {
    std::ofstream logFile("genetic_build_log.csv", std::ios::app);
    if (!logFile.is_open()) return;

    // Serialize build order
    std::ostringstream buildOrderStream;
    for (const auto& action : buildOrder) {
        switch (action.actionType) {
            case ActionType::Unit:      buildOrderStream << "U" << action.unitType.getID(); break;
            case ActionType::Building:  buildOrderStream << "B" << action.unitType.getID(); break;
            case ActionType::Upgrade:   buildOrderStream << "Up" << action.upgradeType.getID(); break;
            case ActionType::Tech:      buildOrderStream << "T" << action.techType.getID(); break;
            case ActionType::MicroMode: buildOrderStream << "M" << static_cast<int>(action.microMode); break;
        }
        buildOrderStream << ";";
    }

    int unitScore = BWAPI::Broodwar->self()->getUnitScore();
    int killScore = BWAPI::Broodwar->self()->getKillScore();
    int buildingScore = BWAPI::Broodwar->self()->getBuildingScore();
    int razingScore = BWAPI::Broodwar->self()->getRazingScore();
    int customScore = BWAPI::Broodwar->self()->getCustomScore();

    logFile << buildOrderStream.str() << ","
            << (isWinner ? "Win" : "Loss") << ","
            << unitScore << ","
            << killScore << ","
            << buildingScore << ","
            << razingScore << ","
            << customScore << std::endl;
    logFile.close();
}

// Helper: Parse a build order string into BuildAction vector (very basic, adjust as needed)
std::vector<BuildAction> ParseBuildOrder(const std::string& orderStr) {
    std::vector<BuildAction> actions;
    std::istringstream iss(orderStr);
    std::string token;
    while (std::getline(iss, token, ';')) {
        // You need to implement a parser that reconstructs BuildAction from the log string
        // For now, this is a placeholder
        // actions.push_back(...);
    }
    return actions;
}

// Helper: Mutate a build order (swap two actions)
void Genetic::MutateBuildOrder(std::vector<BuildAction>& actions) {
    if (actions.size() < 2) return;
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> dist(0, actions.size() - 1);

    // Swap two actions
    size_t i = dist(rng);
    size_t j = dist(rng);
    if (i != j) std::swap(actions[i], actions[j]);

    std::uniform_real_distribution<double> prob(0.0, 1.0);

    // 50% chance to mutate micro mode
    if (prob(rng) < 0.5) {
        size_t pos = dist(rng);
        std::uniform_int_distribution<int> modeDist(0, 2);
        Micro::MicroMode newMode = static_cast<Micro::MicroMode>(modeDist(rng));
        if (prob(rng) < 0.5) {
            BuildAction microAction;
            microAction.supply = -1;
            microAction.frame = -1;
            microAction.actionType = ActionType::MicroMode;
            microAction.microMode = newMode;
            actions.insert(actions.begin() + pos, microAction);
        }
        else {
            bool replaced = false;
            for (auto& act : actions) {
                if (act.actionType == ActionType::MicroMode) {
                    act.microMode = newMode;
                    replaced = true;
                    break;
                }
            }
            if (!replaced) {
                BuildAction microAction;
                microAction.supply = -1;
                microAction.frame = -1;
                microAction.actionType = ActionType::MicroMode;
                microAction.microMode = newMode;
                actions.insert(actions.begin() + pos, microAction);
            }
        }
    }

    // 30% chance to add a new random build/unit/upgrade/tech action
    if (prob(rng) < 0.3) {
        // Choose a random action type (excluding MicroMode)
        std::uniform_int_distribution<int> actionTypeDist(0, 3); // 0:Unit, 1:Building, 2:Upgrade, 3:Tech
        ActionType newType = static_cast<ActionType>(actionTypeDist(rng));
        BuildAction newAction;
        newAction.supply = -1;
        newAction.frame = -1;
        newAction.actionType = newType;

        switch (newType) {
        case ActionType::Unit: {
            // Pick a random Zerg unit
            auto& all = BWAPI::UnitTypes::allUnitTypes();
            std::vector<BWAPI::UnitType> zergUnits;
            for (auto& t : all) if (t.getRace() == BWAPI::Races::Zerg && !t.isBuilding()) zergUnits.push_back(t);
            if (!zergUnits.empty()) {
                newAction.unitType = zergUnits[dist(rng) % zergUnits.size()];
            }
            break;
        }
        case ActionType::Building: {
            auto& all = BWAPI::UnitTypes::allUnitTypes();
            std::vector<BWAPI::UnitType> zergBuildings;
            for (auto& t : all) if (t.getRace() == BWAPI::Races::Zerg && t.isBuilding() && t != BWAPI::UnitTypes::Zerg_Larva && t != BWAPI::UnitTypes::Zerg_Egg) zergBuildings.push_back(t);
            if (!zergBuildings.empty()) {
                newAction.unitType = zergBuildings[dist(rng) % zergBuildings.size()];
            }
            break;
        }
        case ActionType::Upgrade: {
            auto& all = BWAPI::UpgradeTypes::allUpgradeTypes();
            std::vector<BWAPI::UpgradeType> zergUpgrades;
            for (auto& u : all) if (u.getRace() == BWAPI::Races::Zerg) zergUpgrades.push_back(u);
            if (!zergUpgrades.empty()) {
                newAction.upgradeType = zergUpgrades[dist(rng) % zergUpgrades.size()];
            }
            break;
        }
        case ActionType::Tech: {
            auto& all = BWAPI::TechTypes::allTechTypes();
            std::vector<BWAPI::TechType> zergTechs;
            for (auto& t : all) if (t.getRace() == BWAPI::Races::Zerg) zergTechs.push_back(t);
            if (!zergTechs.empty()) {
                newAction.techType = zergTechs[dist(rng) % zergTechs.size()];
            }
            break;
        }
        default: break;
        }
        // Insert at a random position
        size_t pos = dist(rng);
        actions.insert(actions.begin() + pos, newAction);
    }
}

void Genetic::AnalyzeBuildOrderResults() {
    std::ifstream infile("genetic_build_log.csv");
    std::string line;
    std::map<std::string, BuildOrderResult> buildOrderStats;

    while (std::getline(infile, line)) {
        // Format: <build_order_serialized>,<Win|Loss>,<unit_score>,<kill_score>,<building_score>,<razing_score>,<custom_score>
        std::istringstream iss(line);
        std::string buildOrderStr, resultStr, unitScoreStr, killScoreStr, buildingScoreStr, razingScoreStr, customScoreStr;

        if (!std::getline(iss, buildOrderStr, ',')) continue;
        if (!std::getline(iss, resultStr, ',')) continue;
        if (!std::getline(iss, unitScoreStr, ',')) continue;
        if (!std::getline(iss, killScoreStr, ',')) continue;
        if (!std::getline(iss, buildingScoreStr, ',')) continue;
        if (!std::getline(iss, razingScoreStr, ',')) continue;
        if (!std::getline(iss, customScoreStr, ',')) continue;

        int unitScore = std::stoi(unitScoreStr);
        int killScore = std::stoi(killScoreStr);
        int buildingScore = std::stoi(buildingScoreStr);
        int razingScore = std::stoi(razingScoreStr);
        int customScore = std::stoi(customScoreStr);

        int total = unitScore + killScore + buildingScore + razingScore + customScore;

        auto& result = buildOrderStats[buildOrderStr];
        result.games++;
        if (resultStr == "Win") result.wins++;
        result.totalScore += total;
    }

    // Output summary (could be to a file or console)
    for (const auto& [order, result] : buildOrderStats) {
        double avgScore = result.games ? (double)result.totalScore / result.games : 0.0;
        std::cout << "BuildOrder: " << order << "\n"
                  << "  Games: " << result.games
                  << "  Wins: " << result.wins
                  << "  AvgScore: " << avgScore << "\n";
    }

    std::ofstream bestFile("best_build_orders.txt");
    for (const auto& [order, result] : buildOrderStats) {
        if (result.games >= 3 && result.wins > 0) { // Example filter
            bestFile << order << "\n";
        }
    }

    // Load best build orders for future use
    std::vector<std::string> bestBuildOrders;
    std::ifstream bestFileInput("best_build_orders.txt");
    while (std::getline(bestFileInput, line)) {
        bestBuildOrders.push_back(line);
    }

    if (!bestBuildOrders.empty()) {
        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<size_t> dist(0, bestBuildOrders.size() - 1);
        std::string selectedOrder = bestBuildOrders[dist(rng)];
        // Parse and mutate
        std::vector<BuildAction> actions = ParseBuildOrder(selectedOrder);
        MutateBuildOrder(actions);
        if (!actions.empty()) {
            buildOrder = actions;
        }
    }

    // After filling buildOrderStats and bestBuildOrders...
    std::string bestOrderStr;
    double bestScore = -1.0;
    for (const auto& [order, result] : buildOrderStats) {
        double avgScore = result.games ? (double)result.totalScore / result.games : 0.0;
        if (avgScore > bestScore) {
            bestScore = avgScore;
            bestOrderStr = order;
        }
    }

    if (!bestOrderStr.empty()) {
        std::vector<BuildAction> actions = ParseBuildOrder(bestOrderStr);
        MutateBuildOrder(actions);
        if (!actions.empty()) {
            buildOrder = actions;
        }
    }
}

void Genetic::OnUnitCreate(BWAPI::Unit unit) {

}
void Genetic::onUnitComplete(BWAPI::Unit unit) {

}