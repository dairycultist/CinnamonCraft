#include "main.h"
#include "io.h"
#include "terrain.h"
#include "player.h"

int main() {

	initialize_io();
	initialize_terrain();
	initialize_player();

	Input input = { 0 };

	while (game_is_running()) {

		populate_input(&input);
		
		player_process_tick(&input);

		present();
	}

	// exit (everything frees automatically, so)
	return 0;
}