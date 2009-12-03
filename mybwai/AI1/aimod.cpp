#include "aimod.h"
#include <set>

using namespace std;
using namespace BWAPI;

void drawEdge(const node * n1, const node * n2) {
    UnitNode * un1 = (UnitNode *)n1;
    UnitNode * un2 = (UnitNode *)n2;
    Color c;
    if (un1->unit()->getPlayer() == un2->unit()->getPlayer())
        c = Colors::Green;
    else
        c = Colors::Red;
    Broodwar->drawLineMap(un1->X(), un1->Y(), un2->X(), un2->Y(), c);
}


UnitNode::UnitNode(Unit * u) {
    attr = u;
	x = X();
	y = Y();
}

Unit * UnitNode::unit() {
    return (Unit *)attr;
}

int UnitNode::X() {
    return unit()->getPosition().x();
}

int UnitNode::Y() {
    return unit()->getPosition().y();
}


AI1::AI1():BWAPI::AIModule() {
	mem_pool_init(&un_pool, sizeof(UnitNode), 256);
	dt_create(&dt);
}

AI1::~AI1() {
	mem_pool_finalize(&un_pool);
	dt_destroy(&dt);
}

void AI1::onStart()
{
    Broodwar->enableFlag(Flag::UserInput);
}

void AI1::onEnd(bool isWinner)
{
}

void AI1::onFrame()
{
	set<Unit *> & all_units = Broodwar->getAllUnits();
	set<Unit *>::iterator iter;
    mem_pool_reset(&un_pool);
    dt_begin(dt, drawEdge);
	for (iter = all_units.begin(); iter != all_units.end(); ++iter) {
		UnitNode * un = new(mem_pool_get(&un_pool)) UnitNode(*iter);
        dt_next(dt, (node *)un);
	}
	dt_end(dt);
}

void AI1::onUnitCreate(BWAPI::Unit* unit)
{
}

void AI1::onUnitDestroy(BWAPI::Unit* unit)
{
}

void AI1::onUnitMorph(BWAPI::Unit* unit)
{
}

void AI1::onUnitShow(BWAPI::Unit* unit)
{
}

void AI1::onUnitHide(BWAPI::Unit* unit)
{
}

void AI1::onUnitRenegade(BWAPI::Unit* unit)
{
}

void AI1::onPlayerLeft(BWAPI::Player* player)
{
}

void AI1::onNukeDetect(BWAPI::Position target)
{
}

bool AI1::onSendText(std::string text)
{
    return true;
}
