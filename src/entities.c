#include <stddef.h>

#include "io.h"
#include "entities.h"

#define MAX_ENTITIES 64

static Entity entities[MAX_ENTITIES] = { 0 };

void initialize_entities() {

    // initialize all entities with ID=-1 so they're "empty"
    for (int i = 0; i < MAX_ENTITIES; i++)
        entities[i].entity_id = -1;
}

void entities_process_tick() {}

void draw_entities(const Transform *camera) {

    for (int i = 0; i < MAX_ENTITIES; i++) {

        if (entities[i].entity_id == -1)
            continue;

		Transform transform = {
			entities[i].aabb.x,
			entities[i].aabb.y,
			entities[i].aabb.z,
			0.0,
			entities[i].rotation
		};

		draw_mesh(camera, &transform, entities[i].meshes[0]);
	}
}

Entity *create_entity_with_id(int32_t entity_id) {

    // prevent duplicates
    Entity *already_exists = get_entity_by_id(entity_id);

    if (already_exists)
        return already_exists;

    // otherwise, add it in an empty slot
    for (int i = 0; i < MAX_ENTITIES; i++) {
        
        if (entities[i].entity_id == -1) {

            entities[i].entity_id = entity_id;
            return &entities[i];
        }
    }

    // return NULL if there are no empty slots
    return NULL;
}

Entity *get_entity_by_id(int32_t entity_id) {

    for (int i = 0; i < MAX_ENTITIES; i++)
        if (entities[i].entity_id == entity_id)
            return &entities[i];
    
    return NULL;
}