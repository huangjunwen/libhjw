#include "aimod.h"
#include <set>

using namespace std;
using namespace BWAPI;

static int mapWidthPixel = 0;
static int mapHeightPixel = 0;
static int farEnough = 0;

void _unit_pair_handler(void * self, const node * n1, const node * n2) {
    ((BattleMgr *)self)->handle((Unit *)n1->attr, (Unit *)n2->attr);

}

void BattleMgr::run(set<Unit *> & units) {
	set<Unit *>::iterator iter;
	Unit * u;	
	dt.begin(_unit_pair_handler, this);
	for (iter = units.begin(); iter != units.end(); ++iter) {
		u = (*iter);
		Position pos = u->getPosition();
		if (pos.x() > mapWidthPixel || pos.y() > mapHeightPixel)
			continue;
		dt.next(pos.x(), pos.y(), u);
	}
	dt.end();

	map<Unit *, Unit *>::iterator target_iter;
	set<Unit *> all_targets;
	for (target_iter = _targets.begin(); target_iter != _targets.end(); ++target_iter) {
		if (exists(target_iter->first) && exists(u=target_iter->second))
			all_targets.insert(u);
	}
	Broodwar->drawTextScreen(5, 20, "target count: %d\n", all_targets.size());

	set<Unit *>::iterator titer;
	for (titer = all_targets.begin(); titer != all_targets.end(); ++titer) {
		u = *titer;
		Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), 20, Colors::Orange);
	}

   
	static int run_count = 0;
	if (++run_count >= 40){
		apply_target();
		run_count = 0;
	} 
}

void BattleMgr::apply_target() {
	map<Unit *, Unit *>::iterator iter;
	for (iter = _targets.begin(); iter != _targets.end(); ++iter) {
		Unit * self = iter->first;
		Unit * enemy = iter->second;
		if (exists(self) && exists(enemy) && self->getTarget() != enemy) {
			self->attackUnit(enemy);
		}
	}
}

Unit * BattleMgr::get_target(Unit * u) {
	map<Unit *, Unit *>::iterator iter = _targets.find(u);
	if (iter != _targets.end())
		return iter->second;
	return 0;
}

void BattleMgr::set_target(Unit * unit, Unit * enemy) {
	if (enemy) {
		_targets[unit] = enemy;
	}
	else
		_targets.erase(unit);
}

bool BattleMgr::better_target(Unit * u, Unit * ot, Unit * nt) {
	double old_dist = u->getDistance(ot);
	double new_dist = u->getDistance(nt);
	const WeaponType * w =  u->getType().groundWeapon();

	bool old_in_rng = w->minRange() < old_dist && w->maxRange() > old_dist;
	bool new_in_rng = w->minRange() < new_dist && w->maxRange() > new_dist;
	bool all_in_rng = old_in_rng && new_in_rng;
	if (!all_in_rng)
		return new_in_rng;
	return nt->getHitPoints() < ot->getHitPoints();
	//return old_dist > new_dist;
}

void BattleMgr::handle(Unit * u1, Unit * u2) {
    Color c;
	Player * p1 = u1->getPlayer();
	Player * p2 = u2->getPlayer();
	{
		if (p1 == p2)
			c = Colors::Green;
		else
			c = Colors::Red;
		Position pos1 = u1->getPosition();
		Position pos2 = u2->getPosition();
		Broodwar->drawLineMap(pos1.x(), pos1.y(), pos2.x(), pos2.y(), c);
	}
	{
		if (p1 == p2) {
			if (p1 == Broodwar->self()) {
				Unit * t1 = get_target(u1);
				Unit * t2 = get_target(u2);
				// if t2 exists and it's a better choice for u1
				if (exists(t2) && (!exists(t1) || better_target(u1, t1, t2)))
					set_target(u1, t2);
				// if t1 exists and it's a better choice for u2
				if (exists(t1) && (!exists(t2) || better_target(u2, t2, t1)))
					set_target(u2, t1);
			}
		}
		else {
			Unit * self_u = p1 == Broodwar->self() ? u1 : u2;
			Unit * enemy_u = self_u == u1 ? u2 : u1;
			Unit * t = get_target(self_u);
			if (!exists(t) || better_target(self_u, t, enemy_u))
				set_target(self_u, enemy_u);
		}
	}
}

void AI1::onStart()
{
    //Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(3);
	mapWidthPixel = Broodwar->mapWidth() * TILE_SIZE;
	mapHeightPixel = Broodwar->mapHeight() * TILE_SIZE;
	farEnough = mapWidthPixel + mapHeightPixel;
}

void AI1::onEnd(bool isWinner)
{
}

void AI1::onFrame()
{
    bm.run(Broodwar->getAllUnits());

    Unit * u;
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

	Broodwar->drawTextScreen(5, 10, "w: %d, h: %d\n", mapWidthPixel, mapHeightPixel);
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
