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
    first_chrono = true;
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

void BasicSc2Bot::TryAttacWithStalker(){
    const ObservationInterface* observation = Observation();
    Units units = Observation()->GetUnits(Unit::Alliance::Self);
    const GameInfo& game_info = Observation()->GetGameInfo();

    if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) > 10){
        for (const auto& u : units){
            if (u->unit_type == UNIT_TYPEID::PROTOSS_STALKER){
                Actions()->UnitCommand(u, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
            }
        }
    }
 }


void BasicSc2Bot::TryChronoBoost() {
    const ObservationInterface* observation = Observation();
    const GameInfo& game_info = Observation()->GetGameInfo();
    const Unit* nexus = FindNexus();
    if (first_chrono && (nexus->energy >= 50)) {
        //Actions()->UnitCommand(nexus, );
        // ABILITY_ID::EFFECT_CHRONOBOOST does not work, ABILITY_ID(3755) is chronoboost
        Actions()->UnitCommand(nexus, ABILITY_ID(3755), nexus);
        first_chrono = false;
    }
    else if (nexus->energy >= 50) {
        const Unit* gateway_target = FindNearestGateway(nexus->pos);
        if (gateway_target) {
            Actions()->UnitCommand(nexus, ABILITY_ID(3755), gateway_target);
        }
    }
}

void BasicSc2Bot::OnStep() {
    TryBuildPylon();
    TryBuildAssimilator();
    TryBuildGateway();
    TryBuildCyberneticsCore();
    TryAttacWithStalker();
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
            }
        }
        case UNIT_TYPEID::PROTOSS_GATEWAY:{
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_STALKER);
        }

        default: {
            break;
        }
    }
}


void BasicSc2Bot::OnUnitCreated(const Unit* unit) {
   switch (unit->unit_type.ToType()){
        case UNIT_TYPEID::PROTOSS_STALKER:{
            std::cout << "STALKER CREATED" << std::endl;
        }
        default: {
            break;
        }
    }
}