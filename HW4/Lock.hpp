/*
|------------------------------------------------------|
|                    LOCK HEADER                       |              
|------------------------------------------------------|
|   This is the header file (.hpp) file that contains  |
|   declarations for the Lock class.                   |
|------------------------------------------------------|
*/

#ifndef LOCK
#define LOCK

#include <raylib.h>
#include <raymath.h>

#include "Entity.hpp"

class Lock : public Entity{
public:
    Texture sprite_sheet;
    Rectangle rect;

    Vector2 doors[2];

    Lock();

    void Update(float delta_time);

    void Draw();

    void Reset(Vector2 new_pos);

    void HandleEntityCollision(Entity* other);
    
    void HandleWallCollisions();
};


#endif