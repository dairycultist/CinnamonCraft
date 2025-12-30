#include "header.h"

// all 3D objects use the same hardcoded shader for simplicity
static char *vertex =
"#version 150 core\n"
"uniform mat4 position_matrix;\n"
"uniform mat4 normal_matrix;\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec2 UV;\n"
"out vec3 normal_camera;\n"
"out vec2 frag_UV;\n"
"void main() {\n"
    "gl_Position = position_matrix * vec4(position.xyz, 1.0);\n" // get final position
    "normal_camera = (normal_matrix * vec4(normal, 1.0)).xyz;\n" // get final normal
    "frag_UV = UV;\n" // pass along UV
"}";

static char *fragment =
"#version 150 core\n"
"uniform sampler2D tex;\n"
"in vec3 normal_camera;\n"
"in vec2 frag_UV;\n"
"out vec4 outColor;\n"
"void main() {\n"
	"float c = dot(normal_camera, vec3(0.7, 0.7, 0)) * 0.3 + 0.7;\n"
	"outColor = texture(tex, frag_UV) * vec4(c, c, c, 1.0);\n"
"}";

static GLuint shader_program;
static GLfloat proj_matrix[4][4] = {0};

Texture *load_texture(const char *path) {

	Texture *texture = malloc(sizeof(Texture));

	texture->surface = IMG_Load(path);

	// flip texture vertically to match OpenGL spec
	unsigned char *pixels = (unsigned char *) texture->surface->pixels;
	int pixelBytes = texture->surface->format->BytesPerPixel;

	for (int y = 0; y < texture->surface->h / 2; y++) {

		for (int x = 0; x < texture->surface->w; x++) {

			int top = x + y * texture->surface->h;
			int bottom = x + (texture->surface->w * texture->surface->h - (y + 1) * texture->surface->h);

			for (int i=0; i<pixelBytes; i++) {

				unsigned char hold = pixels[top * pixelBytes + i];
				pixels[top * pixelBytes + i] = pixels[bottom * pixelBytes + i];
				pixels[bottom * pixelBytes + i] = hold;
			}
		}
	}

	return texture;
}

// returns NULL on error
Mesh *create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, const Texture *texture) {

	// make vertex array
	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	// make vertex buffer (stored by vertex_array)
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);								// make it the active buffer
	glBufferData(GL_ARRAY_BUFFER, mesh_bytecount, mesh_data, GL_STATIC_DRAW);	// copy vertex data into the active buffer

	// link active vertex data and shader attributes
	GLint pos_attrib = glGetAttribLocation(shader_program, "position");
	glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
	glEnableVertexAttribArray(pos_attrib); // requires a VAO to be bound

	GLint normal_attrib = glGetAttribLocation(shader_program, "normal");
	glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid *) (sizeof(float) * 3));
	glEnableVertexAttribArray(normal_attrib);

	GLint uv_attrib = glGetAttribLocation(shader_program, "UV");
	glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid *) (sizeof(float) * 6));
	glEnableVertexAttribArray(uv_attrib);

	// debind vertex array
	glBindVertexArray(0);

	// create texture object
	GLuint texture_object;
	glGenTextures(1, &texture_object);

	// bind texture (to active texture 2D)
	glBindTexture(GL_TEXTURE_2D, texture_object);

	// wrap repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// filter linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// write texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->surface->w, texture->surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->surface->pixels);

	// create final mesh object to return
	Mesh *mesh = malloc(sizeof(Mesh));
	mesh->vertex_array = vertex_array;
	mesh->vertex_count = mesh_vertcount;
	mesh->texture = texture_object;

	return mesh;
}

