#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// this script is a utility, since must include data object files (images, etc.) in program by formatting the object as C sourcecode (most portable/least effort)

typedef struct {

	char data[65536 * 64];
	int bytecount;

} EZArray;

void append_ezarray(EZArray *array, void *data, int data_length) {

	memcpy(&array->data[array->bytecount], data, data_length);
	array->bytecount += data_length;
}

void print_ppm_to_binary(const char *ppm_path) {

	// by default, OpenGL reads texture data with a 4-byte row alignment: https://stackoverflow.com/questions/72177553/why-is-gl-unpack-alignment-default-4
	// it's more efficient, but means this function cannot properly read images whose dimensions aren't a multiple of 4 (fix is simple tho)

	int width, height;

	FILE *file = fopen(ppm_path, "r");

	// read header
	{
		char line[1024];

		fgets(line, 1024, file); // not gonna verify header because I'm lazy and just wanna get this working right now
		fgets(line, 1024, file);
		sscanf(line, "%d %d", &width, &height);
		fgets(line, 1024, file);
	}

	unsigned char *pixels = malloc(width * height * 3);

	// ppms store pixels starting from the top left, but opengl wants them starting from the bottom left, so you need to flip the "layers"
	int i;

	for (i = (width - 1) * height * 3; i >= 0; i -= width * 3) {
		fread(pixels + i, 3, width, file);
	}

	fclose(file);

	printf("unsigned char tex[] = {");
	for (i = 0; i < width * height * 3; i++) {
		printf("%#x,", *(pixels + i));
	}
	printf("};\n");
}

// void print_obj_to_binary(const char *obj_path) {

// 	// read obj file
// 	FILE *file = fopen(obj_path, "r");

// 	if (file == NULL) {
// 		return NULL;
// 	}

// 	char line[1024];

// 	EZArray position_data  = {0};
// 	EZArray normal_data    = {0};
// 	EZArray texture_data   = {0};
// 	EZArray composite_data = {0}; // stores combined vertex position, normal, and texture data

// 	int vertex_count;

// 	while (fgets(line, 1024, file)) {

// 		char prefix[8];

// 		sscanf(line, "%s", prefix);

// 		if (!strcmp(prefix, "v")) {

// 			float v[3];
			
// 			sscanf(line, "v %f %f %f", &v[0], &v[1], &v[2]);

// 			// obj vertices have reverse xz
// 			v[0] = -v[0];
// 			v[2] = -v[2];

// 			append_ezarray(&position_data, v, sizeof(float) * 3);
// 		}

// 		else if (!strcmp(prefix, "vn")) {

// 			float n[3];
			
// 			sscanf(line, "vn %f %f %f", &n[0], &n[1], &n[2]);

// 			append_ezarray(&normal_data, n, sizeof(float) * 3);
// 		}

// 		else if (!strcmp(prefix, "vt")) {

// 			float n[2];
			
// 			sscanf(line, "vt %f %f", &n[0], &n[1]);

// 			append_ezarray(&texture_data, n, sizeof(float) * 2);
// 		}

// 		else if (!strcmp(prefix, "f")) {

// 			// only works with tris right now (no quads or ngons)

// 			GLuint p[3]; // vertex position indices
// 			GLuint t[3]; // vertex texture coordinate indices
// 			GLuint n[3]; // vertex normal indices
			
// 			sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
// 				&p[0], &t[0], &n[0],
// 				&p[1], &t[1], &n[1],
// 				&p[2], &t[2], &n[2]);

// 			// convert vertex indices to vertex positions (indices start at 1 for some reason)
// 			append_ezarray(&composite_data, position_data.data + ((p[0] - 1) * sizeof(float) * 3), sizeof(float) * 3);
// 			append_ezarray(&composite_data, normal_data.data +   ((n[0] - 1) * sizeof(float) * 3), sizeof(float) * 3);
// 			append_ezarray(&composite_data, texture_data.data +  ((t[0] - 1) * sizeof(float) * 2), sizeof(float) * 2);

// 			append_ezarray(&composite_data, position_data.data + ((p[1] - 1) * sizeof(float) * 3), sizeof(float) * 3);
// 			append_ezarray(&composite_data, normal_data.data +   ((n[1] - 1) * sizeof(float) * 3), sizeof(float) * 3);
// 			append_ezarray(&composite_data, texture_data.data +  ((t[1] - 1) * sizeof(float) * 2), sizeof(float) * 2);

// 			append_ezarray(&composite_data, position_data.data + ((p[2] - 1) * sizeof(float) * 3), sizeof(float) * 3);
// 			append_ezarray(&composite_data, normal_data.data +   ((n[2] - 1) * sizeof(float) * 3), sizeof(float) * 3);
// 			append_ezarray(&composite_data, texture_data.data +  ((t[2] - 1) * sizeof(float) * 2), sizeof(float) * 2);
			
// 			vertex_count += 3;
// 		}
// 	}

// 	fclose(file);

// 	// make vertex array
// 	GLuint vertex_array;
// 	glGenVertexArrays(1, &vertex_array);
// 	glBindVertexArray(vertex_array);

// 	// make vertex buffer (stored by vertex_array)
// 	GLuint vertexBuffer;
// 	glGenBuffers(1, &vertexBuffer);
// 	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);													// make it the active buffer
// 	glBufferData(GL_ARRAY_BUFFER, composite_data.bytecount, composite_data.data, GL_STATIC_DRAW);	// copy vertex data into the active buffer

// 	// link active vertex data and shader attributes
// 	GLint pos_attrib = glGetAttribLocation(shader_program, "position");
// 	glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
// 	glEnableVertexAttribArray(pos_attrib); // requires a VAO to be bound

// 	GLint normal_attrib = glGetAttribLocation(shader_program, "normal");
// 	glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid *) (sizeof(float) * 3));
// 	glEnableVertexAttribArray(normal_attrib);

// 	GLint uv_attrib = glGetAttribLocation(shader_program, "UV");
// 	glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid *) (sizeof(float) * 6));
// 	glEnableVertexAttribArray(uv_attrib);

// 	// debind vertex array
// 	glBindVertexArray(0);

// 	// create texture object
// 	GLuint texture;
// 	glGenTextures(1, &texture);

// 	// bind texture (to active texture 2D)
// 	glBindTexture(GL_TEXTURE_2D, texture);

// 	// wrap repeat
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

// 	// filter linear
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

// 	// write texture data
// 	load_ppm(GL_TEXTURE_2D, ppm_path);

// 	// create final mesh object to return
// 	Mesh *mesh = malloc(sizeof(Mesh));
// 	mesh->transform.x 		= 0.0f;
// 	mesh->transform.y 		= 0.0f;
// 	mesh->transform.z 		= 0.0f;
// 	mesh->transform.pitch 	= 0.0f;
// 	mesh->transform.yaw 	= 0.0f;
// 	mesh->vertex_array = vertex_array;
// 	mesh->vertex_count = vertex_count;
// 	mesh->texture = texture;

// 	return mesh;
// }

int main() {

	// miku_mesh, miku_tex
	// block_mesh, block_tex

	// mesh1 = import_mesh("res/miku.obj", "res/miku.ppm");
	// mesh2 = import_mesh("res/block.obj", "res/block.ppm");

	print_ppm_to_binary("block.ppm");

	return 0;
}