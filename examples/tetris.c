#include "../pishtov.h"

#include <stdbool.h>

#define board_h 20
#define board_w 10

#define block_ids 7

bool board[board_w][board_h];

typedef struct Block_Definition Block_Definition;
struct Block_Definition {
	int64_t x[4][4];
	int64_t y[4][4];
	uint32_t color;
};

typedef struct Block Block;
struct Block {
	uint64_t id;
	int64_t x;
	int64_t y;
	int64_t rot;
};

Block_Definition block_defs[block_ids] = {
	// I
	{
		.x = { { -1, 0, 1, 2 }, {  0, 0,  0, 0 }, { -1, 0, 1, 2 }, {  0, 0,  0, 0 } },
		.y = { {  0, 0, 0, 0 }, { -1, 0,  1, 2 }, {  0, 0, 0, 0 }, { -1, 0,  1, 2 } },
		.color = 0x000000,
	},
	// J
	{
		.x = { { -1, -1, 0, 1 }, {  1,  0, 0, 0 }, { 1, 1, 0, -1 }, { -1, 0, 0,  0 } },
		.y = { { -1,  0, 0, 0 }, { -1, -1, 0, 1 }, { 1, 0, 0,  0 }, {  1, 1, 0, -1 } },
		.color = 0x0000ff,
	},
	// L
	{
		.x = { { -1, 0, 1,  1 }, {  0, 0,  0, 1 }, { 1, 0, -1, -1 }, { 0, 0,  0, -1 } },
		.y = { {  0, 0, 0, -1 }, { -1, 0,  1, 1 }, { 0, 0,  0,  1 }, { 1, 0, -1, -1 } },
		.color = 0x00ff00,
	},
	// O
	{
		.x = { { 0, 1, 1, 0 }, { 0, 1, 1, 0 }, { 0, 1, 1, 0 }, { 0, 1, 1, 0 } },
		.y = { { 0, 0, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 1, 1 } },
		.color = 0x00ffff,
	},
	// S
	{
		.x = { { 1, 0, 0, -1 }, { 0, 0, -1, -1 }, { 1, 0, 0, -1 }, { 0, 0, -1, -1 } },
		.y = { { 0, 0, 1,  1 }, { 1, 0,  0, -1 }, { 0, 0, 1,  1 }, { 1, 0,  0, -1 } },
		.color = 0xff0000,
	},
	// T
	{
		.x = { { 0, -1, 0, 1 }, { 0,  0, -1, 0 }, { 0, 1,  0, -1 }, { 0, 0, 1,  0 } },
		.y = { { 0,  0, 1, 0 }, { 0, -1,  0, 1 }, { 0, 0, -1,  0 }, { 0, 1, 0, -1 } },
		.color = 0xff00ff,
	},
	// Z
	{
		.x = { { -1, 0, 0,  1 }, {  0, 0, -1, -1 }, { -1, 0, 0,  1 }, {  0, 0, -1, -1 } },
		.y = { {  0, 0, 1,  1 }, { -1, 0,  0,  1 }, {  0, 0, 1,  1 }, { -1, 0,  0,  1 } },
		.color = 0xffff00,
	},
};

Block block;

void save_block() {
	for (uint64_t i = 0; i < 4; i += 1) {
		int64_t x = block.x + block_defs[block.id].x[block.rot][i];
		int64_t y = block.y + block_defs[block.id].y[block.rot][i];
		board[x][y] = true;
	}
}

void next_block() {
	block.id = rand() % block_ids;
	block.rot = 0;
	block.x = 4;
	block.y = -2;
}

void init() {
	srand(time(0));
	next_block();
}

bool block_conflicts() {
	for (uint64_t i = 0; i < 4; i += 1) {
		int64_t x = block.x + block_defs[block.id].x[block.rot][i];
		int64_t y = block.y + block_defs[block.id].y[block.rot][i];

		if (x < 0)        return true;
		if (x >= board_w) return true;
		if (y >= board_h) return true;

		if (y < 0) continue;
		if (board[x][y]) return true;
	}

	return false;
}

void tick() {
	block.y += 1;
	if (block_conflicts()) {
		block.y -= 1;
		save_block();
		next_block();
	}
}

void update() {
	static uint64_t updates;
	if (updates == 0) tick();
	updates += 1;
	updates %= 30;
}

void draw() {
	{
		float scale_to_fit_w = window_w / board_w;
		float scale_to_fit_h = window_h / board_h;
		float scale_to_fit = fminf(scale_to_fit_w, scale_to_fit_h);
		scale(scale_to_fit, scale_to_fit);
	}

	for (uint64_t x = 0; x < board_w; x += 1) {
		for (uint64_t y = 0; y < board_h; y += 1) {
			if (board[x][y]) fill_color(0x808080);
			else             fill_color(0xf0f0f0);
			fill_rect(x, y, .95f, .95f);
		}
	}

	fill_color(block_defs[block.id].color);
	for (uint64_t i = 0; i < 4; i += 1) {
		float x = block.x + block_defs[block.id].x[block.rot][i];
		float y = block.y + block_defs[block.id].y[block.rot][i];
		fill_rect(x, y, .95f, .95f);
	}
}

void keydown(int key) {
	if (key == 'Z') {
		int64_t prev = block.rot;
		if (block.rot == 0) block.rot = 3;
		else block.rot -= 1;
		if (block_conflicts()) block.rot = prev;
		return;
	}

	if (key == 'X' || key == 38) {
		int64_t prev = block.rot;
		if (block.rot == 3) block.rot = 0;
		else block.rot += 1;
		if (block_conflicts()) block.rot = prev;
		return;
	}

	if (key == 37) {
		block.x -= 1;
		if (block_conflicts()) block.x += 1;
		return;
	}

	if (key == 39) {
		block.x += 1;
		if (block_conflicts()) block.x -= 1;
		return;
	}

	if (key == 40) {
		block.y += 1;
		if (block_conflicts()) {
			block.y -= 1;
			save_block();
			next_block();
		}
		return;
	}

	if (key == ' ') {
		while (!block_conflicts()) block.y += 1;
		block.y -= 1;
		save_block();
		next_block();
		return;
	}

	printf("Keydown %d", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
	printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

