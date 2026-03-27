/*
|------------------------------------------------------|
|                      LOCK                            |              
|------------------------------------------------------|
|   This is the file that contains definitions for     |
|   all the functions declared in the Lock.hpp file.   |
|------------------------------------------------------|
*/

#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "Lock.hpp"

void Lock::Update(float delta_time) {}

void Lock::Draw()
{
	if (is_lock_active)
		DrawTexturePro(sprite_sheet, {16,0,16,16}, rect, {0,0}, 0.0, WHITE);
}

void Lock::Reset(Vector2 new_pos)
{
	position = new_pos;
	rect = {position.x, position.y, 32.0f, 32.0f};
    min = {position.x, position.y};
    max = {position.x + rect.width + 8.0f, position.y + rect.height + 8.0f};

    is_lock_active = true;
}

Lock::Lock()
{
	entity_type = "Lock";
}

void Lock::HandleEntityCollision(Entity* other)
{
	// "open" doors by making them floor tiles
	grid[(int) doors[0].x][(int) doors[0].y] = 15;
	grid[(int) doors[1].x][(int) doors[1].y] = 15;
}

void Lock::HandleWallCollisions() {}