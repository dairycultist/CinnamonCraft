#include "main.h"
#include "io.h"
#include "conn.h"
#include "terrain.h"
#include "player.h"

#include <stdio.h>

int main() {

	initialize_io();
	initialize_conn();

    send_packet(PKT_PRE_LOGIN, (Packet) { .pre_login = { "Steve" } });
    send_packet(PKT_LOGIN, (Packet) { .login = { 14, "Steve" } });

	initialize_terrain();
	initialize_entities();
	initialize_player();

	Input input = { 0 };

	int frames_until_tick = 3;

	// todo (for movement + chunk loading)
	// - player pos and rot
	// - set chunk vis
	// - chunk

	while (game_is_running()) {

		if (--frames_until_tick == 0) {

			frames_until_tick = 3;

			// drain the packet queue every tick
			int limit = 20;

			Packet packet;
			packet_t type;
			
			while (--limit > 0 && (type = read_packet(&packet)) != PKT_EOB) {
				
				printf("%d\n", type);
			}
		}

		populate_input(&input);
		
		entities_process_tick();
		player_process_tick(&input);

		present();
	}

	send_packet(PKT_DISCONNECT, (Packet) { .disconnect = { "disconnect.quitting" } });

	// sometimes the socket closes so fast that this final packet doesn't get through (if you sleep it works), kinda lame but idc rn

	return 0;
}