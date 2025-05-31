#pragma once
#include <BWAPI.h>
class BuildOrder
{
public:
    virtual ~BuildOrder() = default;
    virtual void Execute() = 0;
    virtual void OnUnitCreate(BWAPI::Unit unit) {};
    virtual void onUnitComplete(BWAPI::Unit unit) {};
    virtual void onStart() {};
    virtual void onEnd(bool isWinner) {};
    virtual std::string GetName() const = 0;
};