#ifndef ENTITIES_H
#define ENTITIES_H

#include "io.h"

// an AABB is a rectangular prism with a square base centered on x,y,z (extruding up)
typedef struct {

    float x, y, z;
    float r, h; // radius, height

} AABB;

typedef struct {

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

#endif