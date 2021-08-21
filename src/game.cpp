#include "pishtov.h"
using pshtv::fill_rect;
using pshtv::fill_circle;
using pshtv::fill_style;

float my_x, my_y;

void update() {
    my_x += (mouse_x - my_x) / 10;
    my_y += (mouse_y - my_y) / 10;
}

void draw() {
    fill_style(0, 0, 1);
    fill_circle(my_x, my_y, 20);
}

void keydown(int key) { 
    std::cout << "Keydown " << key << std::endl;
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) { 
    std::cout << "Mouse clicked at " << mouse_x << " " << mouse_y << " from " << window_w << " " << window_h << std::endl;
}

