#include "BasicSc2Bot.h"

bool BasicSc2Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type){
    const Unit* unit_to_build = GetProbe(ability_type_for_structure);

    float rx = GetRandomScalar();
    float ry = GetRandomScalar();
    
    if (ability_type_for_structure != ABILITY_ID::BUILD_ASSIMILATOR){
        Actions()->UnitCommand(unit_to_build, ability_type_for_structure,
                Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));

    }else{
		const Unit* vespene_target = FindNearestVespeneGeyser(unit_to_build->pos);
		Actions()->UnitCommand(unit_to_build, ability_type_for_structure, vespene_target);
    }
    return true;
}

bool BasicSc2Bot::TryBuildPylon(){
    const ObservationInterface* observation = Observation();

    if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2){
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) == 1) {
        built_first_pylon = true;
    }

    if (built_first_pylon && CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) == 0) {
        return false;
    }

    return TryBuildStructure(ABILITY_ID::BUILD_PYLON);
}

bool BasicSc2Bot::TryBuildAssimilator(){
    const ObservationInterface* observation = Observation();
    if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1){
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) < 1) {
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) == 1 && CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0) {
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) >= 2){
        return false;
    }

    return TryBuildStructure(ABILITY_ID::BUILD_ASSIMILATOR);
}

void BasicSc2Bot::OnGameStart() { 
    first_chrono = true;
    building_gateway = false;
    enemyBase = Observation()->GetGameInfo().enemy_start_locations.front();
    return;
}

size_t BasicSc2Bot::CountUnitType(UNIT_TYPEID unit_type){
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

 bool BasicSc2Bot::TryBuildGateway(){
    const ObservationInterface* observation = Observation();

    if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1){
        return false;
    }


    if ((CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) == 1 && CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0) || CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) > 3) {
        return false;
    }

    building_gateway = true;
    return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY);
 }

 bool BasicSc2Bot::TryBuildCyberneticsCore(){
    const ObservationInterface* observation = Observation();

    if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1){
        return false;
    }

    if (!building_gateway){
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) > 0){
        return false;
    }

    return TryBuildStructure(ABILITY_ID::BUILD_CYBERNETICSCORE);
 }

bool BasicSc2Bot::TryAttackWithStalker(){
    const ObservationInterface* observation = Observation();
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    const GameInfo& game_info = Observation()->GetGameInfo();
    bool stalker_attacking = false;

    if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) > stalkers_to_build) {
        for (const auto& u : units) {
            if (u->unit_type == UNIT_TYPEID::PROTOSS_STALKER) {
                StalkerAttack(u);
                stalker_attacking = true;
            }
        }
    }

    return stalker_attacking;
 }


void BasicSc2Bot::TryScoutWithProbe() {

    if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) < 1) {
        return;
    }

    const ObservationInterface* observation = Observation();
    const GameInfo& game_info = Observation()->GetGameInfo();
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    std::vector<Point2D> enemyStartLocation = game_info.enemy_start_locations;

    for (const auto& u:units) {
        if (u->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
            scout = u;
            scout_id = u->tag;
            break;
        }
    }
    
    for (int i = 0; i < enemyStartLocation.size(); i++) {
        Actions()->UnitCommand(scout, ABILITY_ID::ATTACK_ATTACK, enemyStartLocation[i], true);
    }

    scouting = true;
}

void BasicSc2Bot::checkScout() {
    if (!(scout->is_alive)) {
        const ObservationInterface* observation = Observation();
		const GameInfo& game_info = Observation()->GetGameInfo();
		std::vector<Point2D> enemyStartLocation = game_info.enemy_start_locations;
        Point2D closestEnemy = enemyStartLocation[0];

        scouted = true;
        Point2D scoutTempLocation;
        scoutTempLocation.x = scout->pos.x;
        scoutTempLocation.y = scout->pos.y;
        scouting = false;
        for (const auto location : enemyStartLocation) {
            if (DistanceSquared2D(location, scoutTempLocation) < DistanceSquared2D(closestEnemy, scoutTempLocation)) {
                closestEnemy = location;
            }
        }
        enemyBase = closestEnemy;
        /*
        std::cout << "scouted base at " << scoutTempLocation.x << ", " << scoutTempLocation.y << "\n";
        enemyBase = scoutTempLocation;
        */
    }
}

