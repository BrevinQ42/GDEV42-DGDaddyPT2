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

    current_state->Update(delta_time);
    HandleWallCollisions();
}

void Player::Draw() {
    DrawCircleV(position, radius, color);
    DrawText(("HP: " + std::to_string(HP)).c_str(), position.x, position.y, 50, RED);
    
    if (GetCurrentState() == &attacking){
        Rectangle rect = {position.x - radius, position.y - radius, radius * 2, radius * 2};
        DrawRectangleLinesEx(rect, 2, RED);
    }
}

void Player::HandleEntityCollision(Entity* other) {
    if (other->entity_type == "Enemy" && other->HP <= 0) return;    // avoid collisions with unalive enemies

    float distance = Vector2Distance(position, other->position);

    distance = distance - (radius + (other->max.x - other->min.x)/2.0f);

    // if key is in the dungeon and we are checking for collision with key,
    // and there is a collision,
    if (is_key_active && other->entity_type == "Key" && distance <= 0.0f)
    {
        // make key be held by player
        other->HandleEntityCollision(this);

        // collect the key
        has_key = true;

        return;
    }

    // if lock is in the dungeon and we are checking for collision with lock,
    // and there is a collision and we have a key,
    if (is_lock_active && other->entity_type == "Lock" && distance <= 0.0f && has_key)
    {
        // open the "doors"
        other->HandleEntityCollision(this);

        // make key and lock disappear
        is_key_active = false;
        is_lock_active = false;
        
        // remove key from "inventory"
        has_key = false;

        return;
    }

    if (damageTimer == 0.0f && distance <= 0.0f) {
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
    if (GetCurrentState() == &attacking && distance <= 0.0f && other->damageTimer == 0.0f) {
        other->damageQueue = PLAYER_DAMAGE_TO_ENEMY;
        other->damageTimer = 0.5f;
    }
}

void collide(Player* player, Rectangle rect) {
    Vector2 closestPoint = {Clamp(player->position.x, rect.x, rect.x + rect.width),
                                Clamp(player->position.y, rect.y, rect.y + rect.height)};

    Vector2 collisionVector = Vector2Subtract(player->position, closestPoint);
    float cvMagnitude = Vector2Length(collisionVector);

    // if distance between closest point and the player is greater than the player's radius,
    // no collision
    if (cvMagnitude > player->radius) return;

    // wall is static, so relative velocity is just the velocity of the player
    Vector2 velocityRel = player->velocity;   
    float dotProduct = Vector2DotProduct(collisionVector, velocityRel);

    // if collision normal and relative velocity are towards roughly the same direction, no collision
    if (dotProduct >= 0) return;

    // let player mass be 1.0f
    float iNum = (1 + ELASTICITY) * dotProduct;
    float iDenom = pow(cvMagnitude, 2)
        * (1.0f + 0.0f);
    float impulse = -(iNum/iDenom);

    player->position = Vector2Add(player->position, Vector2Scale(collisionVector, impulse/1.0f));
}

void Player::HandleWallCollisions() {
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

Player::Player(Vector2 pos, float rad, float spd) {
    entity_type = "Player";
    
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

    has_key = false;
}

void Player::SetState(PlayerState* state) {
    if (current_state != nullptr) {
        current_state->Exit();
    }

    current_state = state;
    current_state->Enter();
}

void Player::Reset(Vector2 new_pos) {
    velocity = Vector2Zero();
    position = new_pos;

    HP = 15.0f;
    damageTimer = 0.0f;

    SetState(&idle);

    has_key = false;
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

    player->position = Vector2Add(player->position, player->velocity);

    // if (Vector2Length(player->velocity) < 0.2f) player->velocity = Vector2Zero();
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
    player->velocity = Vector2Normalize(player->velocity);
    player->velocity = Vector2Scale(player->velocity, player->speed * 2.0f * delta_time); // Move faster while dodging
    player->position = Vector2Add(player->position, player->velocity);

    dodgeTimer -= delta_time;

    if (dodgeTimer <= 0.0f) {
        dodgeTimer = 0.0f;
        player->SetState(&player->idle);
    }
}
