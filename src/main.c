#include "main.h"

#include "io.h"
#include "terrain.h"
#include "player.h"

int main() {

	printf("Starting CinnamonCraft\n");

	initialize_io();
	initialize_renderer();
	initialize_terrain();
	initialize_player();

	Input input = { 0 };

	while (io_keep_program_alive()) {

		io_populate_input(&input);
		
		// player stuff (drawing their perspective, handling their input, etc)
		player_process_tick(&input);

		io_present();
	}

	// exit (everything frees automatically, so)
	return 0;
}