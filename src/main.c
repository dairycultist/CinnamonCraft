#include "main.h"
#include "io.h"
#include "conn.h"
#include "terrain.h"
#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {

	initialize_io();
	initialize_terrain();
	initialize_entities();
	initialize_player();
	initialize_conn();

    send_packet(PKT_PRE_LOGIN, (Packet) { .pre_login = { "Steve" } });
    send_packet(PKT_LOGIN, (Packet) { .login = { 14, "Steve" } });

	Input input = { 0 };

	int frames_until_tick = 3;

	while (game_is_running()) {

		if (--frames_until_tick == 0) {

			frames_until_tick = 3;

			// drain the packet queue every tick
			int limit = 20;

			Packet packet;
			packet_t type;
			
			while (--limit > 0 && (type = read_packet(&packet)) != PKT_EOB) {
				
				// printf("Got packet: 0x%02x\n", type);

				if (type == PKT_SET_CHUNK_VISIBILITY) {

					if (packet.set_chunk_visibility.load)
						create_chunk_at(packet.set_chunk_visibility.x, packet.set_chunk_visibility.z);
					else
						destroy_chunk_at(packet.set_chunk_visibility.x, packet.set_chunk_visibility.z);

				} else if (type == PKT_CHUNK) {

					// TODO
					// void set_delay_remesh_block_at(int x, int y, int z, block_t block);
					// void remesh_delayed_chunks();
				
				} else if (type == PKT_SET_MULTIPLE_BLOCKS) {

					printf("Got packet: 0x%02x\n", type);

					for (int i = 0; i < packet.set_multiple_blocks.block_count; i++) {

						set_delay_remesh_block_at(
							((packet.set_multiple_blocks.block_positions[i] >> 12) & 0x0F) + 16 * packet.set_multiple_blocks.x,
							((packet.set_multiple_blocks.block_positions[i]      ) & 0xFF) + 16 * packet.set_multiple_blocks.z,
							((packet.set_multiple_blocks.block_positions[i] >>  8) & 0x0F),
							BLOCK_GRASS
						);
					}

					remesh_delayed_chunks();

					free(packet.set_multiple_blocks.block_positions);
					free(packet.set_multiple_blocks.blocks);
					free(packet.set_multiple_blocks.block_metadatas);

				} else if (type == PKT_SET_BLOCK) {

					set_block_at(packet.set_block.x, packet.set_block.y, packet.set_block.z, BLOCK_GRASS);
				
				} else if (type == PKT_PLAYER_POSITION_AND_ROTATION) {

					set_player_position(packet.player_position_and_rotation.x, packet.player_position_and_rotation.y, packet.player_position_and_rotation.z);
					set_player_rotation(packet.player_position_and_rotation.yaw, packet.player_position_and_rotation.pitch);
				}
			}

			// send packets
			float x, y, z, camera_y, yaw, pitch;
			int grounded;

			get_player_information(&x, &y, &z, &camera_y, &yaw, &pitch, &grounded);

			send_packet(PKT_PLAYER_POSITION_AND_ROTATION, (Packet) { .player_position_and_rotation = { x, camera_y, y, z, yaw, pitch, grounded } });
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