void BasicSc2Bot::TryChronoBoost() {
    const ObservationInterface* observation = Observation();
    const GameInfo& game_info = Observation()->GetGameInfo();
    const Unit* nexus = FindNexus();
    if (first_chrono && (nexus->energy >= 50)) {
        // ABILITY_ID::EFFECT_CHRONOBOOST does not work, ABILITY_ID(3755) is chronoboost
        Actions()->UnitCommand(nexus, ABILITY_ID(3755), nexus);
        first_chrono = false;
    }
   
}

const Unit* BasicSc2Bot::GetProbe(ABILITY_ID ability_type_for_structure){
    // Get a probe which is not harvesting vespene gas and has order of building
    Units units = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::PROTOSS_PROBE));
    for (const auto& unit : units){
        if (unit->tag != scout_id) {
            for (const auto& order : unit->orders) {
                if (order.ability_id != ABILITY_ID::HARVEST_GATHER && order.ability_id != ability_type_for_structure) {
                    return unit;
                }
            }
        }
    }

    return units[0];
 }

 void BasicSc2Bot::TryFillVespeneGas(){
    // Mine vespene if vespene gas not full
    Units vespene_gas = Observation()->GetUnits(Unit::Alliance::Self, IsVisibleGeyser());
    const auto unit = GetProbe(ABILITY_ID::HARVEST_GATHER);
    if (CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) == 0){
        return;
    }
    for (const auto& gas : vespene_gas) {
        if (gas->build_progress == 1) {
            if (gas->assigned_harvesters < gas->ideal_harvesters) {
                Actions()->UnitCommand(unit, ABILITY_ID::HARVEST_GATHER, gas);
            }
            else if (gas->assigned_harvesters > gas->ideal_harvesters) {
                const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
                Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
            }
        }
    }         
 }

 void BasicSc2Bot::TryTrainStalker() {
     const ObservationInterface* observation = Observation();
     const GameInfo& game_info = Observation()->GetGameInfo();
     Units units = Observation()->GetUnits(Unit::Alliance::Self);
     const Unit *nexus = FindNexus();
     bool cyber = false;
     
     for (const auto& u : units) {
         if (u->unit_type == UNIT_TYPEID::PROTOSS_CYBERNETICSCORE && u->build_progress == 1) {
             cyber = true;
        }
     }

     // need fully built cybernetics core to train stalkers
     if (!cyber) {
         return;
     }

     for (const auto& u : units) {
         if (u->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY) {
             if (CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) >= 1 && u->orders.size() == 0 && Observation()->GetMinerals() >= 125 && Observation()->GetVespene()) {
                 Actions()->UnitCommand(u, ABILITY_ID::TRAIN_STALKER);
                 if (nexus->energy >= 50) {
					 Actions()->UnitCommand(nexus, ABILITY_ID(3755), u);
                 }
             }
         }
     }
    

 }

void BasicSc2Bot::OnStep() {
    if (!scouted && !scouting) {
        TryScoutWithProbe();
    }
    if (!scouted) {
        checkScout();
    }
    TryTrainStalker();
    TryBuildPylon();
    TryBuildAssimilator();
    TryFillVespeneGas();
    TryBuildCyberneticsCore();
    StalkerCommander();
    TryBuildGateway();
    TryChronoBoost();
    return; 
}

const Unit* BasicSc2Bot::FindNearestMineralPatch(const Point2D& start){
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units){
        if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD){
            float d = DistanceSquared2D(u->pos, start);
            if (d<distance){
                distance = d;
                target = u;
            }
        }
    }
    return target;
}

const Unit* BasicSc2Bot::FindNearestVespeneGeyser(const Point2D& start){
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
    const GameInfo& game_info = Observation()->GetGameInfo();
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    
    for (const auto& u : units){
        if (game_info.local_map_path != "ProximaStationLE.SC2Map"){
            if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER){
                float d = DistanceSquared2D(u->pos, start);
                if (d<distance){
                    distance = d;
                    target = u;
                }
            }
        }else{
            if (u->unit_type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER){
                float d = DistanceSquared2D(u->pos, start);
                if (d<distance){
                    distance = d;
                    target = u;
                }
            }
        }
    }
    return target;
}

const Unit* BasicSc2Bot::FindNearestAssimilator(const Point2D& start){
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units){
        if (u->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR){
            float d = DistanceSquared2D(u->pos, start);
            if (d<distance){
                distance = d;
                target = u;
            }
        }
    }
    return target;
}


