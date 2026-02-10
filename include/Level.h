/**
 * @file Level.h
 * @brief Level tile data container.
 * @ingroup level
 * 
 * Level stores the tile map and provides tile behavior
 * lookup. Each tile has appearance and behavior data.
 */
#pragma once
#include "Common.h"

class SpriteTemplate;

class Level {
public:
    // Tile behavior flags
    static constexpr int BIT_BLOCK_UPPER = 1 << 0;
    static constexpr int BIT_BLOCK_ALL = 1 << 1;
    static constexpr int BIT_BLOCK_LOWER = 1 << 2;
    static constexpr int BIT_SPECIAL = 1 << 3;
    static constexpr int BIT_BUMPABLE = 1 << 4;
    static constexpr int BIT_BREAKABLE = 1 << 5;
    static constexpr int BIT_PICKUPABLE = 1 << 6;
    static constexpr int BIT_ANIMATED = 1 << 7;
    
    // Tile behaviors array
    static std::array<uint8_t, 256> TILE_BEHAVIORS;
    
    int width;
    int height;
    
    std::vector<std::vector<uint8_t>> map;
    std::vector<std::vector<uint8_t>> data;
    std::vector<std::vector<SpriteTemplate*>> spriteTemplates;
    
    int xExit;
    int yExit;
    
    Level(int width, int height);
    ~Level();
    
    static bool loadBehaviors(const std::string& path);
    static bool saveBehaviors(const std::string& path);
    
    void tick();
    
    uint8_t getBlockCapped(int x, int y) const;
    uint8_t getBlock(int x, int y) const;
    void setBlock(int x, int y, uint8_t b);
    void setBlockData(int x, int y, uint8_t b);
    
    bool isBlocking(int x, int y, float xa, float ya) const;
    
    SpriteTemplate* getSpriteTemplate(int x, int y) const;
    void setSpriteTemplate(int x, int y, SpriteTemplate* spriteTemplate);
};
