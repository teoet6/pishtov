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

void draw() {
    for (int i = 0; i < 30; ++i) {
        for (int j = 0; j < 30; ++j) {
            pshtv_fill_color[0] = i / 30.f;
            pshtv_fill_color[1] = j / 30.f;
            pshtv_fill_color[2] = 1.f;
            fill_rect(j * 20, i * 20, 15, 15);
        }
    }

    fill_color(0xffffff);
    fill_ellipse(my_x, my_y, 15, 15);
}

void keydown(int key) {
    printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
    printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

