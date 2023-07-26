#include "../pishtov.h"

#include <stdbool.h>

enum direction {
	LEFT,
	UP,
	RIGHT,
	DOWN,
	NUM_DIRECTIONS,
};

#define grid_w 100
#define grid_h 100

int upt = 1;

uint8_t grid[100][100];

ssize_t head_x = 50;
ssize_t head_y = 50;
uint8_t prev_dir = -1;

bool always_turn = true;

void init() {
	srand(time(0));
}

ssize_t norm_x(ssize_t x) {
	return (x % grid_w + grid_w) % grid_w;
}

ssize_t norm_y(ssize_t y) {
	return (y % grid_h + grid_h) % grid_h;
}

void update_no_go(ssize_t x, ssize_t y) {
	if ((grid[x][y] >> NUM_DIRECTIONS) & 1) return;

	int num_blocked = 0;

	if ((grid[x][y] >> LEFT  & 1) || ((grid[norm_x(x-1)][norm_y(y  )] >> NUM_DIRECTIONS) & 1)) num_blocked += 1;
	if ((grid[x][y] >> UP    & 1) || ((grid[norm_x(x  )][norm_y(y-1)] >> NUM_DIRECTIONS) & 1)) num_blocked += 1;
	if ((grid[x][y] >> RIGHT & 1) || ((grid[norm_x(x+1)][norm_y(y  )] >> NUM_DIRECTIONS) & 1)) num_blocked += 1;
	if ((grid[x][y] >> DOWN  & 1) || ((grid[norm_x(x  )][norm_y(y+1)] >> NUM_DIRECTIONS) & 1)) num_blocked += 1;

	if (num_blocked == 3) {
		grid[x][y] |= 1 << NUM_DIRECTIONS;
		update_no_go(norm_x(x+1), norm_y(y  ));
		update_no_go(norm_x(x-1), norm_y(y  ));
		update_no_go(norm_x(x  ), norm_y(y+1));
		update_no_go(norm_x(x  ), norm_y(y-1));
	}
}

void tick() {
	ssize_t delta_x[] = {
		[LEFT]  = -1,
		[UP]    = 0,
		[RIGHT] = 1,
		[DOWN]  = 0,
	};

	ssize_t delta_y[] = {
		[LEFT]  = 0,
		[UP]    = -1,
		[RIGHT] = 0,
		[DOWN]  = 1,
	};

	enum direction opposite[] = {
		[LEFT]  = RIGHT,
		[UP]    = DOWN,
		[RIGHT] = LEFT,
		[DOWN]  = UP,
	};

	while (true) {
		uint8_t dir = rand() % NUM_DIRECTIONS;

		ssize_t next_x = norm_x(head_x + delta_x[dir]);
		ssize_t next_y = norm_y(head_y + delta_y[dir]);

		if (grid[next_x][next_y] & (1 << opposite[dir])) continue;
		if (grid[next_x][next_y] & (1 << NUM_DIRECTIONS)) continue;
		if (always_turn && prev_dir == dir) continue;

		grid[head_x][head_y] |= 1 << dir;
		grid[next_x][next_y] |= 1 << opposite[dir];

		update_no_go(head_x, head_y);

		head_x = next_x;
		head_y = next_y;
		prev_dir = dir;

		break;
	}
}

void update() {
	static int64_t updates;
	if (updates == 0) tick();
	updates += 1;
	updates %= upt;
}

void draw() {
	{
		float scale_to_fit_w = window_w / grid_w;
		float scale_to_fit_h = window_h / grid_h;
		float scale_to_fit = fminf(scale_to_fit_w, scale_to_fit_h);
		scale(scale_to_fit, scale_to_fit);
	}

	for (ssize_t x = 0; x < grid_w; x += 1) {
		for (ssize_t y = 0; y < grid_w; y += 1) {
			if (grid[x][y] & (1 << NUM_DIRECTIONS)) fill_color(0xff0000);
			else                                    fill_color(0x0000ff);

			if (grid[x][y] & (1 << LEFT))  fill_rect(x,    y-.1, -.5,  .2);
			if (grid[x][y] & (1 << UP))    fill_rect(x-.1, y,     .2, -.5);
			if (grid[x][y] & (1 << RIGHT)) fill_rect(x,    y-.1, +.5,  .2);
			if (grid[x][y] & (1 << DOWN))  fill_rect(x-.1, y,     .2, +.5);
		}
	}
}

void keydown(int key) {
	if (key == 38) upt *= 2;
	if (key == 40) {
		upt /= 2;
		if (upt < 1) upt = 1;
	}

	printf("upt: %d\n", upt);

	printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
	printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

