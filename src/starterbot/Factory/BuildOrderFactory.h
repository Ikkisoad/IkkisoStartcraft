// BuildOrderFactory.h
#pragma once
#include "../../../visualstudio/src/starterbot/BuildOrder.h" // Adjusted the include path to match the relative location
#include "4Pool.h"
// #include "OtherBuildOrder.h" // Add more as needed

enum class BuildOrderType { FourPool /*, OtherBuildOrder */ };

class BuildOrderFactory {
public:
    static BuildOrder* Create(BuildOrderType type) {
        switch (type) {
            case BuildOrderType::FourPool: return &FourPool::Instance();
            // case BuildOrderType::OtherBuildOrder: return &OtherBuildOrder::Instance();
            default: return nullptr;
        }
    }
};