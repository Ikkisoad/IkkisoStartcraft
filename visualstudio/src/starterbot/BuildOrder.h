#pragma once
#include <BWAPI.h>
class BuildOrder
{
public:
    virtual ~BuildOrder() = default;
    virtual void Execute() = 0;
    virtual void OnUnitCreate(BWAPI::Unit unit) {};
    virtual void onUnitComplete(BWAPI::Unit unit) {};
    virtual std::string GetName() const = 0;
};