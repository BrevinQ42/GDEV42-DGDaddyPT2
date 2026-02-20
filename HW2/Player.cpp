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

const float WINDOW_WIDTH = 1280;
const float WINDOW_HEIGHT = 720;

const Vector2 WORLD_MIN = {-500, -500};
const Vector2 WORLD_MAX = {1300, 1100};

void Player::Update(float delta_time) {
    min = {position.x - radius, position.y - radius};
    max = {position.x + radius, position.y + radius};

    current_state->Update(delta_time);
}

void Player::Draw() {
    DrawCircleV(position, radius, color);
}

void Player::HandleCollision(Entity* other) {
    if (Vector2Distance(position, other->position) < radius) {
        isColliding = true;
    }
}

Player::Player(Vector2 pos, float rad, float spd) {
    isPlayer = true;
    
    position = pos;
    radius = rad;
    speed = spd;

    min = {position.x - radius, position.y - radius};
    max = {position.x + radius, position.y + radius};

    colliding.player = this;
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

void PlayerColliding::Enter() {
    player->color = ORANGE;
}

void PlayerIdle::Enter() {
    player->color = SKYBLUE;
}

void PlayerMoving::Enter() {
    player->color = GREEN;
}

void PlayerAttacking::Enter() {
    player->color = RED;
    attackTimer += 5.0f; // Attack lasts for 0.5 seconds
}

void PlayerBlocking::Enter() {
    player->color = BLUE;
}

void PlayerDodging::Enter() {
    player->color = YELLOW;
    dodgeTimer += 0.5f;
}

void PlayerColliding::Exit() {}

void PlayerIdle::Exit() {}

void PlayerMoving::Exit() {}

void PlayerAttacking::Exit() {}

void PlayerBlocking::Exit() {}

void PlayerDodging::Exit() {}

void PlayerColliding::Update(float delta_time) {}

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

    std::cout << "Velocity: (" << player->velocity.x << ", " << player->velocity.y << ")\n";

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
    attackTimer-= delta_time;

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
