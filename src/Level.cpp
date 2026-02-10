/**
 * @file Level.cpp
 * @brief Level data implementation.
 */
#include "Level.h"
#include "SpriteTemplate.h"
#include <fstream>
#include <iostream>

std::array<uint8_t, 256> Level::TILE_BEHAVIORS = {};

Level::Level(int width, int height) : width(width), height(height) {
    xExit = 10;
    yExit = 10;
    
    map.resize(width);
    data.resize(width);
    spriteTemplates.resize(width);
    
    for (int x = 0; x < width; x++) {
        map[x].resize(height, 0);
        data[x].resize(height, 0);
        spriteTemplates[x].resize(height, nullptr);
    }
}

Level::~Level() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (spriteTemplates[x][y]) {
                delete spriteTemplates[x][y];
            }
        }
    }
}

bool Level::loadBehaviors(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open tile behaviors file: " << path << std::endl;
        return false;
    }
    
    file.read(reinterpret_cast<char*>(TILE_BEHAVIORS.data()), 256);
    std::streamsize bytesRead = file.gcount();
    
    if (bytesRead != 256) {
        std::cerr << "Expected 256 bytes, got " << bytesRead << std::endl;
        return false;
    }
    
    DEBUG_PRINT("Loaded tile behaviors from %s", path.c_str());
    return true;
}

bool Level::saveBehaviors(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(TILE_BEHAVIORS.data()), 256);
    return file.good();
}

void Level::tick() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (data[x][y] > 0) {
                data[x][y]--;
            }
        }
    }
}

uint8_t Level::getBlockCapped(int x, int y) const {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= width) x = width - 1;
    if (y >= height) y = height - 1;
    return map[x][y];
}

uint8_t Level::getBlock(int x, int y) const {
    if (x < 0) x = 0;
    if (y < 0) return 0;
    if (x >= width) x = width - 1;
    if (y >= height) y = height - 1;
    return map[x][y];
}

void Level::setBlock(int x, int y, uint8_t b) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    map[x][y] = b;
}

void Level::setBlockData(int x, int y, uint8_t b) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    data[x][y] = b;
}

bool Level::isBlocking(int x, int y, float xa, float ya) const {
    uint8_t block = getBlock(x, y);
    bool blocking = (TILE_BEHAVIORS[block] & BIT_BLOCK_ALL) > 0;
    blocking |= (ya > 0) && ((TILE_BEHAVIORS[block] & BIT_BLOCK_UPPER) > 0);
    blocking |= (ya < 0) && ((TILE_BEHAVIORS[block] & BIT_BLOCK_LOWER) > 0);
    return blocking;
}

SpriteTemplate* Level::getSpriteTemplate(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return nullptr;
    return spriteTemplates[x][y];
}

void Level::setSpriteTemplate(int x, int y, SpriteTemplate* spriteTemplate) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    spriteTemplates[x][y] = spriteTemplate;
}
