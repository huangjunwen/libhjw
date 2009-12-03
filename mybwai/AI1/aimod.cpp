#include "aimod.h"
#include <set>

using namespace std;
using namespace BWAPI;

void drawEdge(const node * n1, const node * n2) {
    UnitData * d1 = (UnitData *)n1->attr;
    UnitData * d2 = (UnitData *)n2->attr;
    Unit * u1 = d1->unit;
    Unit * u2 = d2->unit;
    Color c;
    if (u1->getPlayer() == u2->getPlayer())
        c = Colors::Green;
    else
        c = Colors::Red;
    Position p1 = u1->getPosition();
    Position p2 = u2->getPosition();
    Broodwar->drawLineMap(p1.x(), p1.y(), p2.x(), p2.y(), c);
}


AI1::AI1():BWAPI::AIModule(), self(Broodwar->self()) {
	dt_create(&dt);
}

AI1::~AI1() {
	dt_destroy(&dt);
}

void AI1::add_unit(Unit * u) {
    if (is_self(u))
        self_units.add_unit(u);
    else
        enemy_units.add_unit(u);  
}

void AI1::remove_unit(Unit *u) {
    if (is_self(u))
        self_units.remove_unit(u);
    else
        enemy_units.remove_unit(u);
}

void AI1::onStart()
{
    Broodwar->enableFlag(Flag::UserInput);
    //Broodwar->setLocalSpeed(20);
    set<Unit *> & all_units = Broodwar->getAllUnits();
    set<Unit *>::iterator iter;
    for (iter = all_units.begin(); iter != all_units.end(); ++iter)
        add_unit(*iter);
}

void AI1::onEnd(bool isWinner)
{
}

void AI1::onFrame()
{
    UnitGrp::const_iterator iter;
    Unit * u;
    dt_begin(dt, drawEdge);
    for (iter = self_units.begin(); iter != self_units.end(); ++iter) {
        u = iter->first;
        dt_next(dt, u->getPosition().x(), u->getPosition().y(), iter->second);
    }

    for (iter = enemy_units.begin(); iter != enemy_units.end(); ++iter) {
        u = iter->first;
        dt_next(dt, u->getPosition().x(), u->getPosition().y(), iter->second);
    }

	dt_end(dt);
}

void AI1::onUnitCreate(BWAPI::Unit* u)
{
    add_unit(u);
}

void AI1::onUnitDestroy(BWAPI::Unit* u)
{
    remove_unit(u);
}

void AI1::onUnitMorph(BWAPI::Unit* u)
{
}

void AI1::onUnitShow(BWAPI::Unit* u)
{
    add_unit(u);
}

void AI1::onUnitHide(BWAPI::Unit* u)
{
    remove_unit(u);
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
