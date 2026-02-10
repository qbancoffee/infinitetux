/**
 * @file Mario.h
 * @brief Player character implementation.
 * @ingroup sprites
 * 
 * Mario handles player movement, power-up states,
 * collision with enemies/items, and death/win conditions.
 */
#pragma once
#include "Sprite.h"

class LevelScene;
class Enemy;
class Shell;
class BulletBill;

class Mario : public Sprite {
public:
    // Static state (shared across levels)
    static bool large;
    static bool fire;
    static int coins;
    static int lives;
    static int score;
    static std::string levelString;
    
    static void resetStatic();
    static void getCoin();
    static void get1Up();
    static void addScore(int points);
    
    // Key indices
    static constexpr int KEY_LEFT = 0;
    static constexpr int KEY_RIGHT = 1;
    static constexpr int KEY_DOWN = 2;
    static constexpr int KEY_UP = 3;
    static constexpr int KEY_JUMP = 4;
    static constexpr int KEY_SPEED = 5;
    
    bool* keys;
    int facing = 1;
    
    int xDeathPos = 0, yDeathPos = 0;
    int deathTime = 0;
    int winTime = 0;
    
    Sprite* carried = nullptr;
    
    Mario(LevelScene* world);
    
    void move() override;
    void render(SDL_Renderer* renderer, float alpha) override;
    
    void stomp(Enemy* enemy);
    void stomp(Shell* shell);
    void stomp(BulletBill* bill);
    void kick(Shell* shell);
    
    void getHurt();
    void die();
    void getFlower();
    void getMushroom();
    void getOneUp();
    
    uint8_t getKeyMask() const;
    void setKeys(uint8_t mask);
    
    // Public for collision checks
    bool wasOnGround = false;
    bool onGround = false;
    int height = 24;
    
    // Made public for test mode (normally only called internally)
    void setLarge(bool large, bool fire);

private:
    static constexpr float GROUND_INERTIA = 0.89f;
    static constexpr float AIR_INERTIA = 0.89f;
    
    float runTime = 0;
    bool mayJump = false;
    bool ducking = false;
    bool sliding = false;
    int jumpTime = 0;
    float xJumpSpeed = 0;
    float yJumpSpeed = 0;
    bool canShoot = false;
    
    int width = 4;
    
    LevelScene* world;
    int powerUpTime = 0;
    int invulnerableTime = 0;
    
    bool lastLarge = false;
    bool lastFire = false;
    bool newLarge = false;
    bool newFire = false;
    
    static Mario* instance;
    
    void blink(bool on);
    bool moveImpl(float xa, float ya);
    bool isBlocking(float _x, float _y, float xa, float ya);
    void calcPic();
    void win();
};
