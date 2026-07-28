#ifndef ENTITIES_H
#define ENTITIES_H

// abstracts away having to deal with memory-managing a list of entities, iterating over entities for rendering/collision...

#include <stdint.h>

#include "io.h"

// an AABB is a rectangular prism with a square base centered on x,y,z (extruding up)
typedef struct {

    float x, y, z;
    float r, h; // radius, height

} AABB;

typedef struct {

    int32_t entity_id; // we can (somewhat) safely assume valid IDs won't be negative, so -1 represents No Entity

    // TODO function pointers for like, entity behaviour
    // TODO stuff like health and move speed and whatever

    // a transform is generated on-demand during rendering from the below
    AABB aabb;
    float rotation;

    Mesh meshes[1]; // TODO vertex animation

} Entity;

void initialize_entities();
void entities_process_tick();
void draw_entities(const Transform *camera);

// both these functions can return NULL, so you should probably handle that
Entity *create_entity_with_id(int32_t entity_id);
Entity *get_entity_by_id(int32_t entity_id);

#endif