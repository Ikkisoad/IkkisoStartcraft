#pragma once
#include "../../src/starterbot/BuildOrder.h"
#include <BWAPI.h>
#include <vector>
#include <string>
#include <fstream>

enum class ActionType { Unit, Building, Upgrade, Tech };

struct BuildAction {
    int supply;         // Supply at which to trigger
    int frame;          // Frame at which to trigger (optional, can be -1 if unused)
    ActionType actionType;
    BWAPI::UnitType unitType;      // For units/buildings
    BWAPI::UpgradeType upgradeType; // For upgrades
    BWAPI::TechType techType;      // For tech research
};

class Genetic : public BuildOrder {
public:
    static Genetic& Instance();
    void Execute() override;
    void OnUnitCreate(BWAPI::Unit unit) override;
    void onUnitComplete(BWAPI::Unit unit) override;
    void onStart() override;
    void onEnd(bool isWinner) override;
    void AnalyzeBuildOrderResults();
    std::string GetName() const override { return "Genetic"; }
    // Logging
    void LogEvent(const std::string& event);
private:
    Genetic();
    std::vector<BuildAction> buildOrder;
    size_t currentAction = 0;
    std::ofstream logFile;
};