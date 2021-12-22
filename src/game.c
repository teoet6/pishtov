#include "pishtov.h"

float my_x;
float my_y;

int is_button_pressed[10];

float start_x, start_y;

void init() { }

void update() {
    my_x += (mouse_x - my_x) / 10;
    my_y += (mouse_y - my_y) / 10;
}

void fill_color_hex(uint32_t hex) {
    int a = hex >> 24 & 0xff;
    int r = hex >> 16 & 0xff;
    int g = hex >>  8 & 0xff;
    int b = hex       & 0xff;

    fill_color[0] = (float)r / 255;
    fill_color[1] = (float)g / 255;
    fill_color[2] = (float)b / 255;
    fill_color[3] = 1 - (float)a / 255;
}

void draw() {
    fill_circle(my_x, my_y, 15);
}

void keydown(int key) {
    printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) {
    is_button_pressed[button] = 1;
    start_x = mouse_x;
    start_y = mouse_y;
}

void mouseup(int button) {
    printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
    is_button_pressed[button] = 0;
}

