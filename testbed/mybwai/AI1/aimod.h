#pragma once
#include <BWAPI.h>
#include <windows.h>
#include <set>
#include <map>
#include "../../mydt/mydt.hpp"

class BattleMgr { 
public:
    void run(std::set<BWAPI::Unit *> &);
    virtual void handle(BWAPI::Unit *, BWAPI::Unit *);
private:
    MyDT dt;
	bool better_target(BWAPI::Unit *, BWAPI::Unit *, BWAPI::Unit *);
	BWAPI::Unit * get_target(BWAPI::Unit *);
	void set_target(BWAPI::Unit *, BWAPI::Unit *);
	void apply_target();
	bool exists(BWAPI::Unit * u) { return u ? u->exists() : false; } 
	std::map<BWAPI::Unit*, BWAPI::Unit*> _targets;	// remember the target for each our unit 
};

class AI1: public BWAPI::AIModule
{
public:
    virtual void onStart();
    virtual void onEnd(bool isWinner);
    virtual void onFrame();
    virtual bool onSendText(std::string text);
    virtual void onPlayerLeft(BWAPI::Player* player);
    virtual void onNukeDetect(BWAPI::Position target);
    virtual void onUnitCreate(BWAPI::Unit* unit);
    virtual void onUnitDestroy(BWAPI::Unit* unit);
    virtual void onUnitMorph(BWAPI::Unit* unit);
    virtual void onUnitShow(BWAPI::Unit* unit);
    virtual void onUnitHide(BWAPI::Unit* unit);
    virtual void onUnitRenegade(BWAPI::Unit* unit);
private:
    BattleMgr bm;
};
