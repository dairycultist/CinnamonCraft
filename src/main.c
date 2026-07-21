#include "main.h"
#include "io.h"
#include "conn.h"
#include "terrain.h"
#include "player.h"

int main() {

	initialize_io();
	initialize_conn();
	initialize_terrain();
	initialize_entities();
	initialize_player();

	Input input = { 0 };

	// drain the packet queue every tick (3 frames)

	while (game_is_running()) {

		populate_input(&input);
		
		entities_process_tick();
		player_process_tick(&input);

		present();
	}

	// exit (everything frees automatically, so)
	return 0;
}