#pragma once
#include "../../src/starterbot/BuildOrder.h"

class FivePool : public BuildOrder {
public:
    static FivePool& Instance();
    void Execute() override;
    void OnUnitCreate(BWAPI::Unit unit) override;
    void onUnitComplete(BWAPI::Unit unit) override;
    void onStart() override;
    std::string GetName() const override { return "FivePool"; }
    FivePool();
private:
    bool builtExtraDrone = false;
};