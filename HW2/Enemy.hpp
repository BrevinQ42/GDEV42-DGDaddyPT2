/*
|------------------------------------------------------|
|                    ENEMY HEADER                     |              
|------------------------------------------------------|
|   This is the header file (.hpp) file that contains  |
|   declarations for the Enemy class along with the   | 
|   EnemyState class and its subclasses.              |
|------------------------------------------------------|
*/

#ifndef ENEMY
#define ENEMY

#include <raylib.h>
#include <raymath.h>

#include "Entity.hpp"

class Enemy;

class EnemyState : public EntityState {
public:
    Enemy* enemy;
};

class EnemyWandering : public EnemyState {
public:
    float directionTimer = 0.0f;

    void Enter();
    void Update(float delta_time);
    void Exit();
};

class EnemyChasing : public EnemyState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class EnemyAttacking : public EnemyState {
public:
    float attackTimer = 0.0f;

    void Enter();
    void Update(float delta_time);
    void Exit();
};

class EnemyReadyingAttack : public EnemyState {
public:
    float attackTimer = 0.0f;

    void Enter();
    void Update(float delta_time);
    void Exit();
};

class EnemyColliding : public EnemyState {
public:
    void Enter();
    void Update(float delta_time);
    void Exit();
};

class Enemy : public Entity{
    EnemyState* current_state = nullptr;
public:
    Rectangle rect;

    Vector2 direction = Vector2Zero();
    float rotation = 0.0f;

    float detectionRadius = 300.0f;
    float aggroRadius = 200.0f;
    float attackRadius = 100.0f;

    bool inDetection = false;
    bool inAggro = false;
    bool inAttack = false;

    bool isChasing = false;
    bool justChased = false;
    
    EnemyColliding colliding;
    EnemyWandering wandering;
    EnemyChasing chasing;
    EnemyAttacking attacking;
    EnemyReadyingAttack readyingAttack;

    Enemy(Vector2 pos, float spd);

    void Update(float delta_time);

    void Draw();

    void HandleCollision(Entity* other);

    void SetState(EnemyState* state);

    void rotateTowardsPlayer(Vector2 directionToPlayer);

    EnemyState* GetCurrentState();
};


#endif