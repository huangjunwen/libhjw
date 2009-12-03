#include "unit_data.h"

using namespace BWAPI;

UnitData * UnitGrp::add_unit(Unit * u) {
    ContType::iterator iter = units.find(u);
    if (iter != units.end())
        return iter->second;
    UnitData * d = new(pool.get()) UnitData(u);
    units[u] = d;
    return d;
}

bool UnitGrp::remove_unit(Unit * u) {
    ContType::iterator iter = units.find(u);
    if (iter == units.end())
        return false;
    iter->second->~UnitData();
    pool.release(iter->second);
    units.erase(iter);
    return true;
}

