#include <iostream>
#include "Application.h"

int main(int argc, char* argv[]) {
    try {
        GameOfLife::Server::Application app;
        
        // Initialize the application
        if (!app.initialize(argc, argv)) {
            return 1;
        }
        
        // Run the application (this will block until shutdown)
        app.run();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
}