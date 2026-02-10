/**
 * @file SpriteTemplate.h
 * @brief Enemy spawn point marker.
 * @ingroup sprites
 * 
 * SpriteTemplate marks positions where enemies spawn
 * when scrolled into view, enabling respawning.
 */
#pragma once
#include "Common.h"

class Sprite;
class LevelScene;

class SpriteTemplate {
public:
    int type;
    bool winged;
    bool isDead = false;
    int lastVisibleTick = -1;
    Sprite* sprite = nullptr;
    
    SpriteTemplate(int type, bool winged);
    void spawn(LevelScene* world, int x, int y, int dir);
};
