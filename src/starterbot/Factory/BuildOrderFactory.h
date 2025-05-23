// BuildOrderFactory.h
#pragma once
#include "../../../visualstudio/src/starterbot/BuildOrder.h" // Adjusted the include path to match the relative location
#include "4Pool.h"
#include "5Pool.h" // Add this line

enum class BuildOrderType { FourPool, FivePool /*, OtherBuildOrder */ };

class BuildOrderFactory {
public:
    static BuildOrder* Create(BuildOrderType type) {
        switch (type) {
            case BuildOrderType::FourPool: return &FourPool::Instance();
            case BuildOrderType::FivePool: return &FivePool::Instance(); // Add this line
            // case BuildOrderType::OtherBuildOrder: return &OtherBuildOrder::Instance();
            default: return nullptr;
        }
    }
};