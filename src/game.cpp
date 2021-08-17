#include "pishtov.h"

void keydown(int key) { 
    std::cout << "Keydown " << key << std::endl;
}

void keyup(int key) { }

void mousedown(int button) { }

void mouseup(int button) { 
    std::cout << "Mouse clicked at " << mouse_x << " " << mouse_y << std::endl;
}

int main() {
    open_window("Igra", 800, 600);
    std::cout << "opened?" << std::endl;
    while (true) {
        handle_events();
    }
}
