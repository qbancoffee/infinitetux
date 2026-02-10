/**
 * @file Common.h
 * @brief Common includes, constants, and utilities.
 * @ingroup core
 * 
 * Provides:
 * - SDL2 library includes
 * - Screen configuration constants
 * - Debug/test mode flags
 * - Java-compatible Random class
 * - Utility functions (lerp)
 */
#pragma once

// Tell SDL we handle our own main() - prevents SDL2main from interfering
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <array>
#include <map>
#include <chrono>

// Global debug flag (defined in main.cpp)
extern bool g_debugMode;

// Global test mode flag (defined in main.cpp)
// When enabled: Mario is invincible (by default), time doesn't run out, debug enabled
extern bool g_testMode;

// Global test mode invincibility flag (defined in main.cpp)
// Can be toggled with ` key in test mode
extern bool g_testInvincible;

// Debug print macro
#define DEBUG_PRINT(...) do { if (g_debugMode) { printf("[DEBUG] "); printf(__VA_ARGS__); printf("\n"); fflush(stdout); } } while(0)

// Screen dimensions
constexpr int SCREEN_WIDTH = 320;
constexpr int SCREEN_HEIGHT = 240;
constexpr int WINDOW_SCALE = 2;
constexpr int TICKS_PER_SECOND = 24;

// Forward declarations
class Game;
class Scene;
class Sprite;
class Mario;
class Level;
class LevelScene;

// Utility functions
inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Java-compatible Random number generator
// Uses the same LCG algorithm as java.util.Random
class Random {
public:
    Random() {
        // Use current time as seed, similar to Java's default
        setSeed(std::chrono::system_clock::now().time_since_epoch().count());
    }
    
    Random(int64_t s) {
        setSeed(s);
    }
    
    void setSeed(int64_t s) {
        seed = (s ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
    }
    
    int next(int bits) {
        seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
        return (int)(seed >> (48 - bits));
    }
    
    int nextInt() {
        return next(32);
    }
    
    int nextInt(int bound) {
        if (bound <= 0) return 0;
        
        // Power of 2 optimization (same as Java)
        if ((bound & -bound) == bound) {
            return (int)((bound * (int64_t)next(31)) >> 31);
        }
        
        int bits, val;
        do {
            bits = next(31);
            val = bits % bound;
        } while (bits - val + (bound - 1) < 0);
        return val;
    }
    
    int64_t nextLong() {
        return ((int64_t)next(32) << 32) + next(32);
    }
    
    float nextFloat() {
        return next(24) / (float)(1 << 24);
    }
    
    double nextDouble() {
        return (((int64_t)next(26) << 27) + next(27)) / (double)(1LL << 53);
    }
    
    bool nextBoolean() {
        return next(1) != 0;
    }
    
    double nextGaussian() {
        // Box-Muller transform for Gaussian distribution
        if (haveNextGaussian) {
            haveNextGaussian = false;
            return nextNextGaussian;
        }
        double v1, v2, s;
        do {
            v1 = 2 * nextDouble() - 1;
            v2 = 2 * nextDouble() - 1;
            s = v1 * v1 + v2 * v2;
        } while (s >= 1 || s == 0);
        double multiplier = std::sqrt(-2 * std::log(s) / s);
        nextNextGaussian = v2 * multiplier;
        haveNextGaussian = true;
        return v1 * multiplier;
    }

private:
    int64_t seed;
    double nextNextGaussian = 0;
    bool haveNextGaussian = false;
};
