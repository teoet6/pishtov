#include "pishtov.h"
#include <stdint.h>
#include <time.h>

float xs[256];
float ys[256];

void init() {
    for (int i = 0; i < 256; ++i) {
        xs[i] = rand() % 800;
        ys[i] = rand() % 600;
    }
}

void update() { }

void draw() {
    static float frame;
    ++frame;

    fill_rect(0, 0, window_w, window_h);

    for (int i = 0; i < 256; ++i) {
        fill_color(i << 16 | i << 8 | i);

        fill_triangle(
            xs[i],                        ys[i],
            xs[i] + 50 * cos(frame / 40), ys[i] + 60 * sin(frame / 40),
            xs[i] + 40 * cos(frame / 25), ys[i] + 40 * sin(frame / 25)
        );
    }

}

void keydown(int key) {
    printf("Keydown %d\n", key);
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) {
    printf("Mouse clicked at %.0f %.0f from %.0f %.0f\n", mouse_x, mouse_y, window_w, window_h);
}

