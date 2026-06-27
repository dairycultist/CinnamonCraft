#ifndef ENTITY_H
#define ENTITY_H

// an AABB is a rectangular prism with a square base centered on x,y,z (extruding up)
typedef struct {

    float x, y, z;
    float wl, h;

} AABB;

// typedef struct {

//     // TODO stuff like health and move speed and whatever

//     // TODO handle entities (aka test miku) in main, passing them to player for rendering

//     AABB aabb;
//     // intentionally no transform; as for mesh, might do an array of meshes to allow for animation

// } Entity;

#endif