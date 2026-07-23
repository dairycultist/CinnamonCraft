#include "main.h"
#include "io.h"
#include "conn.h"
#include "terrain.h"
#include "player.h"

#include <stdio.h>
#include <math.h>

static void populator(int x, int y, int z) {

	set_delay_remesh_block_at(
		x, y, z,
		sin(x * 0.1) * 16 + 16 < y ? 0 : (y < 20 ? 2 : 1)
	);
}

int main() {

	initialize_io();
	initialize_conn();

    send_packet(PKT_PRE_LOGIN, (Packet) { .pre_login = { "Steve" } });
    send_packet(PKT_LOGIN, (Packet) { .login = { 14, "Steve" } });

	initialize_terrain();

	// initialize chunks
	for (int chunk_x = 0; chunk_x < 4; chunk_x++) {
		for (int chunk_z = 0; chunk_z < 4; chunk_z++) {

			create_chunk_at(chunk_x, chunk_z);

			for (int x = 0; x < 16; x++) {
				for (int y = 0; y < 128; y++) {
					for (int z = 0; z < 16; z++) {

						populator(
							x + chunk_x * 16,
							y,
							z + chunk_z * 16
						);
					}
				}
			}
		}
	}

	// remesh every chunk at once (can't do right after populating because it needs
	// its neighbor to be loaded to be able to remesh at its chunk boundary properly)
	remesh_delayed_chunks();

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
				
				printf("Got packet: 0x%02x\n", type);

				if (type == PKT_SET_CHUNK_VISIBILITY) {

				}
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