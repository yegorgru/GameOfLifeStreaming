#include "GameOfLife.h"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip> // Required for std::setw and std::setfill

namespace GameOfLife::Server {

namespace {
    std::random_device gRandomDevice;
    std::mt19937 gRandomGenerator = std::mt19937(gRandomDevice());
}

GameOfLife::GameOfLife(int width, int height)
    : mWidth(width)
    , mHeight(height) 
{
    mGrid.resize(mHeight, std::vector<bool>(mWidth, false));
}

void GameOfLife::initializeRandom(float fillRatio) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            mGrid[y][x] = (dist(gRandomGenerator) < fillRatio);
        }
    }
}

void GameOfLife::update() {
    auto newGrid = mGrid;
    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            int neighbors = countLivingNeighbors(x, y);
            bool currentlyAlive = mGrid[y][x];
            
            // Apply the rules of Conway's Game of Life:
            // 1. Any live cell with fewer than two live neighbors dies (underpopulation)
            // 2. Any live cell with two or three live neighbors lives on
            // 3. Any live cell with more than three live neighbors dies (overpopulation)
            // 4. Any dead cell with exactly three live neighbors becomes a live cell (reproduction)
            
            if (currentlyAlive) {
                newGrid[y][x] = (neighbors == 2 || neighbors == 3);
            } else {
                newGrid[y][x] = (neighbors == 3);
            }
        }
    }
    
    mGrid = std::move(newGrid);
}

std::string GameOfLife::toString() const {
    std::ostringstream ss;
    ss << std::setw(3) << std::setfill('0') << mWidth 
       << 'x' 
       << std::setw(3) << std::setfill('0') << mHeight;

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            ss << (mGrid[y][x] ? '#' : ' ');
        }
    }

    ss << "\n";

    return ss.str();
}

int GameOfLife::countLivingNeighbors(int x, int y) const {
    int count = 0;
    
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            
            if (isAlive(x + dx, y + dy)) {
                count++;
            }
        }
    }
    
    return count;
}

bool GameOfLife::isAlive(int x, int y) const {
    if (x < 0) x = mWidth - 1;
    else if (x >= mWidth) x = 0;
    
    if (y < 0) y = mHeight - 1;
    else if (y >= mHeight) y = 0;
    
    return mGrid[y][x];
}

} // namespace GameOfLife::Server
