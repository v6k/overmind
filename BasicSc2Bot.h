#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2api/sc2_unit_filters.h"
#include <random>

using namespace sc2;

class BasicSc2Bot : public Agent {
public:
	virtual void OnGameStart();
	virtual void OnStep();
	virtual void OnUnitIdle(const Unit*);
	virtual void OnUnitCreated(const Unit*);
	size_t CountUnitType(UNIT_TYPEID unit_type);
	bool TryBuildGateway();
	bool TryBuildCyberneticsCore();
	bool TryBuildAssimilator();
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::PROTOSS_PROBE);
	bool TryBuildPylon();
	const Unit* FindNearestMineralPatch(const Point2D& start);
	const Unit* FindNearestVespeneGeyser(const Point2D& start);
	const Unit* FindNearestAssimilator(const Point2D& start);
	bool TryAttackWithStalker();
	void StalkerAttack(const Unit *stalker);
	void StalkerDefend(const Unit *stalker, const Unit *attacker, const Point3D &kite);
	void StalkerCommander();
	const Unit* FindNearestEnemy(const Point2D& start);
	const Unit* FindNexus();
	const Unit* FindNearestGateway(const Point2D& start);
	void TryScoutWithProbe();
	void checkScout();
	void TryFillVespeneGas();
	const Unit* GetProbe(ABILITY_ID ability_type_for_structure);
	void TryChronoBoost();
	void TryTrainStalker();

	private:
	bool first_chrono;
	bool building_gateway;
	bool built_first_pylon = false;
	Point2D enemyBase;
	bool scouted = false;
	bool scouting = false;
	const Unit* scout;
	Point3D scoutLocation;
	Tag scout_id;
	// can play around with values
	int defense_range = pow(double(30), double(2)); // defense_range^2 
	int stalkers_to_build = 10;
};

#endif
