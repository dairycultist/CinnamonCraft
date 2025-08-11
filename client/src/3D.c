#include "../../util.c"

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
    "gl_Position = position_matrix * vec4(position.xy, -position.z, 1.0);\n" // get final position
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
	"float c = dot(normal_camera, vec3(0.7, 0.7, 0)) * 0.5 + 0.5;\n"
	"outColor = texture(tex, frag_UV) * vec4(c, c, c, 1.0);\n"
"}";

static GLuint shader_program;
static GLfloat proj_matrix[4][4] = {0};

typedef struct {

	float x;
	float y;
	float z;
	float pitch;
	// no one needs roll
	float yaw;

} Transform;

typedef struct {

	Transform transform;

	GLuint vertex_array; // "VAO"
	uint vertex_count;
	GLuint texture;

} Model;

typedef struct {

	Model model;
	unsigned char blocks[16][16][16]; // array of bytes representing blockstates

} ChunkModel;

static unsigned char block_types[256 * 4] = { // 4 bytes: block model (0:empty,1:cube) | top texture index | side texture index | bottom texture index
	0, 0, 0, 0,
	1, 98, 243, 242
};

#define BLOCK_MESH_EMPTY 0
#define BLOCK_MESH_CUBE 1

#define BLOCK_GET_MESH_TYPE(block) (block_types[block * 4])
#define BLOCK_GET_TOP(block) (block_types[block * 4 + 1])
#define BLOCK_GET_SIDE(block) (block_types[block * 4 + 2])
#define BLOCK_GET_BOTTOM(block) (block_types[block * 4 + 3])
#define BLOCK_HAS_PASSTHROUGH(block) (BLOCK_GET_MESH_TYPE(block) == 0) // "passthrough" means adjacent blocks aren't able to cull the faces that touch it

#define GET_SPRITEMAP_UV(index, u_sml, v_sml, u_big, v_big) u_sml = ((index) % 16) / 16.; v_sml = ((index) / 16) / 16.; u_big = (((index) + 1) % 16) / 16.; v_big = ((index) / 16 + 1) / 16.;

// returns NULL on error
Model *create_model(const unsigned char *mesh, const int mesh_bytecount, const int mesh_vertcount, const unsigned char *tex, const int tex_width, const int tex_height) {

	// make vertex array
	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	// make vertex buffer (stored by vertex_array)
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);							// make it the active buffer
	glBufferData(GL_ARRAY_BUFFER, mesh_bytecount, mesh, GL_STATIC_DRAW);	// copy vertex data into the active buffer

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
	GLuint texture;
	glGenTextures(1, &texture);

	// bind texture (to active texture 2D)
	glBindTexture(GL_TEXTURE_2D, texture);

	// wrap repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// filter linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// write texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);

	// create final model object to return
	Model *model = malloc(sizeof(Model));
	model->transform.x 		= 0.0f;
	model->transform.y 		= 0.0f;
	model->transform.z 		= 0.0f;
	model->transform.pitch 	= 0.0f;
	model->transform.yaw 	= 0.0f;
	model->vertex_array = vertex_array;
	model->vertex_count = mesh_vertcount;
	model->texture = texture;

	return model;
}

