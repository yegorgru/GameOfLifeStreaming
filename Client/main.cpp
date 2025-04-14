#include "Application.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        GameOfLife::Client::Application app;

        if (!app.initialize(argc, argv)) {
            return 1;
        }
        app.run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
}