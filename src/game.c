#include "pishtov.h"
#include <stdint.h>
#include <time.h>

float my_x;
float my_y;

void init() { }

void update() {
    my_x += (mouse_x - my_x) / 10;
    my_y += (mouse_y - my_y) / 10;
}

#define WHITE 0xff, 0xff, 0xff, 0xff
#define GREEN 0x00, 0xff, 0x00, 0xff
#define RED   0xff, 0x00, 0x00, 0xff

void draw() {
    fill_color(0x000040);
    fill_rect(0, 0, window_w, window_h);

    uint8_t flag[] = {
        WHITE, WHITE, WHITE, WHITE,
        GREEN, GREEN, GREEN, GREEN,
        RED,   RED,   RED,   RED,
    };

    draw_image_buffer(flag, 4, 3, my_x, my_y, 40, 30);
}

void keydown(int key) {
    printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
    printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

