#include "pishtov.h"

float my_x;
float my_y;

void init() { }

void update() {
    my_x += (mouse_x - my_x) / 10;
    my_y += (mouse_y - my_y) / 10;
}

void draw() {
    fill_circle(my_x, my_y, 15);
}

void keydown(int key) {
    printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
    printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

