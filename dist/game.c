#include "pishtov.h"
#include <stdint.h>
#include <time.h>

float my_x;
float my_y;

void init() { }

uint64_t get_timestamp() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

float frand() {
    return (float)rand() / RAND_MAX;
}

void fill_color_hex(uint32_t c) {
    fill_color[0] =     (c >> 16 & 0xff) / 255.f;
    fill_color[1] =     (c >>  8 & 0xff) / 255.f;
    fill_color[2] =     (c       & 0xff) / 255.f;
    fill_color[3] = 1 - (c >> 24 & 0xff) / 255.f;
}

void update() {
    my_x += (mouse_x - my_x) / 10;
    my_y += (mouse_y - my_y) / 10;
}

void draw() {
    uint64_t begin_draw = get_timestamp();

    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            fill_color[0] = i / 1000.f;
            fill_color[1] = j / 1000.f;
            fill_color[2] = 1.f;
            fill_rect(j, i, 1, 1);
        }
    }

    fill_color_hex(0xffffff);
    fill_ellipse(my_x, my_y, 15, 15);

    uint64_t end_draw = get_timestamp();
    printf("draw() took %.0fms\n", (end_draw - begin_draw) / 1000000.f);
}

void keydown(int key) {
    printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
    printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

