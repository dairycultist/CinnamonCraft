#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <zlib.h> // TODO https://refspecs.linuxbase.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/zlib-uncompress-1.html

#include "main.h"
#include "conn.h"

static int sock;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htonll(x) __builtin_bswap64(x)
#define ntohll(x) (x)
#else
#define htonll(x) (x)
#define ntohll(x) __builtin_bswap64(x)
#endif

#define SEND_I8(value)  { int8_t  temp = value;         send(sock, &temp, 1, 0); }
#define SEND_I16(value) { int16_t temp = htons (value); send(sock, &temp, 2, 0); }
#define SEND_I32(value) { int32_t temp = htonl (value); send(sock, &temp, 4, 0); }
#define SEND_I64(value) { int64_t temp = htonll(value); send(sock, &temp, 8, 0); }

#define SEND_F32(value) { int32_t temp; memcpy(&temp, &(value), 4); temp = htonl (temp); send(sock, &temp, 4, 0); }
#define SEND_F64(value) { int64_t temp; memcpy(&temp, &(value), 8); temp = htonll(temp); send(sock, &temp, 8, 0); }

#define READ_I8(buffer)  { read(sock, (buffer), 1); }
#define READ_I16(buffer) { read(sock, (buffer), 2); int16_t temp = ntohs (*((int16_t *) (buffer))); memcpy((buffer), &temp, 2); }
#define READ_I32(buffer) { read(sock, (buffer), 4); int32_t temp = ntohl (*((int32_t *) (buffer))); memcpy((buffer), &temp, 4); }
#define READ_I64(buffer) { read(sock, (buffer), 8); int64_t temp = ntohll(*((int64_t *) (buffer))); memcpy((buffer), &temp, 8); }

#define READ_F32(buffer) READ_I32(buffer)
#define READ_F64(buffer) READ_I64(buffer)

static void send_string16(const char *string) {

    int16_t len = strlen(string);

    SEND_I16(len);

    for (int i = 0; i < len; i++) {

        SEND_I8(0x00);
        SEND_I8(string[i]);
    }
}

static void read_string16(char *out) {

    int16_t len;

    READ_I16(&len);

    for (int i = 0; i < len; i++) {

        READ_I8(out + i); // first one is always 0x00
        READ_I8(out + i);
    }

    // null terminator
    out[len] = '\0';
}

static void read_entity_metadata() { // TODO an actual metadata datatype...

    int8_t header;
    READ_I8(&header);

    int8_t unused_buf[256];
    
    while (header != 0x7F) {
        
        switch (header >> 5) {
            case 0: READ_I8(unused_buf); break;
            case 1: READ_I16(unused_buf); break;
            case 2: READ_I32(unused_buf); break;
            case 3: READ_I32(unused_buf); break; // technically float
            case 4: read_string16(unused_buf); break;
            case 5: READ_I16(unused_buf); READ_I8(unused_buf); READ_I16(unused_buf); break;
            case 6: READ_I32(unused_buf); READ_I32(unused_buf); READ_I32(unused_buf); break;
        }

        READ_I8(&header);
    }
}

void send_packet(packet_t type, Packet data) {

    // send packet type
    send(sock, &type, 1, 0);

    // send packet data
    switch (type) {

        case PKT_LOGIN:
            SEND_I32(data.login.int_val);
            send_string16(data.login.string);
            SEND_I64(data.login.long_val);
            SEND_I8(data.login.byte_val);
            break;

        case PKT_PRE_LOGIN:
            send_string16(data.pre_login.string);
            break;

        case PKT_SET_TIME: break;             // never sent by client
        case PKT_SET_SPAWN_POSITION: break;   // never sent by client

        case PKT_PLAYER_POSITION_AND_ROTATION:
            SEND_F64(data.player_position_and_rotation.x);
            SEND_F64(data.player_position_and_rotation.camera_y);
            SEND_F64(data.player_position_and_rotation.y);
            SEND_F64(data.player_position_and_rotation.z);
            SEND_F32(data.player_position_and_rotation.yaw);
            SEND_F32(data.player_position_and_rotation.pitch);
            SEND_I8(data.player_position_and_rotation.on_ground);
            break;

        case PKT_SPAWN_ITEM: break;                   // never sent by client
        case PKT_SPAWN_MOB: break;                    // never sent by client
        case PKT_ENTITY_VELOCITY: break;              // never sent by client
        case PKT_DESPAWN_ENTITY: break;               // never sent by client
        case PKT_ENTITY_POSITION: break;              // never sent by client
        case PKT_ENTITY_POSITION_AND_ROTATION: break; // never sent by client
        case PKT_TELEPORT_ENTITY: break;              // never sent by client
        case PKT_ENTITY_EVENT: break;                 // never sent by client
        case PKT_ENTITY_METADATA: break;              // never sent by client
        case PKT_SET_CHUNK_VISIBILITY: break;         // never sent by client
        case PKT_SET_BLOCK: break;                    // never sent by client
        case PKT_SET_SLOT: break;                     // never sent by client
        case PKT_FILL_CONTAINER: break;               // never sent by client

        case PKT_DISCONNECT:
            send_string16(data.disconnect.reason);
            break;

        default:
            break;
    }
}

