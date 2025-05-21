#pragma once
#include "../../src/starterbot/BuildOrder.h"
#include <BWAPI.h>

class FourPool : public BuildOrder {
public:
    static FourPool& Instance();
    void Execute() override;
    void OnUnitCreate(BWAPI::Unit unit) override;
    void onUnitComplete(BWAPI::Unit unit) override;
    void attack();
private:
    FourPool() = default;
};