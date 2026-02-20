/*
|------------------------------------------------------|
|                 ENEMY STATE MACHINE                 |              
|------------------------------------------------------|
|   This is the file that contains definitions for     |
|   all the functions declared in the Enemy.hpp        |
|   file.                                              |
|------------------------------------------------------|
*/

#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "Enemy.hpp"

const float WINDOW_WIDTH = 1280;
const float WINDOW_HEIGHT = 720;

const Vector2 WORLD_MIN = {-500, -500};
const Vector2 WORLD_MAX = {1300, 1100};

void Enemy::Update(float delta_time) {
    rect = {position.x, position.y, 50.0f, 50.0f};
    min = {position.x - rect.width / 2.0f, position.y - rect.height / 2.0f};
    max = {position.x + rect.width / 2.0f, position.y + rect.height / 2.0f};

    current_state->Update(delta_time);
}

void Enemy::Draw() {
    if (HP > 0){
        DrawRectanglePro(rect, {rect.width / 2.0f, rect.height / 2.0f}, rotation, color);   
        DrawCircleLinesV(position, detectionRadius, YELLOW);
        DrawCircleLinesV(position, aggroRadius, ORANGE);
        DrawCircleLinesV(position, attackRadius, RED);
        DrawText(("Enemy HP: " + std::to_string(HP)).c_str(), position.x, position.y, 50, RED);
    }
}

void Enemy::HandleCollision(Entity* other) {
    if(!other->isPlayer) return; //only handles player for now

    float playerRadius = (other->max.x - other->min.x) / 2.0f;

    float playerDistance = Vector2Distance(position, other->position) - playerRadius;

    // will optimize
    if (playerDistance <= detectionRadius) {
        inDetection = true;
    }
    else {
        inDetection = false;
        justChased = false;
    }

    if (playerDistance <= aggroRadius) {
        inAggro = true;
    }
    else {
        inAggro = false;
    }

    if (playerDistance <= attackRadius) {
        inAttack = true;

    }
    else {
        inAttack = false;
    }

    if(isChasing){
        Vector2 directionToPlayer = Vector2Normalize(Vector2Subtract(other->position, position));

        rotateTowardsPlayer(directionToPlayer);
        direction = directionToPlayer;
    }
}

Enemy::Enemy(Vector2 pos, float spd) {
    position = pos;
    speed = spd;

    rect = {position.x, position.y, 50.0f, 50.0f};
    min = {position.x - rect.width / 2.0f, position.y - rect.height / 2.0f};
    max = {position.x + rect.width / 2.0f, position.y + rect.height / 2.0f};

    HP = 4.0f;

    colliding.enemy = this;
    wandering.enemy = this;
    chasing.enemy = this;
    attacking.enemy = this;
    readyingAttack.enemy = this;

    SetState(&wandering);
}

void Enemy::SetState(EnemyState* state) {
    if (current_state != nullptr) {
        current_state->Exit();
    }

    current_state = state;
    current_state->Enter();
}

EnemyState* Enemy::GetCurrentState() {
    return current_state;
}

void EnemyColliding::Enter() {
    enemy->color = BLACK;
}

void EnemyWandering::Enter() {
    enemy->color = YELLOW;

    // Initialize timer and direction
    directionTimer = (float) GetRandomValue(1, 5);
    float angle = GetRandomValue(0, 360) * DEG2RAD;
    enemy->direction = {cosf(angle), sinf(angle)};
}

void EnemyChasing::Enter() {
    enemy->color = ORANGE;
    enemy->isChasing = true;
}

void EnemyAttacking::Enter() {
    enemy->color = RED;
    attackTimer += 0.5f;
}

void EnemyReadyingAttack::Enter() {
    enemy->color = VIOLET;
    attackTimer += 1.0f;
}

void EnemyColliding::Exit() {}

void EnemyWandering::Exit() {
    directionTimer = 0.0f;
}

void EnemyChasing::Exit() {
}

void EnemyAttacking::Exit() {
    attackTimer = 0.0f;
}

void EnemyReadyingAttack::Exit() {
    attackTimer = 0.0f;
}

void EnemyColliding::Update(float delta_time) {}

void EnemyWandering::Update(float delta_time) {
    enemy->velocity = Vector2Zero();

    directionTimer -= delta_time;

    if (enemy->inAggro){
        enemy->SetState(&enemy->chasing);
        enemy->justChased = true;
    }
    else if (directionTimer <= 0.0f) {
        enemy->SetState(&enemy->wandering);
    }

    enemy->velocity = Vector2Scale(enemy->direction, enemy->speed * delta_time);

    Vector2 tempPosition = Vector2Add(enemy->position, enemy->velocity);
    if (enemy->min.x <= WORLD_MIN.x){
        tempPosition.x = WORLD_MIN.x;
        enemy->velocity = Vector2Negate(enemy->velocity);
    }

    if (enemy->max.x >= WORLD_MAX.x){
        tempPosition.x = WORLD_MAX.x - enemy->rect.width;
        enemy->velocity = Vector2Negate(enemy->velocity);
    }

    if (enemy->min.y <= WORLD_MIN.y){
        tempPosition.y = WORLD_MIN.y + enemy->rect.height;
        enemy->velocity = Vector2Negate(enemy->velocity);
    }

    if (enemy->max.y >= WORLD_MAX.y){
        tempPosition.y = WORLD_MAX.y - enemy->rect.height;
        enemy->velocity = Vector2Negate(enemy->velocity);
    }

    enemy->position = tempPosition;
}

void EnemyChasing::Update(float delta_time) {
    enemy->velocity = Vector2Scale(enemy->direction, enemy->speed * delta_time);
    enemy->position = Vector2Add(enemy->position, enemy->velocity);

    if (!enemy->justChased) {
        enemy->SetState(&enemy->wandering);

        enemy->isChasing = false;
        enemy->rotation = 0.0f;
    }
    else if (enemy->inAttack) {
        enemy->SetState(&enemy->readyingAttack);
    }
}

void Enemy::rotateTowardsPlayer(Vector2 directionToPlayer){
    float angle = atan2f(directionToPlayer.y, directionToPlayer.x) * RAD2DEG;

    rotation = angle; 
 }

void EnemyAttacking::Update(float delta_time) {
    // dash toward the last position
    // prev velocity * 3
    enemy->velocity = Vector2Scale(enemy->direction, enemy->speed * delta_time);
    enemy->position = Vector2Add(enemy->position, Vector2Scale(enemy->velocity, 3.0f));

    attackTimer -= delta_time;

    if (enemy->inAttack && attackTimer <= 0.0f){
        enemy->SetState(&enemy->readyingAttack);
    }
    
}

void EnemyReadyingAttack::Update(float delta_time) {
    //no longer moving
    //continuing to rotate itself in the player's direction
    attackTimer -= delta_time;


    if (attackTimer <= 0){
        enemy->SetState(&enemy->attacking);
    }
    else if(!enemy->inAttack && enemy->inAggro){
        enemy->SetState(&enemy->chasing);
    }


}

