#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "BasicSc2Bot.h"
#include "LadderInterface.h"

// LadderInterface allows the bot to be tested against the built-in AI or
// played against other bots
int main(int argc, char* argv[]) {
	RunBot(argc, argv, new BasicSc2Bot(), sc2::Race::Protoss);
	/*
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);

	BasicSc2Bot bot;
	sc2::Agent player;
	coordinator.SetParticipants({
		sc2::CreateParticipant(sc2::Race::Protoss, &player),
		sc2::CreateParticipant(sc2::Race::Protoss, &bot)
		});
	coordinator.LaunchStarcraft();
	coordinator.StartGame(sc2::kMapBelShirVestigeLE);

	while (coordinator.Update()) {

	}
	*/
	return 0;
}