void append_block_to_mesh(EZArray *mesh, int *vertex_count, const unsigned char blocks[16][16][16], int block_x, int block_y, int block_z) {

	// this function determines what mesh/UV a block gets (including considering its environment)

	unsigned char block = blocks[block_x][block_y][block_z];

	if (BLOCK_GET_MESH_TYPE(block) == BLOCK_MESH_EMPTY) { return; }

	float u_sml, v_sml, u_big, v_big;

	// get UV for sides
	GET_SPRITEMAP_UV(BLOCK_GET_SIDE(block), u_sml, v_sml, u_big, v_big)

	// -x face
	if (block_x == 0 || BLOCK_HAS_PASSTHROUGH(blocks[block_x-1][block_y][block_z])) {

		float full_block_data[] = {
			block_x, block_y, block_z,			-1, 0, 0,	u_big, v_sml,
			block_x, block_y, block_z + 1,		-1, 0, 0,	u_sml, v_sml,
			block_x, block_y + 1, block_z,		-1, 0, 0,	u_big, v_big,
			block_x, block_y + 1, block_z + 1,	-1, 0, 0,	u_sml, v_big,
			block_x, block_y + 1, block_z,		-1, 0, 0,	u_big, v_big,
			block_x, block_y, block_z + 1,		-1, 0, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +x face
	if (block_x == 15 || BLOCK_HAS_PASSTHROUGH(blocks[block_x+1][block_y][block_z])) {

		float full_block_data[] = {
			block_x + 1, block_y, block_z,			1, 0, 0,	u_sml, v_sml,
			block_x + 1, block_y + 1, block_z,		1, 0, 0,	u_sml, v_big,
			block_x + 1, block_y, block_z + 1,		1, 0, 0,	u_big, v_sml,
			block_x + 1, block_y + 1, block_z + 1,	1, 0, 0,	u_big, v_big,
			block_x + 1, block_y, block_z + 1,		1, 0, 0,	u_big, v_sml,
			block_x + 1, block_y + 1, block_z,		1, 0, 0,	u_sml, v_big,
		};

		append_ezarray(mesh, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -z face
	if (block_z == 0 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y][block_z-1])) {

		float full_block_data[] = {
			block_x, block_y, block_z,			0, 0, -1,	u_sml, v_sml,
			block_x, block_y + 1, block_z,		0, 0, -1,	u_sml, v_big,
			block_x + 1, block_y, block_z,		0, 0, -1,	u_big, v_sml,
			block_x + 1, block_y + 1, block_z,	0, 0, -1,	u_big, v_big,
			block_x + 1, block_y, block_z,		0, 0, -1,	u_big, v_sml,
			block_x, block_y + 1, block_z,		0, 0, -1,	u_sml, v_big,
		};

		append_ezarray(mesh, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +z face
	if (block_z == 15 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y][block_z+1])) {

		float full_block_data[] = {
			block_x, block_y, block_z + 1,			0, 0, 1,	u_big, v_sml,
			block_x + 1, block_y, block_z + 1,		0, 0, 1,	u_sml, v_sml,
			block_x, block_y + 1, block_z + 1,		0, 0, 1,	u_big, v_big,
			block_x + 1, block_y + 1, block_z + 1,	0, 0, 1,	u_sml, v_big,
			block_x, block_y + 1, block_z + 1,		0, 0, 1,	u_big, v_big,
			block_x + 1, block_y, block_z + 1,		0, 0, 1,	u_sml, v_sml,
		};

		append_ezarray(mesh, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -y face
	if (block_y == 0 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y-1][block_z])) {

		// get UV for bottom
		GET_SPRITEMAP_UV(BLOCK_GET_BOTTOM(block), u_sml, v_sml, u_big, v_big)

		float full_block_data[] = {
			block_x, block_y, block_z,			0, -1, 0,	u_sml, v_sml,
			block_x + 1, block_y, block_z,		0, -1, 0,	u_sml, v_big,
			block_x, block_y, block_z + 1,		0, -1, 0,	u_big, v_sml,
			block_x + 1, block_y, block_z + 1,	0, -1, 0,	u_big, v_big,
			block_x, block_y, block_z + 1,		0, -1, 0,	u_big, v_sml,
			block_x + 1, block_y, block_z,		0, -1, 0,	u_sml, v_big,
		};

		append_ezarray(mesh, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +y face
	if (block_y == 15 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y+1][block_z])) {

		// get UV for top
		GET_SPRITEMAP_UV(BLOCK_GET_TOP(block), u_sml, v_sml, u_big, v_big)

		float full_block_data[] = {
			block_x, block_y + 1, block_z,			0, 1, 0,	u_big, v_sml,
			block_x, block_y + 1, block_z + 1,		0, 1, 0,	u_sml, v_sml,
			block_x + 1, block_y + 1, block_z,		0, 1, 0,	u_big, v_big,
			block_x + 1, block_y + 1, block_z + 1,	0, 1, 0,	u_sml, v_big,
			block_x + 1, block_y + 1, block_z,		0, 1, 0,	u_big, v_big,
			block_x, block_y + 1, block_z + 1,		0, 1, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}
}

// remeshes based on the model's internal chunk_data
void remesh_chunk(const ChunkModel *chunk) {

	EZArray mesh = {0};

	int vertex_count = 0;

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			for (int z = 0; z < 16; z++)
				append_block_to_mesh(&mesh, &vertex_count, chunk->blocks, x, y, z);

	Model *model = create_model(mesh.data, mesh.bytecount, vertex_count, block_spritemap, 256, 256);

	memcpy((void *) &chunk->model, model, sizeof(Model));
	free(model);
}

void mat4_mult(const GLfloat b[4][4], const GLfloat a[4][4], GLfloat out[4][4]) {

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

void generate_rotation_matrices(GLfloat pitch_matrix[4][4], float pitch, GLfloat yaw_matrix[4][4], float yaw) {

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

void draw_model(const Transform *camera, const Model *model) {

	// shared buffers
	GLfloat pitch_matrix[4][4];
	GLfloat yaw_matrix[4][4];

	GLfloat position_matrix[4][4];

	// bind the model's vertex mesh and texture
	glBindVertexArray(model->vertex_array);
	glBindTexture(GL_TEXTURE_2D, model->texture);

	// model matrix (converts from model space to world space)
	generate_rotation_matrices(
		pitch_matrix, model->transform.pitch,
		yaw_matrix, model->transform.yaw
	);

	GLfloat model_matrix[4][4];

	mat4_mult(yaw_matrix, pitch_matrix, model_matrix); // rotation

	model_matrix[3][0] = model->transform.x; // translation
	model_matrix[3][1] = model->transform.y;
	model_matrix[3][2] = model->transform.z;

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

	// normal matrix (applied to normals to account for model rotation)
	GLfloat normal_matrix[4][4];

	generate_rotation_matrices(
		pitch_matrix, -model->transform.pitch,
		yaw_matrix, -model->transform.yaw
	);

	mat4_mult(yaw_matrix, pitch_matrix, normal_matrix);

	// load the shader program and the uniforms we just calculated
	glUseProgram(shader_program);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "position_matrix"), 1, GL_FALSE, &position_matrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "normal_matrix"), 1, GL_FALSE, &normal_matrix[0][0]);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, model->vertex_count);
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