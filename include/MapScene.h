/**
 * @file MapScene.h
 * @brief World map scene.
 * @ingroup scenes
 * 
 * MapScene shows the overworld map where Mario
 * moves between level nodes.
 */
#pragma once
#include "Scene.h"
#include "Common.h"
#include <vector>

class MapScene : public Scene {
public:
    MapScene(Game* game, long seed);
    
    void init() override;
    void tick() override;
    void render(SDL_Renderer* renderer, float alpha) override;
    
    void startMusic();
    void levelWon();

private:
    // Tile type constants
    static constexpr int TILE_GRASS = 0;
    static constexpr int TILE_WATER = 1;
    static constexpr int TILE_LEVEL = 2;
    static constexpr int TILE_ROAD = 3;
    static constexpr int TILE_DECORATION = 4;
    
    long seed;
    Random random;
    
    int tickCount = 0;
    int xMario = 0, yMario = 0;
    int xMarioA = 0, yMarioA = 0;
    int moveTime = 0;
    
    int worldNumber = 0;
    int levelId = 0;
    int farthest = 0;
    int xFarthestCap = 0;
    int yFarthestCap = 0;
    
    bool canEnterLevel = false;
    
    // Map data - generated procedurally
    std::vector<std::vector<int>> level;
    std::vector<std::vector<int>> data;
    
    void nextWorld();
    bool generateLevel();
    bool findConnection(int width, int height);
    void connect(int xSource, int ySource, int width, int height);
    void drawRoad(int x0, int y0, int x1, int y1);
    void findCaps(int width, int height);
    void travel(int x, int y, int dir, int depth);
    void tryWalking(int xd, int yd);
    int calcDistance(int x, int y, int xa, int ya);
    void drawStringDropShadow(const std::string& text, int x, int y, int c);
    bool isRoad(int x, int y);
    bool isWater(int x, int y);
};
