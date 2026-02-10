/**
 * @file Sparkle.h
 * @brief Animated sparkle effects.
 * @ingroup sprites
 * 
 * Sparkle is a short-lived animated particle for
 * coin collection and other effects.
 */
#pragma once
#include "Sprite.h"

class Sparkle : public Sprite {
public:
    Sparkle(int x, int y, float xa, float ya, int xPic, int yPic, int life);
    void move() override;

private:
    int life;
    int xPicStart;
};
