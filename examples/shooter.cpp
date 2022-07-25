#include "../pishtov.h"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

float my_speed =   5;
float my_x     = 300;
float my_y     = 300;
float my_w     =  30;
float my_h     =  30;

int64_t       bullet_count =  0;
float         bullet_speed = 15;
float         bullet_w     =  5;
float         bullet_h     = 10;
float         bullet_dmg   =  1;
vector<float> bullet_xs;
vector<float> bullet_ys;

int64_t       enemy_count  =   0;
float         enemy_max_hp = 100;
float         enemy_speed  =   1;
float         enemy_w      =  60;
float         enemy_h      =  60;
vector<float> enemy_xs;
vector<float> enemy_ys;
vector<float> enemy_hps;

float start_updates_to_next_enemy_factor = .95;
float start_updates_to_next_enemy = 1000;
float updates_to_next_enemy       =    0;

bool is_key_pressed[256];

float frand() {
	return (float) rand() / RAND_MAX;
}

bool are_colliding(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
	return x1 + w1 >= x2 && x2 + w2 >= x1 && y1 + h1 >= y2 && y2 + h2 >= y1;
}

void init() { }

void update() {
	for (int64_t i = 0; i < bullet_count; i += 1) {
		bullet_ys[i] -= bullet_speed;
	}

	for (int64_t i = 0; i < enemy_count; i += 1) {
		enemy_ys[i] += enemy_speed;
	}

	for (int64_t i = 0; i < bullet_count; i += 1) {
		for (int64_t j = 0; j < enemy_count; j += 1) {
			if (are_colliding(bullet_xs[i], bullet_ys[i], bullet_w, bullet_h, enemy_xs[j], enemy_ys[j], enemy_w, enemy_h)) {
				bullet_xs[i] = -1000;
				enemy_hps[j] -= bullet_dmg;
				if (enemy_hps[j] <= 0) {
					enemy_xs[j] = -1000;
				}
			}
		}
	}

	if (is_key_pressed[37]) my_x -= my_speed;
	if (is_key_pressed[38]) my_y -= my_speed;
	if (is_key_pressed[39]) my_x += my_speed;
	if (is_key_pressed[40]) my_y += my_speed;

	if (is_key_pressed[32]) {
		bullet_xs.push_back(my_x + (my_w - bullet_w) * frand());
		bullet_ys.push_back(my_y);
		bullet_count += 1;

		my_y += my_speed * .75;
	}

	updates_to_next_enemy -= 1;
	if (updates_to_next_enemy <= 0) {
		enemy_xs.push_back(frand() * (window_w - enemy_w));
		enemy_ys.push_back(-enemy_w);
		enemy_hps.push_back(enemy_max_hp);
		enemy_count += 1;

		start_updates_to_next_enemy *= start_updates_to_next_enemy_factor;
		updates_to_next_enemy = start_updates_to_next_enemy;
	}
}

void draw() {
	fill_color(0xc08000);
	for (int64_t i = 0; i < bullet_count; i += 1) {
		fill_rect(bullet_xs[i], bullet_ys[i], bullet_w, bullet_h);
	}

	fill_color(0x0000ff);
	fill_rect(my_x, my_y, my_w, my_h);

	fill_color(0xff0000);
	for (int64_t i = 0; i < enemy_count; i += 1) {
		fill_rect(enemy_xs[i], enemy_ys[i] - 6, enemy_w * enemy_hps[i] / enemy_max_hp, 3);
		fill_rect(enemy_xs[i], enemy_ys[i], enemy_w, enemy_h);
	}
}

void keydown(int key) {
	cout << "Keydown " << key << std::endl;
	is_key_pressed[key] = true;
}

void keyup(int key) {
	is_key_pressed[key] = false;
}

void mousedown(int button) { }

void mouseup(int button) {
	cout << "Mouse clicked at " << mouse_x << " " << mouse_y << " from " << window_w << " " << window_h << std::endl;
}

