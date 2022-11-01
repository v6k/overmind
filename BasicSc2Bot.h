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
	void TryAttacWithStalker();
private:
};

#endif