const Unit* BasicSc2Bot::FindNexus() {
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units) {
        if (u->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
            float d = DistanceSquared2D(u->pos, Point2D(0,0));
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}

const Unit* BasicSc2Bot::FindNearestGateway(const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units) {
        if (u->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY) {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}

void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
    switch (unit->unit_type.ToType()) {
        case UNIT_TYPEID::PROTOSS_NEXUS: {
            if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) < 22) {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
            }
            break;
        }
        case UNIT_TYPEID::PROTOSS_PROBE: {
            if (unit->tag == scout_id) {
                break;
            }
			// mine mineral if idle
			const Unit* mineral_target = FindNearestMineralPatch(Observation()->GetStartLocation());
			if (!mineral_target) {
				break;
;
			}
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
			break;
        }
        case UNIT_TYPEID::PROTOSS_STALKER: {
            const Unit* nexus = FindNexus();
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, nexus);
            break;
        }
        case UNIT_TYPEID::PROTOSS_GATEWAY: {
        }
        default: {
            break;
        }
    }
}

void BasicSc2Bot::OnUnitCreated(const Unit* unit) {
   switch (unit->unit_type.ToType()){
        case UNIT_TYPEID::PROTOSS_STALKER:{
            //std::cout << "STALKER CREATED" << std::endl;
        }
        default: {
            break;
        }
    }
}

void BasicSc2Bot::StalkerAttack(const Unit* stalker) {
    const Unit *nexus = FindNexus();
    const Unit *enemy = nullptr;
        if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) > stalkers_to_build) {
            if (stalker->weapon_cooldown <= 0) {
                enemy = FindNearestEnemy(stalker->pos);
                // no enemies seen
                if (enemy == nullptr) {
                    Actions()->UnitCommand(stalker, ABILITY_ID::ATTACK_ATTACK, enemyBase);
                }
                else {
                    Actions()->UnitCommand(stalker, ABILITY_ID::ATTACK_ATTACK, enemy);
                }
            }
            else if (stalker->shield < stalker->shield_max){
				Actions()->UnitCommand(stalker, ABILITY_ID::MOVE_MOVE, nexus);
			}
        }
}

void BasicSc2Bot::StalkerDefend(const Unit *stalker, const Unit *attacker, const Point3D &kite) {
    if (stalker->weapon_cooldown <= 0) {
        Actions()->UnitCommand(stalker, ABILITY_ID::ATTACK_ATTACK, attacker);
    }
    else if (stalker->shield < stalker->shield_max) {
        Actions()->UnitCommand(stalker, ABILITY_ID::MOVE_MOVE, kite);
    }
}

void BasicSc2Bot::StalkerCommander() {
    const ObservationInterface* observation = Observation();
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    Units enemies = Observation()->GetUnits(Unit::Enemy);
    const GameInfo& game_info = Observation()->GetGameInfo();
    const Unit *nexus = FindNexus();
    const Unit *attacker = nullptr;
    float distance_to_spawn;

    // try to attack when no enemies seen
    if (enemies.empty()) {
        TryAttackWithStalker();
        return;
    }

    // get an attacker in the defense range of nexus
    attacker = FindNearestEnemy(nexus->pos);
    if (DistanceSquared2D(nexus->pos, attacker->pos) > defense_range) {
        attacker = nullptr;
    }

    for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::PROTOSS_STALKER) {
            distance_to_spawn = DistanceSquared2D(nexus->pos, u->pos);
			if (distance_to_spawn < defense_range) {
                if (attacker == nullptr) {
                    // no enemy in defense range
                    StalkerAttack(u);
                }
                else {
                    // stalkers in defense range attack the attacker
                    StalkerDefend(u, attacker, nexus->pos);
                }
			}
			else {
                // continue attacking if way outside of defense range
                if (distance_to_spawn >= defense_range) {
                    StalkerAttack(u);
                }
                else {
                    // within defense range
                    std::cout << distance_to_spawn << std::endl;
                    Actions()->UnitCommand(u, ABILITY_ID::MOVE_MOVE, nexus->pos);
                }
			}
		}
	}
}

const Unit* BasicSc2Bot::FindNearestEnemy(const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Enemy);
    float distance = std::numeric_limits<float>::max();
    const Unit* enemy = nullptr;
     for (const auto& u : units){
            float d = DistanceSquared2D(u->pos, start);
            if (d<distance){
                distance = d;
                enemy = u;
            }
    }

     return enemy;
}