Mesh *create_mesh_from_obj(const char *obj_path, const Texture *texture) {

	// read obj file
	FILE *file = fopen(obj_path, "r");

	if (file == NULL) {
		printf("File \"%s\" does not exist!\n", obj_path);
		exit(1);
	}

	char line[1024];

	EZArray position_data  = {0};
	EZArray normal_data    = {0};
	EZArray texture_data   = {0};
	EZArray composite_data = {0}; // stores combined vertex position, normal, and texture data

	int vertex_count;

	while (fgets(line, 1024, file)) {

		char prefix[8];

		sscanf(line, "%s", prefix);

		if (!strcmp(prefix, "v")) {

			float v[3];
			
			sscanf(line, "v %f %f %f", &v[0], &v[1], &v[2]);

			// obj vertices have reverse xz
			v[0] = -v[0];
			v[2] = -v[2];

			append_ezarray(&position_data, v, sizeof(float) * 3);
		}

		else if (!strcmp(prefix, "vn")) {

			float n[3];
			
			sscanf(line, "vn %f %f %f", &n[0], &n[1], &n[2]);

			// obj vertices have reverse xz
			n[0] = -n[0];
			n[2] = -n[2];

			append_ezarray(&normal_data, n, sizeof(float) * 3);
		}

		else if (!strcmp(prefix, "vt")) {

			float n[2];
			
			sscanf(line, "vt %f %f", &n[0], &n[1]);

			append_ezarray(&texture_data, n, sizeof(float) * 2);
		}

		else if (!strcmp(prefix, "f")) {

			// only works with tris right now (no quads or ngons)

			unsigned int p[3]; // vertex position indices
			unsigned int t[3]; // vertex texture coordinate indices
			unsigned int n[3]; // vertex normal indices
			
			sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
				&p[0], &t[0], &n[0],
				&p[1], &t[1], &n[1],
				&p[2], &t[2], &n[2]);

			// convert vertex indices to vertex positions (indices start at 1 for some reason)
			append_ezarray(&composite_data, position_data.data + ((p[0] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, normal_data.data +   ((n[0] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, texture_data.data +  ((t[0] - 1) * sizeof(float) * 2), sizeof(float) * 2);

			append_ezarray(&composite_data, position_data.data + ((p[1] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, normal_data.data +   ((n[1] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, texture_data.data +  ((t[1] - 1) * sizeof(float) * 2), sizeof(float) * 2);

			append_ezarray(&composite_data, position_data.data + ((p[2] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, normal_data.data +   ((n[2] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, texture_data.data +  ((t[2] - 1) * sizeof(float) * 2), sizeof(float) * 2);
			
			vertex_count += 3;
		}
	}

	fclose(file);

	// create mesh
	return create_mesh(composite_data.data, composite_data.bytecount, vertex_count, texture);
}

static void mat4_mult(const GLfloat b[4][4], const GLfloat a[4][4], GLfloat out[4][4]) {

	// a (rightmost) is applied first, then b

	GLfloat matrix[4][4] = {
		{
			a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0],
			a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1],
			a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2],
			a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3],
		},
		{
			a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0],
			a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1],
			a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2],
			a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3],
		},
		{
			a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0],
			a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1],
			a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2],
			a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3],
		},
		{
			a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0],
			a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1],
			a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2],
			a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3],
		},
	};

	int x, y;

	for (x = 0; x < 4; x++) {
		for (y = 0; y < 4; y++) {
			out[x][y] = matrix[x][y];
		}
	}
}

static void generate_rotation_matrices(GLfloat pitch_matrix[4][4], float pitch, GLfloat yaw_matrix[4][4], float yaw) {

	pitch_matrix[0][0] = 1;
	pitch_matrix[0][1] = 0;
	pitch_matrix[0][2] = 0;
	pitch_matrix[0][3] = 0;

	pitch_matrix[1][0] = 0;
	pitch_matrix[1][1] = cos(pitch);
	pitch_matrix[1][2] = -sin(pitch);
	pitch_matrix[1][3] = 0;

	pitch_matrix[2][0] = 0;
	pitch_matrix[2][1] = sin(pitch);
	pitch_matrix[2][2] = cos(pitch);
	pitch_matrix[2][3] = 0;

	pitch_matrix[3][0] = 0;
	pitch_matrix[3][1] = 0;
	pitch_matrix[3][2] = 0;
	pitch_matrix[3][3] = 1;

	yaw_matrix[0][0] = cos(yaw);
	yaw_matrix[0][1] = 0;
	yaw_matrix[0][2] = sin(yaw);
	yaw_matrix[0][3] = 0;

	yaw_matrix[1][0] = 0;
	yaw_matrix[1][1] = 1;
	yaw_matrix[1][2] = 0;
	yaw_matrix[1][3] = 0;

	yaw_matrix[2][0] = -sin(yaw);
	yaw_matrix[2][1] = 0;
	yaw_matrix[2][2] = cos(yaw);
	yaw_matrix[2][3] = 0;

	yaw_matrix[3][0] = 0;
	yaw_matrix[3][1] = 0;
	yaw_matrix[3][2] = 0;
	yaw_matrix[3][3] = 1;
}

