#include "pishtov.h"

#define NUM_CIRCLES 100

float my_x;
float my_y;

float frame = 0;

float frand() {
    return (float)rand() / (float)RAND_MAX;
}

void init() {
}

void update() {
    ++frame;
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
    translate(my_x, my_y);
    rotate(frame/50);
    translate(-my_x, -my_y);

    fill_rect(my_x, my_y, 30, 30);

    translate(my_x, my_y);
    rotate(-frame/50);
    translate(-my_x, -my_y);

    fill_color_hex(0xff0000);
    fill_circle(window_w/2, window_h/2, 15);
}

void keydown(int key) {
    printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
    printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

