#pragma once
#include <BWAPI.h>
class BuildOrder
{
public:
    virtual ~BuildOrder() = default;
    virtual void onStart() {};
    virtual void Execute() = 0;
    virtual void onEnd(bool isWinner) {};
    virtual void OnUnitCreate(BWAPI::Unit unit) {};
    virtual void onUnitComplete(BWAPI::Unit unit) {};
    virtual void onUnitShow(BWAPI::Unit unit) {};
    virtual void onUnitHide(BWAPI::Unit unit) {};
    virtual void onUnitMorph(BWAPI::Unit unit) {};
    virtual void onUnitDestroy(BWAPI::Unit unit) {};
    virtual void onUnitRenegade(BWAPI::Unit unit) {};
    virtual void onSendText(std::string text) {};
    virtual std::string GetName() const = 0;
};