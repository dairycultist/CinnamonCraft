#ifndef CONN_H
#define CONN_H

#include <stdint.h>

typedef unsigned char packet_t;

// returned by read_packet when there are no more packets in the queue (also lets us ignore packets that we haven't implemented yet)
#define PKT_EOQ 0xFE

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

typedef union {

    Login login;
    PreLogin pre_login;

} Packet;

void send_packet(packet_t type, Packet packet);
packet_t read_packet(Packet *out);
void initialize_conn();

#endif