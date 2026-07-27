#include "io.h"
#include "entities.h"

static Entity entities[16] = { 0 };
static int entity_count = 0;

void initialize_entities() {

    // create test entity
    entities[0].meshes[0] = create_mesh_from_obj("res/biped.obj", load_texture("res/biped_jenny.png"));
    entities[0].aabb.x = 8;
    entities[0].aabb.y = 30;
    entities[0].aabb.z = 8;
    entity_count++;
}

void entities_process_tick() {

    // update test entity
    entities[0].rotation += 0.01;
}

void draw_entities(const Transform *camera) {

    for (int i = 0; i < entity_count; i++) {

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