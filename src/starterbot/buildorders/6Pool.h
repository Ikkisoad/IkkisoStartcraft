#pragma once
#include "../../src/starterbot/BuildOrder.h"
#include <BWAPI.h>

class SixPool : public BuildOrder {
public:
    static SixPool& Instance();
    void Execute() override;
    void OnUnitCreate(BWAPI::Unit unit) override;
    void onUnitComplete(BWAPI::Unit unit) override;
    void onStart() override;
    std::string GetName() const override { return "SixPool"; }
private:
    SixPool() = default;
    bool builtSixDrones = false;
};