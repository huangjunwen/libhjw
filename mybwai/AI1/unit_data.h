#pragma once
#include <BWAPI.h>
#include <hash_map>
#include "mem_pool.hpp"
#include "dt.h"

// add additional attributes to this to remember state across different frames
class UnitData {
public:
	UnitData(BWAPI::Unit * u): unit(u) {}
private:
    BWAPI::Unit * unit;
};

class UnitGrp {
public:
    typedef stdext::hash_map<BWAPI::Unit *, UnitData *> ContType;
    typedef ContType::const_iterator const_iterator;

    // modify
    UnitData * add_unit(BWAPI::Unit *);
    bool remove_unit(BWAPI::Unit *);
    UnitData * find_unit(BWAPI::Unit *);
    // iterations
    const_iterator begin() const { return units.begin(); }
    const_iterator end() const { return units.end(); }
    const_iterator find(BWAPI::Unit * u) const { return units.find(u); }
private:
    ContType units;
    MemPool<UnitData> pool;
};
