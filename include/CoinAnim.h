/**
 * @file CoinAnim.h
 * @brief Coin pop-up animation.
 * @ingroup sprites
 * 
 * CoinAnim shows the spinning coin when Mario
 * hits a coin block.
 */
#pragma once
#include "Sprite.h"

class CoinAnim : public Sprite {
public:
    CoinAnim(int xTile, int yTile);
    void move() override;

private:
    int life = 10;
};
