/**
 * @file LevelGenerator.h
 * @brief Procedural level generation.
 * @ingroup level
 * 
 * LevelGenerator creates playable levels from a seed.
 * Same seed always produces same level.
 */
#pragma once
#include "Common.h"

class Level;

class LevelGenerator {
public:
    static const int TYPE_OVERGROUND;
    static const int TYPE_UNDERGROUND;
    static const int TYPE_CASTLE;
    
    static Level* createLevel(int width, int height, long seed, int difficulty, int type);
    static Level* createBgLevel(int width, int height, bool distant, int type);

private:
    static const int ODDS_STRAIGHT;
    static const int ODDS_HILL_STRAIGHT;
    static const int ODDS_TUBES;
    static const int ODDS_JUMP;
    static const int ODDS_CANNONS;
    
    int width;
    int height;
    int type;
    int difficulty;
    int totalOdds;
    int odds[5];
    Random random;
    Level* level;
    
    LevelGenerator(int width, int height);
    Level* createLevelInternal(long seed, int difficulty, int type);
    
    int buildZone(int x, int maxLength);
    int buildStraight(int xo, int maxLength, bool safe);
    int buildHillStraight(int xo, int maxLength);
    int buildTubes(int xo, int maxLength);
    int buildJump(int xo, int maxLength);
    int buildCannons(int xo, int maxLength);
    void decorate(int x0, int x1, int floor);
    void addEnemyLine(int x0, int x1, int y);
    void fixWalls();
};
