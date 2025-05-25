#pragma once
#include "../../src/starterbot/BuildOrder.h"
#include <BWAPI.h>

class SevenPool : public BuildOrder {
public:
    static SevenPool& Instance();
    void Execute() override;
    void OnUnitCreate(BWAPI::Unit unit) override;
    void onUnitComplete(BWAPI::Unit unit) override;
    void onStart() override;
    std::string GetName() const override { return "SevenPool"; }
private:
    SevenPool() = default;
    bool builtSevenDrones = false;
};