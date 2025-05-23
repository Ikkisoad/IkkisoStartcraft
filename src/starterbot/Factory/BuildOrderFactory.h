// BuildOrderFactory.h
#pragma once
#include "../../../visualstudio/src/starterbot/BuildOrder.h" // Adjusted the include path to match the relative location
#include "4Pool.h"
#include "5Pool.h"
#include "6Pool.h"
#include "7Pool.h"
#include "8Pool.h"

enum class BuildOrderType { FourPool, FivePool, SixPool, SevenPool, EightPool };

class BuildOrderFactory {
public:
    static BuildOrder* Create(BuildOrderType type) {
        switch (type) {
            case BuildOrderType::FourPool: return &FourPool::Instance();
            case BuildOrderType::FivePool: return &FivePool::Instance();
            case BuildOrderType::SixPool: return &SixPool::Instance();
            case BuildOrderType::SevenPool: return &SevenPool::Instance();
            case BuildOrderType::EightPool: return &EightPool::Instance();
            default: return nullptr;
        }
    }
};