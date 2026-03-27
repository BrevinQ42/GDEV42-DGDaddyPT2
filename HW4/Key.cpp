/*
|------------------------------------------------------|
|                        KEY                           |              
|------------------------------------------------------|
|   This is the file that contains definitions for     |
|   all the functions declared in the Key.hpp file.    |
|------------------------------------------------------|
*/

#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "Key.hpp"

void Key::Update(float delta_time)
{
	if (owner != nullptr) {
		position = {owner->min.x, owner->position.y};
		rect = {position.x, position.y, 16.0f, 16.0f};
		min = {position.x, position.y};
    	max = {position.x + rect.width + 8.0f, position.y + rect.height + 8.0f};
	}
	else {
		anim_timer -= delta_time;

		if (anim_timer <= 0)
		{
			anim_timer = 0.15f;
			frame_index = (frame_index + 1) % 8;
		}
	}
}

void Key::Draw()
{
	if (is_key_active)
		DrawTexturePro(sprite_sheet, frames[frame_index], rect, {0,0}, 0.0, WHITE);
}

void Key::Reset(Vector2 new_pos)
{
	owner = nullptr;
	position = new_pos;
	rect = {position.x, position.y, 16.0f, 16.0f};
    min = {position.x - rect.width / 2.0f, position.y - rect.height / 2.0f};
    max = {position.x + rect.width / 2.0f, position.y + rect.height / 2.0f};

	frame_index = 0;
	anim_timer = 0.15f;

	is_key_active = true;
}

Key::Key(Vector2 pos, Texture texture)
{
	entity_type = "Key";

	position = pos;
	rect = {position.x, position.y, 16.0f, 16.0f};
	min = {position.x - rect.width / 2.0f, position.y - rect.height / 2.0f};
	max = {position.x + rect.width / 2.0f, position.y + rect.height / 2.0f};

	sprite_sheet = texture;

	is_key_active = true;
}

void Key::HandleEntityCollision(Entity* other) {
	if (owner == nullptr)
	{
		owner = other;
		frame_index = 0;
	}
}

void Key::HandleWallCollisions() {}