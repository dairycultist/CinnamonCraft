#include "main.h"
#include "io.h"
#include "conn.h"
#include "terrain.h"
#include "player.h"

int main() {

	initialize_io();
	initialize_conn();

    send_packet(PKT_PRE_LOGIN, (Packet) { .pre_login = { "Steve" } });
    send_packet(PKT_LOGIN, (Packet) { .login = { 14, "Steve" } });

	initialize_terrain();
	initialize_entities();
	initialize_player();

	Input input = { 0 };

	// drain the packet queue every tick (3 frames)

	// todo (for movement + chunk loading)
	// - player pos and rot
	// - set chunk vis
	// - chunk

	while (game_is_running()) {

		populate_input(&input);
		
		entities_process_tick();
		player_process_tick(&input);

		present();
	}
	
	send_packet(PKT_DISCONNECT, (Packet) { .disconnect = { "disconnect.quitting" } });

	return 0;
}