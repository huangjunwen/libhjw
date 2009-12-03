#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include "dt.h"
#include "mem_pool.h"

class UnitNode: public node {
public:
	UnitNode(BWAPI::Unit * u);
    int X();
    int Y();
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
  AI1();
  virtual ~AI1();
private:
	memPool un_pool;
	myDt dt;
};
