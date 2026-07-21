#ifndef CONN_H
#define CONN_H

#include <stdint.h>

typedef unsigned char packet_t;

// returned by read_packet when there are no more packets in the buffer
#define PKT_EOB 0xFE

// https://pixelbrush.dev/beta-wiki/networking/packets/001-login
#define PKT_LOGIN 0x01
typedef struct {

    int32_t int_val;
    char string[256];
    int64_t long_val;
    int8_t byte_val;

} Login;

// https://pixelbrush.dev/beta-wiki/networking/packets/002-pre-login
#define PKT_PRE_LOGIN 0x02
typedef struct {

    char string[256];

} PreLogin;

// https://pixelbrush.dev/beta-wiki/networking/packets/004-set-time
#define PKT_SET_TIME 0x04
typedef struct {

    int64_t time;

} SetTime;

// https://pixelbrush.dev/beta-wiki/networking/packets/006-set-spawn-position
#define PKT_SET_SPAWN_POSITION 0x06
typedef struct {

    int32_t x, y, z;

} SetSpawnPosition;

// https://pixelbrush.dev/beta-wiki/networking/packets/255-disconnect
#define PKT_DISCONNECT 0xFF
typedef struct {

    char reason[256];

} Disconnect;

typedef union {

    Login login;
    PreLogin pre_login;
    SetTime set_time;
    SetSpawnPosition set_spawn_position;
    Disconnect disconnect;

} Packet;

void send_packet(packet_t type, Packet packet);
packet_t read_packet(Packet *out);
void initialize_conn();

#endif