void draw_mesh(const Transform *camera, const Transform *transform, const Mesh *mesh) {

	// shared buffers
	GLfloat pitch_matrix[4][4];
	GLfloat yaw_matrix[4][4];

	GLfloat position_matrix[4][4];

	// bind the mesh's vertex mesh and texture
	glBindVertexArray(mesh->vertex_array);
	glBindTexture(GL_TEXTURE_2D, mesh->texture);

	// model matrix (converts from model space to world space)
	generate_rotation_matrices(
		pitch_matrix, transform->pitch,
		yaw_matrix, transform->yaw
	);

	GLfloat model_matrix[4][4];

	mat4_mult(yaw_matrix, pitch_matrix, model_matrix); // rotation

	model_matrix[3][0] = transform->x; // translation
	model_matrix[3][1] = transform->y;
	model_matrix[3][2] = transform->z;

	// view matrix (converts from world space to view space, aka accounts for camera transformations)
	// must apply translations before rotations this time, unlike model matrix!
	generate_rotation_matrices(
		pitch_matrix, -camera->pitch,
		yaw_matrix, -camera->yaw
	);

	GLfloat view_matrix[4][4] = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{-camera->x, -camera->y, -camera->z, 1}
	};

	mat4_mult(yaw_matrix, view_matrix, view_matrix);
	mat4_mult(pitch_matrix, view_matrix, view_matrix);

	// final position matrix (proj_matrix * view_matrix * model_matrix)
	mat4_mult(proj_matrix, view_matrix, position_matrix);
	mat4_mult(position_matrix, model_matrix, position_matrix);

	// normal matrix (applied to normals to account for mesh rotation)
	GLfloat normal_matrix[4][4];

	generate_rotation_matrices(
		pitch_matrix, -transform->pitch,
		yaw_matrix, -transform->yaw
	);

	mat4_mult(yaw_matrix, pitch_matrix, normal_matrix);

	// load the shader program and the uniforms we just calculated
	glUseProgram(shader_program);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "position_matrix"), 1, GL_FALSE, &position_matrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "normal_matrix"), 1, GL_FALSE, &normal_matrix[0][0]);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
}

void initialize_shader() {

	// create shader program
	shader_program = glCreateProgram();

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char *const *) &vertex, NULL);
	glCompileShader(vertex_shader);
	glAttachShader(shader_program, vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char *const *) &fragment, NULL);
	glCompileShader(fragment_shader);
	glAttachShader(shader_program, fragment_shader);

	// apply changes to shader program (not gonna call "glUseProgram" yet bc not drawing)
	glLinkProgram(shader_program);
}

void initialize_perspective(const float aspectRatio) {

	#define DEG2RAD (M_PI / 180)

	// perspective projection matrix (converts from view space to clip space)
	const float fovY = 90;
	const float front = 0.01; // near plane
	const float back = 100;   // far plane

	float tangent = tan(fovY / 2 * DEG2RAD); // tangent of half fovY
	float top = front * tangent;             // half height of near plane
	float right = top * aspectRatio;         // half width of near plane

	proj_matrix[0][0] = front / right;
	proj_matrix[1][1] = front / top;
	proj_matrix[2][2] = -(back + front) / (back - front);
	proj_matrix[2][3] = -1.0;
	proj_matrix[3][2] = -(2.0 * back * front) / (back - front);
}