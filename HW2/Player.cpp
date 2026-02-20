/*
|------------------------------------------------------|
|                 PLAYER STATE MACHINE                 |              
|------------------------------------------------------|
|   This is the file that contains definitions for     |
|   all the functions declared in the Player.hpp       |
|   file.                                              |
|                                                      |
|   All functions from the Player.hpp SHOULD be        |
|   defined here (For Now)                             |
|                                                      |
|------------------------------------------------------|
*/

#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "Player.hpp"

const int ENEMY_DAMAGE_TO_PLAYER = 2.0f;
const int PLAYER_DAMAGE_TO_ENEMY = 2.0f;

void Player::Update(float delta_time) {
    min = {position.x - radius, position.y - radius};
    max = {position.x + radius, position.y + radius};

    // Update player's HP based on whether the player has collided (damageQueue)
    // and on whether player has recently taken damage (damageTimer)
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

    std::cout << "Damage Timer: " << damageTimer << std::endl;
    std::cout << "Damage Queue: " << damageQueue << std::endl;

    current_state->Update(delta_time);
}

void Player::Draw() {
    DrawCircleV(position, radius, color);
    DrawText(("HP: " + std::to_string(HP)).c_str(), position.x, position.y, 50, RED);
    
    if (GetCurrentState() == &attacking){
        Rectangle rect = {position.x - radius, position.y - radius, radius * 2, radius * 2};
        DrawRectangleLinesEx(rect, 2, RED);
    }
}

void Player::HandleCollision(Entity* other) {
    float enemyDistance = Vector2Distance(position, other->position);

    enemyDistance = enemyDistance - (radius + (other->max.x - other->min.x)/2.0f);

    std::cout << "Enemy Distance: " << enemyDistance << std::endl;
    if (damageTimer == 0.0f && enemyDistance <= 0.0f) {
        if (GetCurrentState() == &idle || GetCurrentState() == &moving || GetCurrentState() == &attacking){
            damageQueue = ENEMY_DAMAGE_TO_PLAYER;
            damageTimer = 0.5f;
        }
        else if (GetCurrentState() == &blocking) {
            damageQueue = ENEMY_DAMAGE_TO_PLAYER / 2.0f; // Blocking reduces damage by half      
            damageTimer = 0.5f;
        }
        // No damage if dodging
    }

    // if attacking, damage enemy if not invulnerable anymore
    if (GetCurrentState() == &attacking && enemyDistance <= 0.0f && other->damageTimer == 0.0f) {
        other->damageQueue = PLAYER_DAMAGE_TO_ENEMY;
        other->damageTimer = 0.5f;
    }
}

Player::Player(Vector2 pos, float rad, float spd) {
    isPlayer = true;
    
    position = pos;
    radius = rad;
    speed = spd;

    HP = 15.0f;
    damageTimer = 0.0f;

    min = {position.x - radius, position.y - radius};
    max = {position.x + radius, position.y + radius};

    idle.player = this;
    moving.player = this;
    attacking.player = this;
    blocking.player = this;
    dodging.player = this;

    SetState(&idle);
}

void Player::SetState(PlayerState* state) {
    if (current_state != nullptr) {
        current_state->Exit();
    }

    current_state = state;
    current_state->Enter();
}

void Player::UpdateCamera(Camera2D* camera_view) {
    camera_view->target = position;

    if (WORLD_MAX.x - position.x <= WINDOW_WIDTH / 2)
        camera_view->offset.x = WINDOW_WIDTH - (WORLD_MAX.x - position.x);
    else if (position.x - WORLD_MIN.x <= WINDOW_WIDTH / 2)
        camera_view->offset.x = position.x - WORLD_MIN.x;
    else
        camera_view->offset.x = WINDOW_WIDTH / 2;

    if (WORLD_MAX.y - position.y <= WINDOW_HEIGHT / 2)
        camera_view->offset.y = WINDOW_HEIGHT - (WORLD_MAX.y - position.y);
    else if (position.y - WORLD_MIN.y <= WINDOW_HEIGHT / 2)
        camera_view->offset.y = position.y - WORLD_MIN.y;
    else
        camera_view->offset.y = WINDOW_HEIGHT / 2;
}

PlayerState* Player::GetCurrentState() {
    return current_state;
}

void PlayerIdle::Enter() {
    player->color = SKYBLUE;
}

void PlayerMoving::Enter() {
    player->color = GREEN;
}

void PlayerAttacking::Enter() {
    player->color = RED;
    attackTimer += 0.3f; // Attack lasts for 0.5 seconds
}

void PlayerBlocking::Enter() {
    player->color = BLUE;
}

void PlayerDodging::Enter() {
    player->color = YELLOW;
    dodgeTimer += 0.5f;
}

void PlayerIdle::Exit() {}

void PlayerMoving::Exit() {}

void PlayerAttacking::Exit() {}

void PlayerBlocking::Exit() {}

void PlayerDodging::Exit() {}

void PlayerIdle::Update(float delta_time) {
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D)) {
        player->SetState(&player->moving);
    } 

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        player->SetState(&player->attacking);
    }

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        player->SetState(&player->blocking);
    }
}

void PlayerMoving::Update(float delta_time) {
    if (IsKeyDown(KEY_SPACE)) {
        player->SetState(&player->dodging);
    }
    
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        player->SetState(&player->attacking);
    }

    player->velocity = Vector2Zero();

    if (IsKeyDown(KEY_W)) player->velocity.y -= player->speed * delta_time;
    if (IsKeyDown(KEY_S)) player->velocity.y += player->speed * delta_time;
    if (IsKeyDown(KEY_A)) player->velocity.x -= player->speed * delta_time;
    if (IsKeyDown(KEY_D)) player->velocity.x += player->speed * delta_time;

    Vector2 TempPosition = Vector2Add(player->position, player->velocity);

    if (TempPosition.x - player->radius <= WORLD_MIN.x){
        TempPosition.x = WORLD_MIN.x + player->radius;
        player->velocity = Vector2Negate(player->velocity);
    }

    if (TempPosition.x + player->radius >= WORLD_MAX.x){
        TempPosition.x = WORLD_MAX.x - player->radius;
        player->velocity = Vector2Negate(player->velocity);
    }

    if (TempPosition.y - player->radius <= WORLD_MIN.y){
        TempPosition.y = WORLD_MIN.y + player->radius;
        player->velocity = Vector2Negate(player->velocity);
    }

    if (TempPosition.y + player->radius >= WORLD_MAX.y){
        TempPosition.y = WORLD_MAX.y - player->radius;
        player->velocity = Vector2Negate(player->velocity);
    }

    player->position = TempPosition;

    if (Vector2Length(player->velocity) < 0.2f) player->velocity = Vector2Zero();
    // NOTE THAT YOU DO NOT HAVE TO DO PHYSICS IMPLEMENTATION
    
    if(Vector2Length(player->velocity) == 0) {
        player->SetState(&player->idle);
    }
}

void PlayerAttacking::Update(float delta_time) {
    attackTimer -= delta_time;

    if (attackTimer <= 0.0f) {
        attackTimer = 0.0f;
        player->SetState(&player->idle);
    }
}

void PlayerBlocking::Update(float delta_time) {
    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
        player->SetState(&player->idle);
    }
}

void PlayerDodging::Update(float delta_time) {
    player->position = Vector2Add(player->position, Vector2Scale(player->velocity, 2.0f)); // Move faster while dodging

    dodgeTimer -= delta_time;

    if (dodgeTimer <= 0.0f) {
        dodgeTimer = 0.0f;
        player->velocity = Vector2Zero(); 
        player->SetState(&player->idle);
    }
}
