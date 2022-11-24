#include "BasicSc2Bot.h"

bool BasicSc2Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type){
    const ObservationInterface* observation = Observation();

    // If  a unit already is building a supply structure of this type, do nothing.
    // Also get an scv to build the structure.
    const Unit* unit_to_build = nullptr;
    Units units = observation->GetUnits(Unit::Alliance::Self);

    // For getting vespene geyser location, and building assimilator on top of it
    const Unit* vespene_geyser = nullptr;
    Units vespene_gas = observation->GetUnits(Unit::Alliance::Neutral);

    for (const auto& unit : units){
        for (const auto& order : unit->orders){
            if (order.ability_id == ability_type_for_structure){
                return false;
            }
        }
        if (unit->unit_type == unit_type){
            unit_to_build = unit;
        }
    }

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

    return TryBuildStructure(ABILITY_ID::BUILD_PYLON);
}


bool BasicSc2Bot::TryBuildAssimilator(){
    const ObservationInterface* observation = Observation();
    if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1){
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::PROTOSS_ASSIMILATOR) > 2){
        return false;
    }

    return TryBuildStructure(ABILITY_ID::BUILD_ASSIMILATOR);
}


void BasicSc2Bot::OnGameStart() { 
    std::cout << "Hello, World!" << std::endl;
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

    if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) > 1){
        return false;
    }

    return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY);
 }

 bool BasicSc2Bot::TryBuildCyberneticsCore(){
    const ObservationInterface* observation = Observation();

    if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1){
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) < 1){
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
                Actions()->UnitCommand(u, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
                stalker_attacking = true;
            }
        }
    }

    return stalker_attacking;
 }


void BasicSc2Bot::OnStep() {
    TryBuildPylon();
    TryBuildAssimilator();
    TryBuildGateway();
    TryBuildCyberneticsCore();
    StalkerCommander();
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
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units){
        if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER){
            float d = DistanceSquared2D(u->pos, start);
            if (d<distance){
                distance = d;
                target = u;
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

void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
    switch (unit->unit_type.ToType()){
        case UNIT_TYPEID::PROTOSS_NEXUS:{
            if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) < 25){
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
            }
            break;
        }
        case UNIT_TYPEID::PROTOSS_PROBE:{
            if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) > 15){
                const Unit* assimilator_target = FindNearestAssimilator(unit->pos);
                if (!assimilator_target){
                    break;
                }
                Actions()->UnitCommand(unit, ABILITY_ID::SMART, assimilator_target);
           }else{
                const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
                if (!mineral_target){
                    break;
                }
                Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
                break;
            }
        }
        case UNIT_TYPEID::PROTOSS_STALKER: {
            Units units = Observation()->GetUnits();
            const Unit* nexsus = nullptr;
            for (auto& u : units) {
                if (u->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
                    nexsus = u;
                }
            }
            Actions()->UnitCommand(unit, ABILITY_ID::MOVE_MOVE, nexsus);
            break;
        }
        case UNIT_TYPEID::PROTOSS_GATEWAY:{
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_STALKER);
            break;
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
    if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) > stalkers_to_build) {
        Actions()->UnitCommand(stalker, ABILITY_ID::ATTACK_ATTACK, Observation()->GetGameInfo().enemy_start_locations.front());
    }
}

void BasicSc2Bot::StalkerDefend(const Unit *stalker, const Unit *attacker) {
    Actions()->UnitCommand(stalker, ABILITY_ID::ATTACK_ATTACK, attacker);
}

void BasicSc2Bot::StalkerCommander() {
    const ObservationInterface* observation = Observation();
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    Units enemies = Observation()->GetUnits(Unit::Enemy);
    const GameInfo& game_info = Observation()->GetGameInfo();
    const Unit *nexsus = nullptr;
    const Unit *attacker = nullptr;
    float distance_to_spawn;

    // try to attack when no enemies seen
    if (enemies.empty()) {
        TryAttackWithStalker();
        return;
    }

    // get nexsus to use as starting point to defend
    for (const auto& u : units) {
        if (u->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
            nexsus = u;
        }
    }

    // get an attacker in the defense range of nexsus
    attacker = FindNearestEnemy(nexsus->pos);
    if (DistanceSquared2D(nexsus->pos, attacker->pos) > defense_range) {
        attacker = nullptr;
    }

    for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::PROTOSS_STALKER) {
            distance_to_spawn = DistanceSquared2D(nexsus->pos, u->pos);
			if (distance_to_spawn < defense_range) {
                if (attacker == nullptr) {
                    // no enemy in defense range
                    StalkerAttack(u);
                }
                else {
                    // stalkers in defense range attack the attacker
                    StalkerDefend(u, attacker);
                }
			}
			else {
                // continue attacking if way outside of defense range
                if (distance_to_spawn > defense_range + 20) {
                    StalkerAttack(u);
                }
                else {
                    // outside of defense range but not too far
                    Actions()->UnitCommand(u, ABILITY_ID::MOVE_MOVE, nexsus);
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
