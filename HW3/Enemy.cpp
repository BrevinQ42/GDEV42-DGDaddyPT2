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

void Enemy::Update(float delta_time) {
    if (HP <= 0) return;

    rect = {position.x, position.y, 50.0f, 50.0f};
    min = {position.x - rect.width / 2.0f, position.y - rect.height / 2.0f};
    max = {position.x + rect.width / 2.0f, position.y + rect.height / 2.0f};

    // Update enemy's HP based on whether the enemy has been attacked by player (damageQueue)
    // and on whether enemy has recently taken damage (damageTimer)
    if (damageQueue != 0.0f) {
        if (damageTimer == 0.5f){
            HP -= damageQueue;
        }

        damageTimer -= delta_time;

        if (damageTimer <= 0.0f) {            
            damageQueue = 0.0f;
            damageTimer = 0.0f; 
        }
    }

    current_state->Update(delta_time);
    HandleWallCollisions();
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

void Enemy::HandleEntityCollision(Entity* other) {
    if(!other->isPlayer || HP <= 0) return; //only handles player for now

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

void collide(Enemy* enemy, Rectangle rect) {
    Vector2 closestPoint = {Clamp(enemy->position.x, rect.x, rect.x + rect.width),
                                Clamp(enemy->position.y, rect.y, rect.y + rect.height)};

    Vector2 collisionVector = Vector2Subtract(enemy->position, closestPoint);
    float cvMagnitude = Vector2Length(collisionVector);

    if (cvMagnitude > (enemy->max.x - enemy->min.x)/2) return;

    // wall is static, so relative velocity is just the velocity of the enemy
    Vector2 velocityRel = enemy->velocity;   
    float dotProduct = Vector2DotProduct(collisionVector, velocityRel);

    // if collision normal and relative velocity are towards roughly the same direction, no collision
    if (dotProduct >= 0) return;

    // let enemy mass be 1.0f
    float iNum = (1 + ELASTICITY) * dotProduct;
    float iDenom = pow(cvMagnitude, 2)
        * (1.0f + 0.0f);
    float impulse = -(iNum/iDenom);

    enemy->position = Vector2Add(enemy->position, Vector2Scale(collisionVector, impulse/1.0f));
}

void Enemy::HandleWallCollisions() {
    Vector2 top_left = {(float) floor(min.x / tile_size) - (WORLD_MIN.x / tile_size), (float) floor(min.y / tile_size) - (WORLD_MIN.y / tile_size)};
    Vector2 bot_right = {(float) floor(max.x / tile_size) - (WORLD_MIN.x / tile_size), (float) floor(max.y / tile_size) - (WORLD_MIN.y / tile_size)};

    // clamp
    if (top_left.x < 0) top_left.x = 0;
    if (top_left.y < 0) top_left.y = 0;
    if (bot_right.x > col_count-1) bot_right.x = col_count-1;
    if (bot_right.y > row_count-1) bot_right.y = row_count-1;

    // check if top left cell contains a wall / obstacle
    if (walls.count(grid[(int) top_left.x][(int) top_left.y]))
    {
        Rectangle rect = {top_left.x * tile_size + WORLD_MIN.x, top_left.y * tile_size + WORLD_MIN.y,
                            (float) tile_size, (float) tile_size};
        collide(this, rect);   
    }

    // if top left same as bot right, no need to check further
    if (FloatEquals(top_left.x, bot_right.x) && FloatEquals(top_left.y, bot_right.y))
        return;
    
    // else, check if bot right cell contains a wall / obstacle
    if (walls.count(grid[(int) bot_right.x][(int) bot_right.y]))
    {                
        Rectangle rect = {bot_right.x * tile_size + WORLD_MIN.x, bot_right.y * tile_size + WORLD_MIN.y,
                            (float) tile_size, (float) tile_size};
        collide(this, rect);
    }

    // if both x and y coordinates are different, check more
    if (!FloatEquals(top_left.x, bot_right.x) && !FloatEquals(top_left.y, bot_right.y))
    {        
        // check top right
        if (walls.count(grid[(int) bot_right.x][(int) top_left.y]))
        {            
            Rectangle rect = {bot_right.x * tile_size + WORLD_MIN.x, top_left.y * tile_size + WORLD_MIN.y,
                                (float) tile_size, (float) tile_size};
            collide(this, rect);
        }
                
        // check bot left
        if (walls.count(grid[(int) top_left.x][(int) bot_right.y]))
        {                    
            Rectangle rect = {top_left.x * tile_size + WORLD_MIN.x, bot_right.y * tile_size + WORLD_MIN.y,
                                (float) tile_size, (float) tile_size};
            collide(this, rect);
        }
    }
}

Enemy::Enemy() {
    HP = 4.0f;

    wandering.enemy = this;
    chasing.enemy = this;
    attacking.enemy = this;
    readyingAttack.enemy = this;

    SetState(&wandering);
}

Enemy::Enemy(Vector2 pos, float spd) {
    position = pos;
    speed = spd;

    rect = {position.x, position.y, 50.0f, 50.0f};
    min = {position.x - rect.width / 2.0f, position.y - rect.height / 2.0f};
    max = {position.x + rect.width / 2.0f, position.y + rect.height / 2.0f};

    HP = 4.0f;

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

void EnemyWandering::Enter() {
    enemy->color = YELLOW;

    enemy->isChasing = false;
    enemy->rotation = 0.0f;

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
    enemy->position = Vector2Add(enemy->position, enemy->velocity);
}

void EnemyChasing::Update(float delta_time) {
    enemy->velocity = Vector2Scale(enemy->direction, enemy->speed * delta_time);
    enemy->position = Vector2Add(enemy->position, enemy->velocity);

    if (!enemy->justChased) {
        enemy->SetState(&enemy->wandering);
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
    enemy->velocity = Vector2Scale(enemy->direction, enemy->speed * 3.0f * delta_time);
    enemy->position = Vector2Add(enemy->position, enemy->velocity);

    attackTimer -= delta_time;

    if (attackTimer <= 0.0f) {
        enemy->SetState(&enemy->wandering);
    }
}

void EnemyReadyingAttack::Update(float delta_time) {
    //no longer moving
    //continuing to rotate itself in the player's direction
    attackTimer -= delta_time;

    if (attackTimer <= 0){
        enemy->SetState(&enemy->attacking);
    }
}

