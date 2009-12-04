#include "aimod.h"
#include <set>

using namespace std;
using namespace BWAPI;

void UnitPairHandler::operator() (Unit * u1, Unit * u2) {
    Color c;
    if (u1->getPlayer() == u2->getPlayer())
        c = Colors::Green;
    else
        c = Colors::Red;
    Position p1 = u1->getPosition();
    Position p2 = u2->getPosition();
    Broodwar->drawLineMap(p1.x(), p1.y(), p2.x(), p2.y(), c);
}


void AI1::onStart()
{
    Broodwar->enableFlag(Flag::UserInput);
    //Broodwar->setLocalSpeed(20);
}

void AI1::onEnd(bool isWinner)
{
}

void AI1::onFrame()
{
    set<Unit *> & all_units = Broodwar->getAllUnits();
    set<Unit *>::iterator iter;
    Unit * u;
    Broodwar->drawTextScreen(5, 10, "total units: %d\n", all_units.size());

    UnitPairHandler handler;
    dt.begin(handler);
    for (iter = all_units.begin(); iter != all_units.end(); ++iter) {
        u = (*iter);
        Position pos = u->getPosition();
        dt.next(pos.x(), pos.y(), u);
    }
    dt.end();

    set<Unit *> & selected = Broodwar->getSelectedUnits();
    set<Unit *>::iterator uiter = selected.begin();
    if (selected.size() == 1) {
        u = *uiter;
        Broodwar->drawTextScreen(5, 0, "%s at x: %d, y: %d\n", u->getType().getName().c_str(), u->getPosition().x(), u->getPosition().y());
    } else if (selected.size() == 2) {
        u = *uiter;
        Broodwar->drawTextScreen(5, 0, "distance between two units %f\n", u->getDistance(*(++uiter)));
    } else {
        Broodwar->drawTextScreen(5, 0, "select one unit or two\n");
    }
}

void AI1::onUnitCreate(BWAPI::Unit* u)
{
}

void AI1::onUnitDestroy(BWAPI::Unit* u)
{
}

void AI1::onUnitMorph(BWAPI::Unit* u)
{
}

void AI1::onUnitShow(BWAPI::Unit* u)
{
}

void AI1::onUnitHide(BWAPI::Unit* u)
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
