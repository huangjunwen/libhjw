#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include "dt.h"
#include "unit_data.h"

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
  AI1();
  virtual ~AI1();
private:
    const BWAPI::Player * self;
    bool is_self(BWAPI::Unit * u) { return u->getPlayer() == self; }
    void add_unit(BWAPI::Unit * u);
    void remove_unit(BWAPI::Unit *u);
    UnitGrp self_units;
    UnitGrp enemy_units;
	myDt dt;
};
