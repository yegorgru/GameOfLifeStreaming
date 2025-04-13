#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <raylib.h>

#include "IStreamingClient.h"
#include "StreamingFactory.h"

// Sample ASCII data to render (simulating received data from server)
const char* sampleAsciiFrame = 
    "##################\n"
    "#                #\n"
    "#  ###   ###     #\n"
    "#   #     #      #\n"
    "#   #     #      #\n"
    "#                #\n"
    "#       ##       #\n"
    "#      #  #      #\n"
    "#      #  #      #\n"
    "#       ##       #\n"
    "#                #\n"
    "##################";

// Render ASCII frame using raylib
void RenderAsciiFrame(const std::string& frame, int cellSize) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    int row = 0;
    int col = 0;

    for (char c : frame) {
        if (c == '\n') {
            row++;
            col = 0;
            continue;
        }

        if (c == '#' || c == '*' || c == 'O') {
            // Draw filled cell for live cells
            DrawRectangle(col * cellSize, row * cellSize, cellSize, cellSize, BLACK);
        } else {
            // Draw empty cell with border
            DrawRectangleLines(col * cellSize, row * cellSize, cellSize, cellSize, GRAY);
        }
        col++;
    }

    DrawFPS(10, 10);
    EndDrawing();
}

int main() {
    const int cellSize = 30;
    const int gridWidth = 18;  // Width of the sample frame
    const int gridHeight = 12; // Height of the sample frame
    
    // Initialize raylib window
    InitWindow(gridWidth * cellSize, gridHeight * cellSize, "Game of Life Client - raylib test");
    SetTargetFPS(60);

    std::string currentFrame = sampleAsciiFrame;
    float frameUpdateTime = 0.0f;
    
    // Main game loop
    while (!WindowShouldClose()) {
        // Update frame every second for testing
        frameUpdateTime += GetFrameTime();
        if (frameUpdateTime >= 1.0f) {
            // In a real implementation, this would receive data from the server
            frameUpdateTime = 0.0f;
            
            // Just toggle some cells in the sample frame to simulate updates
            std::string modifiedFrame = currentFrame;
            for (size_t i = 0; i < modifiedFrame.length(); i++) {
                if (modifiedFrame[i] == ' ' && GetRandomValue(0, 20) == 0) {
                    modifiedFrame[i] = '#';
                } else if (modifiedFrame[i] == '#' && GetRandomValue(0, 10) == 0) {
                    modifiedFrame[i] = ' ';
                }
            }
            currentFrame = modifiedFrame;
        }
        
        // Render the current frame
        RenderAsciiFrame(currentFrame, cellSize);
    }
    
    // Clean up raylib resources
    CloseWindow();
    
    return 0;
}