packet_t read_packet(Packet *out) {

    packet_t type;

    int result = read(sock, &type, 1);

    if (result == 0) {
        strcpy(out->disconnect.reason, "Server shut down unexpectedly");
        return PKT_DISCONNECT;
    }

    if (result == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return PKT_EOB;
        } else {
            // an actual error occurred, but I don't really care
        }
    }

    // should ideally parse all packets, even if you ignore them
    switch (type) {

        case PKT_LOGIN:
            READ_I32(&out->login.int_val);
            read_string16(out->login.string);
            READ_I64(&out->login.long_val);
            READ_I8(&out->login.byte_val);
            break;

        case PKT_PRE_LOGIN:
            read_string16(out->pre_login.string);
            break;

        case PKT_SET_TIME:
            READ_I64(&out->set_time.time);
            break;

        case PKT_SET_SPAWN_POSITION:
            READ_I32(&out->set_spawn_position.x);
            READ_I32(&out->set_spawn_position.y);
            READ_I32(&out->set_spawn_position.z);
            break;
        
        case PKT_PLAYER_POSITION_AND_ROTATION:
            READ_F64(&out->player_position_and_rotation.x);
            READ_F64(&out->player_position_and_rotation.y);
            READ_F64(&out->player_position_and_rotation.camera_y);
            READ_F64(&out->player_position_and_rotation.z);
            READ_F32(&out->player_position_and_rotation.yaw);
            READ_F32(&out->player_position_and_rotation.pitch);
            READ_I8(&out->player_position_and_rotation.on_ground);
            break;

        case PKT_SPAWN_ITEM:
            READ_I32(&out->spawn_item.entity_id);
            READ_I16(&out->spawn_item.item_id);
            READ_I8(&out->spawn_item.item_amount);
            READ_I16(&out->spawn_item.item_metadata);
            READ_I32(&out->spawn_item.x);
            READ_I32(&out->spawn_item.y);
            READ_I32(&out->spawn_item.z);
            READ_I8(&out->spawn_item.yaw);
            READ_I8(&out->spawn_item.pitch);
            READ_I8(&out->spawn_item.roll);
            break;
        
        case PKT_SPAWN_MOB:
            READ_I32(&out->spawn_mob.entity_id);
            READ_I8(&out->spawn_mob.mob_type);
            READ_I32(&out->spawn_mob.x);
            READ_I32(&out->spawn_mob.y);
            READ_I32(&out->spawn_mob.z);
            READ_I8(&out->spawn_mob.yaw);
            READ_I8(&out->spawn_mob.pitch);
            read_entity_metadata();
            break;

        case PKT_ENTITY_VELOCITY:
            READ_I32(&out->entity_velocity.entity_id);
            READ_I16(&out->entity_velocity.x_vel);
            READ_I16(&out->entity_velocity.y_vel);
            READ_I16(&out->entity_velocity.z_vel);
            break;

        case PKT_DESPAWN_ENTITY:
            READ_I32(&out->despawn_entity.entity_id);
            break;

        case PKT_ENTITY_POSITION:
            READ_I32(&out->entity_position.entity_id);
            READ_I8(&out->entity_position.x);
            READ_I8(&out->entity_position.y);
            READ_I8(&out->entity_position.z);
            break;

        case PKT_ENTITY_POSITION_AND_ROTATION:
            READ_I32(&out->entity_position_and_rotation.entity_id);
            READ_I8(&out->entity_position_and_rotation.x);
            READ_I8(&out->entity_position_and_rotation.y);
            READ_I8(&out->entity_position_and_rotation.z);
            READ_I8(&out->entity_position_and_rotation.yaw);
            READ_I8(&out->entity_position_and_rotation.pitch);
            break;

        case PKT_TELEPORT_ENTITY:
            READ_I32(&out->teleport_entity.entity_id);
            READ_I32(&out->teleport_entity.x);
            READ_I32(&out->teleport_entity.y);
            READ_I32(&out->teleport_entity.z);
            READ_I8(&out->teleport_entity.yaw);
            READ_I8(&out->teleport_entity.pitch);
            break;

        case PKT_ENTITY_EVENT:
            READ_I32(&out->entity_event.entity_id);
            READ_I8(&out->entity_event.action);
            break;

        case PKT_ENTITY_METADATA:
            READ_I32(&out->entity_metadata.entity_id);
            read_entity_metadata();
            break;

        case PKT_SET_CHUNK_VISIBILITY:
            READ_I32(&out->set_chunk_visibility.x);
            READ_I32(&out->set_chunk_visibility.z);
            READ_I8(&out->set_chunk_visibility.load);
            break;

        case PKT_SET_BLOCK:
            READ_I32(&out->set_block.x);
            READ_I8(&out->set_block.y);
            READ_I32(&out->set_block.z);
            READ_I8(&out->set_block.type);
            READ_I8(&out->set_block.metadata);
            break;

        case PKT_SET_SLOT:
            READ_I8(&out->set_slot.window_id);
            READ_I16(&out->set_slot.slot);
            READ_I16(&out->set_slot.item_id);
            if (out->set_slot.item_id > 0) { // maybe > -1?
                READ_I8(&out->set_slot.item_amount);
                READ_I16(&out->set_slot.item_metadata);
            }
            break;

        case PKT_FILL_CONTAINER:
            READ_I8(&out->fill_container.window_id);
            READ_I16(&out->fill_container.payload_size);
            int16_t buf;
            for (int i = 0; i < out->fill_container.payload_size; i++) {
                READ_I16(&buf);
                if (buf != -1) {
                    READ_I8(&buf);
                    READ_I16(&buf);
                }
            }
            break;

        case PKT_DISCONNECT:
            read_string16(out->disconnect.reason);
            break;

        default:
            fprintf(stderr, "Received package 0x%02x which we don't know how to handle!\n", type);
            exit(1);
    }

    return type;
}

void initialize_conn() {

	// initialize server connection
	sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock < 0) {
		perror("Could not create socket");
        exit(1);
    }

	struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
		perror("Invalid address");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 && errno != EINPROGRESS) {
        perror("Could not connect to server");
        exit(1);
    }
}