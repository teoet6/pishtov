#include "pishtov.h"
#include <iostream>

#define ARR_LEN(X) (sizeof(X) / sizeof((X)[0]))

using namespace std;

struct Vertex3 {
	float pos[3];
	float col[4];
};

Vertex3 pyramid_mesh[] = {
	// Front
	{ .pos = { -1, -1, -1 }, .col = { 1, 0, 0, 1 } },
	{ .pos = {  0,  1, -1 }, .col = { 1, 0, 0, 1 } },
	{ .pos = {  1, -1, -1 }, .col = { 1, 0, 0, 1 } },

	// Left
	{ .pos = { -1, -1,  -1 }, .col = { 0, 1, 0, 1 } },
	{ .pos = {  0,  1,  -1 }, .col = { 0, 1, 0, 1 } },
	{ .pos = {  0, -.17, 1 }, .col = { 0, 1, 0, 1 } },

	// Right
	{ .pos = {  1, -1,  -1 }, .col = { 0, 0, 1, 1 } },
	{ .pos = {  0,  1,  -1 }, .col = { 0, 0, 1, 1 } },
	{ .pos = {  0, -.17, 1 }, .col = { 0, 0, 1, 1 } },

	// Bottom
	{ .pos = { -1, -1,  -1 }, .col = { 0, 0, 0, 1 } },
	{ .pos = {  0, -.17, 1 }, .col = { 0, 0, 0, 1 } },
	{ .pos = {  1, -1,  -1 }, .col = { 0, 0, 0, 1 } },
};

GLuint pyramid_vao, pyramid_vbo;
GLuint solid_shader_prog;

void init() {
	pglDepthFunc(GL_LEQUAL);

	solid_shader_prog = pshtv_make_shader_prog(
	R"XXX(
		#version 130

		in vec3 in_pos;
		in vec4 in_col;

		out vec4 ex_col;

		uniform mat4 u_transform;

		void main() {
			vec4 transformed = u_transform * vec4(in_pos, 1.0);
			gl_Position.xy = transformed.xy / transformed.z;
			gl_Position.z  = transformed.z / (transformed.z + 1);
			gl_Position.w  = 1.0;
			ex_col = in_col;
		}
	)XXX",
	R"XXX(
		#version 130

		in vec4 ex_col;

		void main() {
			gl_FragColor = ex_col;
		}
	)XXX"
	);

	pglGenVertexArrays(1, &pyramid_vao);
	pglBindVertexArray(pyramid_vao);

	pglGenBuffers(1, &pyramid_vbo);
	pglBindBuffer(GL_ARRAY_BUFFER, pyramid_vbo);
	pglBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_mesh), pyramid_mesh, GL_STATIC_DRAW);
}

void update() {
}

void ver_rotate(float a) {
	float s = sin(a);
	float c = cos(a);
	float rotate_matrix[4][4] = {
		{ c, 0, -s, 0 },
		{ 0, 1,  0, 0 },
		{ s, 0,  c, 0 },
		{ 0, 0,  0, 1 },
	};
	pshtv_mul_transform_matrix_by(rotate_matrix);
}

float a = 0;

void draw() {
	pglClearDepth(1.0);
	pglClear(GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			pshtv_transform_matrix[i][j] = i == j ? 1 : 0;
	pshtv_transform_matrix[2][3] = 3;

	a += .01;
	ver_rotate(a);

	pglUseProgram(solid_shader_prog);

	{
		static GLint in_pos, in_col, u_transform;
		if (!in_pos) in_pos = pglGetAttribLocation(solid_shader_prog, "in_pos");
		if (!in_col) in_col = pglGetAttribLocation(solid_shader_prog, "in_col");
		if (!u_transform) u_transform = pglGetUniformLocation(solid_shader_prog, "u_transform");

		pglBindVertexArray(pyramid_vao);
		pglBindBuffer(GL_ARRAY_BUFFER, pyramid_vbo);

		PSHTV_PASS_FIELD_AS_ATTRIBUTE(solid_shader_prog, in_pos, 3, GL_FLOAT, GL_FALSE, struct Vertex3, pos);
		PSHTV_PASS_FIELD_AS_ATTRIBUTE(solid_shader_prog, in_col, 4, GL_FLOAT, GL_FALSE, struct Vertex3, col);

		pglUniformMatrix4fv(u_transform, 1, GL_TRUE, (const float*)pshtv_transform_matrix);
		pglEnableVertexAttribArray(u_transform);

		pglDrawArrays(GL_TRIANGLES, 0, ARR_LEN(pyramid_mesh));
	}
}

void keydown(int key) {
	cout << "Keydown " << key << std::endl;
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
	cout << "Mouse clicked at " << mouse_x << " " << mouse_y << " from " << window_w << " " << window_h << std::endl;
}

