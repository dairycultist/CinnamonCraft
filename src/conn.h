#ifndef CONN_H
#define CONN_H

#include <stdint.h>

typedef float  float32_t;
typedef double float64_t;

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

// https://pixelbrush.dev/beta-wiki/networking/packets/005-set-equipment
#define PKT_SET_EQUIPMENT 0x05
typedef struct {

    int32_t entity_id;
    int16_t slot;
    int16_t item, item_metadata;

} SetEquipment;

// https://pixelbrush.dev/beta-wiki/networking/packets/006-set-spawn-position
#define PKT_SET_SPAWN_POSITION 0x06
typedef struct {

    int32_t x, y, z;

} SetSpawnPosition;

// https://pixelbrush.dev/beta-wiki/networking/packets/013-player-position-and-rotation
#define PKT_PLAYER_POSITION_AND_ROTATION 0x0D
typedef struct {

    float64_t x, y, z;
    float64_t camera_y;
    float32_t yaw, pitch;
    int8_t on_ground;

} PlayerPositionAndRotation;

// https://pixelbrush.dev/beta-wiki/networking/packets/020-spawn-player
#define PKT_SPAWN_PLAYER 0x14
typedef struct {

    int32_t entity_id;
    char username[17]; // max of 16 characters!!
    int32_t x, y, z;
    int8_t yaw, pitch;
    int16_t held_item;

} SpawnPlayer;

// https://pixelbrush.dev/beta-wiki/networking/packets/021-spawn-item
#define PKT_SPAWN_ITEM 0x15
typedef struct {

    int32_t entity_id;
    int16_t item_id;
    int8_t item_amount;
    int16_t item_metadata;
    int32_t x, y, z;
    int8_t yaw, pitch, roll;

} SpawnItem;

// https://pixelbrush.dev/beta-wiki/networking/packets/024-spawn-mob
#define PKT_SPAWN_MOB 0x18
typedef struct {

    int32_t entity_id;
    int8_t mob_type;
    int32_t x, y, z;
    int8_t yaw, pitch;
    // TODO entity metadata

} SpawnMob;

// https://pixelbrush.dev/beta-wiki/networking/packets/028-entity-velocity
#define PKT_ENTITY_VELOCITY 0x1C
typedef struct {

    int32_t entity_id;
    int16_t x_vel, y_vel, z_vel;

} EntityVelocity;

// https://pixelbrush.dev/beta-wiki/networking/packets/029-despawn-entity
#define PKT_DESPAWN_ENTITY 0x1D
typedef struct {

    int32_t entity_id;

} DespawnEntity;

// https://pixelbrush.dev/beta-wiki/networking/packets/031-entity-position
#define PKT_ENTITY_POSITION 0x1F
typedef struct {

    int32_t entity_id;
    int8_t x, y, z;

} EntityPosition;

// https://pixelbrush.dev/beta-wiki/networking/packets/033-entity-position-and-rotation
#define PKT_ENTITY_POSITION_AND_ROTATION 0x21
typedef struct {

    int32_t entity_id;
    int8_t x, y, z;
    int8_t yaw, pitch;

} EntityPositionAndRotation;

// https://pixelbrush.dev/beta-wiki/networking/packets/034-teleport-entity
#define PKT_TELEPORT_ENTITY 0x22
typedef struct {

    int32_t entity_id;
    int32_t x, y, z;
    int8_t yaw, pitch;

} TeleportEntity;

// https://pixelbrush.dev/beta-wiki/networking/packets/038-entity-event
#define PKT_ENTITY_EVENT 0x26
typedef struct {

    int32_t entity_id;
    int8_t action;

} EntityEvent;

// https://pixelbrush.dev/beta-wiki/networking/packets/040-entity-metadata
#define PKT_ENTITY_METADATA 0x28
typedef struct {

    int32_t entity_id;
    // TODO metadata

} EntityMetadata;

// https://pixelbrush.dev/beta-wiki/networking/packets/050-set-chunk-visibility
#define PKT_SET_CHUNK_VISIBILITY 0x32
typedef struct {

    int32_t x, z;
    int8_t load;

} SetChunkVisibility;

// https://pixelbrush.dev/beta-wiki/networking/packets/051-chunk
#define PKT_CHUNK 0x33
typedef struct {

    int32_t x;
    int16_t y;
    int32_t z;
    int8_t w, h, l; // technically w-1, h-1, l-1

    // compressed_size and compressed_data are what's sent over the network, but we're abstracting compression away
    int8_t *blocks;
    int8_t *block_datas;
    int8_t *block_lights;
    int8_t *sky_lights;

} Chunk;

// https://pixelbrush.dev/beta-wiki/networking/packets/052-set-multiple-blocks
#define PKT_SET_MULTIPLE_BLOCKS 0x34
typedef struct {

    int32_t x, z;
    int16_t block_count;
    int16_t *block_positions;
    int8_t *blocks;
    int8_t *block_metadatas;

} SetMultipleBlocks;

// https://pixelbrush.dev/beta-wiki/networking/packets/053-set-block
#define PKT_SET_BLOCK 0x35
typedef struct {

    int32_t x;
    int8_t y;
    int32_t z;
    int8_t type;
    int8_t metadata;

} SetBlock;

// https://pixelbrush.dev/beta-wiki/networking/packets/070-game-event
#define PKT_GAME_EVENT 0x46
typedef struct {

    int8_t type;

} GameEvent;

// https://pixelbrush.dev/beta-wiki/networking/packets/103-set-slot
#define PKT_SET_SLOT 0x67
typedef struct {

    int8_t window_id;
    int16_t slot;
    int16_t item_id;
    int8_t item_amount;
    int16_t item_metadata;

} SetSlot;

// https://pixelbrush.dev/beta-wiki/networking/packets/104-fill-container
#define PKT_FILL_CONTAINER 0x68
typedef struct {

    int8_t window_id;
    int16_t payload_size;
    // TODO payload

} FillContainer;

// https://pixelbrush.dev/beta-wiki/networking/packets/255-disconnect
#define PKT_DISCONNECT 0xFF
typedef struct {

    char reason[256];

} Disconnect;

typedef union {

    Login login;
    PreLogin pre_login;
    SetTime set_time;
    SetEquipment set_equipment;
    SetSpawnPosition set_spawn_position;
    PlayerPositionAndRotation player_position_and_rotation;
    SpawnPlayer spawn_player;
    SpawnItem spawn_item;
    SpawnMob spawn_mob;
    EntityVelocity entity_velocity;
    DespawnEntity despawn_entity;
    EntityPosition entity_position;
    EntityPositionAndRotation entity_position_and_rotation;
    TeleportEntity teleport_entity;
    EntityEvent entity_event;
    EntityMetadata entity_metadata;
    SetChunkVisibility set_chunk_visibility;
    Chunk chunk;
    SetMultipleBlocks set_multiple_blocks;
    SetBlock set_block;
    GameEvent game_event;
    SetSlot set_slot;
    FillContainer fill_container;
    Disconnect disconnect;

} Packet;

void send_packet(packet_t type, Packet packet);
packet_t read_packet(Packet *out);
void initialize_conn();

#endif