#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "main.h"
#include "conn.h"

// What to do to get basic login + movement + chunk loading
// - parse all packets, even if you ignore them
// - implement these packets https://pixelbrush.dev/beta-wiki/
//     - pre login
//     - login
//     - player pos and rot
//     - set chunk vis
//     - chunk

static int sock; // close(sock);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htonll(x) __builtin_bswap64(x)
#else
#define htonll(x) (x)
#endif

#define SEND_I8(value)  { int8_t  temp = value;         send(sock, &temp, 1, 0); }
#define SEND_I16(value) { int16_t temp = htons(value);  send(sock, &temp, 2, 0); }
#define SEND_I32(value) { int32_t temp = htonl(value);  send(sock, &temp, 4, 0); }
#define SEND_I64(value) { int64_t temp = htonll(value); send(sock, &temp, 8, 0); }

static void send_string16(const char *string) {

    int16_t len = strlen(string);

    SEND_I16(len);

    for (int i = 0; i < len; i++) {

        SEND_I8(0x00);
        SEND_I8(string[i]);
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

        default:
            break;
    }
}

packet_t read_packet(Packet *out) {
	
	// char buffer[1024] = {0};
	// read(sock, buffer, 1024);
    // ntohs()

    return 0;
}

void initialize_conn() {

	// initialize server connection
	sock = socket(AF_INET, SOCK_STREAM, 0);
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

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Could not connect to server");
        exit(1);
    }

    // pre-login packet
    send_packet(PKT_PRE_LOGIN, (Packet) { .pre_login = { "Steve" } });

    // login packet
    send_packet(PKT_LOGIN, (Packet) { .login = { 14, "Steve" } });
}