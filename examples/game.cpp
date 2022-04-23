#include "pishtov.h"
#include <iostream>

using namespace std;

float my_x;
float my_y;

void init() {
	my_x = 0;
	my_y = 0;
}

void update() {
	my_x += (mouse_x - my_x) / 10;
	my_y += (mouse_y - my_y) / 10;
}

void draw() {
	fill_rect(my_x, my_y, 30, 30);
}

void keydown(int key) {
	cout << "Keydown " << key << std::endl;
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
	cout << "Mouse clicked at " << mouse_x << " " << mouse_y << "from" << window_w << " " << window_h << std::endl;
}

