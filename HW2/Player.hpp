/*
|------------------------------------------------------|
|                    PLAYER HEADER                     |              
|------------------------------------------------------|
|   This is the header file (.hpp) file that contains  |
|   declarations for the Player class along with the   | 
|   PlayerState class and its subclasses.              |
|                                                      |
|   Note that ONLY DECLARATIONS are made here. The     |
|   DEFINITION of all member variables and functions   |
|   are done in the PlayerStateMachine.cpp file        |
|                                                      |
|   When adding new classes to this file               |
|   (i.e. PlayerBlocking, PlayerAttacking, etc.),      |
|   Only have the class declarations and define them   |
|   later in the PlayerStateMachine.cpp file           |
|                                                      |
|------------------------------------------------------|
*/

#ifndef PLAYER
#define PLAYER

#include <raylib.h>
#include <raymath.h>

#include "Entity.hpp"

class Player;

class PlayerState : public EntityState {
public:
    Player* player;
};

class PlayerIdle : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerMoving : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerAttacking : public PlayerState {
public:
    float attackTimer = 0.0f;

    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerBlocking : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerDodging : public PlayerState {
public:
    float dodgeTimer = 0.0f;

    void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerColliding : public PlayerState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class Player : public Entity{
    PlayerState* current_state = nullptr;
public:
    float radius;

    float damageTimer = 0.0f; //stops the player from taking damage every frame
    float damageQueue = 0.0f;
    float enemyDamageQueue = 0.0f;
    
    PlayerColliding colliding;
    PlayerIdle idle;
    PlayerMoving moving;
    PlayerAttacking attacking;
    PlayerBlocking blocking;
    PlayerDodging dodging;

    Player(Vector2 pos, float rad, float spd);

    void Update(float delta_time);

    void Draw();

    void SetState(PlayerState* state);

    void HandleCollision(Entity* other);

    void UpdateCamera(Camera2D* camera_view);

    PlayerState* GetCurrentState();
};


#endif