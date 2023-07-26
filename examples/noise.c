#include "../pishtov.h"
#undef __STRICT_ANSI__
#include <math.h>

#define LEN(X) (sizeof(X) / sizeof(X[0]))

double buffer[500];
int cur_idx;

double frand() {
	return (double)rand() / RAND_MAX;
}

double f(double x) {
	double order = (sin((float)cur_idx / LEN(buffer) * 3.141595 * 2) + 1) / 2;
	double chaos = frand();
	return x * order + (1 - x) * chaos;
}

void init() {
	buffer[cur_idx] = 0;
}

void update() {
	int next_idx = (cur_idx + 1) % LEN(buffer);
	buffer[next_idx] = f(buffer[cur_idx]);
	cur_idx = next_idx;
}

void draw() {
	double bar_w = window_w / LEN(buffer);

	for (int i = 0; i < LEN(buffer); i += 1) {
		pshtv_fill_color[2] = (i - cur_idx + LEN(buffer)) % LEN(buffer) / (float)LEN(buffer);
		fill_rect(i * bar_w, window_h * (1 - buffer[i]), bar_w, window_h * buffer[i]);
	}
}

void keydown(int key) { }
void keyup(int key) { }
void mousedown(int button) { }
void mouseup(int button) { }
