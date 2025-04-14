#pragma once

#include <vector>
#include <string>
#include <random>

namespace GameOfLife::Server {

class GameOfLife {
public:
    GameOfLife(int width, int height);
public:
    void initializeRandom(float fillRatio = 0.3f);
public:
    void update();
    std::string toString() const;
private:
    int countLivingNeighbors(int x, int y) const;
    bool isAlive(int x, int y) const;
private:
    using Grid = std::vector<std::vector<bool>>;
private:
    Grid mGrid;
    int mWidth;
    int mHeight;
};

} // namespace GameOfLife::Server
