#include "../pishtov.h"

#include <stdbool.h>

#define board_h 20
#define board_w 10

#define next_len 3

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
	int64_t id;
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
		.x = { { 0, -1, -1,  0 }, { 0, -1, -1,  0 }, { 0, -1, -1,  0 }, { 0, -1, -1,  0 } },
		.y = { { 0,  0, -1, -1 }, { 0,  0, -1, -1 }, { 0,  0, -1, -1 }, { 0,  0, -1, -1 } },
		.color = 0x00ffff,
	},
	// S
	{
		.x = { {  1,  0, 0, -1 }, { 1, 1, 0,  0 }, {  1,  0, 0, -1 }, { 1, 1, 0,  0 } },
		.y = { { -1, -1, 0,  0 }, { 1, 0, 0, -1 }, { -1, -1, 0,  0 }, { 1, 0, 0, -1 } },
		.color = 0xff0000,
	},
	// T
	{
		.x = { { 0, 1,  0, -1 }, { 0, 0, 1,  0 }, { 0, -1, 0, 1 }, { 0,  0, -1, 0 } },
		.y = { { 0, 0, -1,  0 }, { 0, 1, 0, -1 }, { 0,  0, 1, 0 }, { 0, -1,  0, 1 } },
		.color = 0xff00ff,
	},
	// Z
	{
		.x = { { -1,  0, 0, 1 }, {  1, 1, 0, 0 }, { -1,  0, 0, 1 }, {  1, 1, 0, 0 } },
		.y = { { -1, -1, 0, 0 }, { -1, 0, 0, 1 }, { -1, -1, 0, 0 }, { -1, 0, 0, 1 } },
		.color = 0xffff00,
	},
};

bool can_hold = true;
int64_t hold_id = -1;
int64_t next_ids[next_len];
Block block;

void save_block() {
	can_hold = true;

	for (int64_t i = 0; i < 4; i += 1) {
		int64_t x = block.x + block_defs[block.id].x[block.rot][i];
		int64_t y = block.y + block_defs[block.id].y[block.rot][i];
		board[x][y] = true;
	}

	int64_t fall = 0;

	for (int64_t y = board_h - 1; y >= 0; y -= 1) {
		bool full_line = true;
		for (int64_t x = 0; x < board_w; x += 1) {
			board[x][y + fall] = board[x][y];
			full_line = full_line && board[x][y];
		}
		if (full_line) fall += 1;
	}

	for (int64_t y = 0; y < fall; y += 1) {
		for (int64_t x = 0; x < board_w; x += 1) {
			board[x][y] = false;
		}
	}
}

void next_block() {
	block.id = next_ids[0];
	block.rot = 0;
	block.x = 4;
	block.y = -2;

	for (int64_t i = 1; i < next_len; i += 1) {
		next_ids[i - 1] = next_ids[i];
	}
	next_ids[next_len - 1] = rand() % block_ids;
}

void hold_block() {
	can_hold = false;

	if (hold_id == -1) {
		hold_id = block.id;
		next_block();
		return;
	}

	int64_t tmp_id = hold_id;
	hold_id = block.id;

	block.id = tmp_id;
	block.rot = 0;
	block.x = 4;
	block.y = -2;
}

void init() {
	srand(time(0));
	for (int64_t i = 0; i < next_len; i += 1) {
		next_ids[i] = rand() % block_ids;
	}
	next_block();
}

bool block_conflicts(Block b) {
	for (int64_t i = 0; i < 4; i += 1) {
		int64_t x = b.x + block_defs[b.id].x[b.rot][i];
		int64_t y = b.y + block_defs[b.id].y[b.rot][i];

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
	if (block_conflicts(block)) {
		block.y -= 1;
		save_block();
		next_block();
	}
}

void update() {
	static int64_t updates;
	if (updates == 0) tick();
	updates += 1;
	updates %= 30;
}

void draw() {
	{
		float needed_w = board_w + 5;
		float needed_h = board_h;

		float scale_to_fit_w = window_w / needed_w;
		float scale_to_fit_h = window_h / needed_h;
		float scale_to_fit = fminf(scale_to_fit_w, scale_to_fit_h);
		scale(scale_to_fit, scale_to_fit);
	}

	for (int64_t x = 0; x < board_w; x += 1) {
		for (int64_t y = 0; y < board_h; y += 1) {
			if (board[x][y]) fill_color(0x808080);
			else             fill_color(0xf0f0f0);
			fill_rect(x, y, .95f, .95f);
		}
	}

	{
		Block ghost = block;
		while (!block_conflicts(ghost)) {
			ghost.y += 1;
		}
		ghost.y -= 1;

		fill_color(0xffffff);
		for (int64_t i = 0; i < 4; i += 1) {
			float x = ghost.x + block_defs[ghost.id].x[ghost.rot][i];
			float y = ghost.y + block_defs[ghost.id].y[ghost.rot][i];
			fill_rect(x, y, .95f, .95f);
		}
	}

	fill_color(block_defs[block.id].color);
	for (int64_t i = 0; i < 4; i += 1) {
		float x = block.x + block_defs[block.id].x[block.rot][i];
		float y = block.y + block_defs[block.id].y[block.rot][i];
		fill_rect(x, y, .95f, .95f);
	}

	fill_color(block_defs[hold_id].color);
	for (int64_t i = 0; i < 4; i += 1) {
		float x = board_w + 2 + block_defs[hold_id].x[0][i];
		float y = board_h - 2 + block_defs[hold_id].y[0][i];
		fill_rect(x, y, .95f, .95f);
	}

	for (int64_t i = 0; i < next_len; i += 1) {
		fill_color(block_defs[next_ids[i]].color);
		for (int64_t j = 0; j < 4; j += 1) {
			float x = board_w + 2 + block_defs[next_ids[i]].x[0][j];
			float y = i * 3 + 3   + block_defs[next_ids[i]].y[0][j];
			fill_rect(x, y, .95f, .95f);
		}
	}
}

void keydown(int key) {
	if (key == 'Z') {
		int64_t prev = block.rot;
		if (block.rot == 0) block.rot = 3;
		else block.rot -= 1;
		if (block_conflicts(block)) block.rot = prev;
		return;
	}

	if (key == 'X' || key == 38) {
		int64_t prev = block.rot;
		if (block.rot == 3) block.rot = 0;
		else block.rot += 1;
		if (block_conflicts(block)) block.rot = prev;
		return;
	}

	if (key == 'C') {
		if (can_hold) hold_block();
	}

	if (key == 37) {
		block.x -= 1;
		if (block_conflicts(block)) block.x += 1;
		return;
	}

	if (key == 39) {
		block.x += 1;
		if (block_conflicts(block)) block.x -= 1;
		return;
	}

	if (key == 40) {
		block.y += 1;
		if (block_conflicts(block)) {
			block.y -= 1;
			save_block();
			next_block();
		}
		return;
	}

	if (key == ' ') {
		while (!block_conflicts(block)) block.y += 1;
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

