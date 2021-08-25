#include "pishtov.h"

float my_x;
float my_y;

void init() {
    line_width = 2;
}

void update() {
    my_x += (mouse_x - my_x) / 10;
    my_y += (mouse_y - my_y) / 10;
}

void draw() {
    fill_style(0, 0, 1);
    fill_rect(my_x, my_y, 30, 30);

    fill_style(1, 0, 0);
    fill_line(my_x, my_y, mouse_x, mouse_y);
}

void keydown(int key) { 
    std::cout << "Keydown " << key << std::endl;
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) { 
    std::cout << "Mouse clicked at " << mouse_x << " " << mouse_y << " from " << window_w << " " << window_h << std::endl;
}

