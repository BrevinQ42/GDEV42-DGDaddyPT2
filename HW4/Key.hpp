/*
|------------------------------------------------------|
|                    KEY HEADER                        |              
|------------------------------------------------------|
|   This is the header file (.hpp) file that contains  |
|   declarations for the Key class.                    |
|------------------------------------------------------|
*/

#ifndef KEY
#define KEY

#include <raylib.h>
#include <raymath.h>

#include "Entity.hpp"

class Key : public Entity{
public:
    Texture sprite_sheet;
    Entity* owner = nullptr;

    int frame_count = 8;
    Rectangle frames[8] = {
        {0, 0, 16, 16},
        {16, 0, 16, 16},
        {32, 0, 16, 16},
        {48, 0, 16, 16},
        {0, 16, 16, 16},
        {16, 16, 16, 16},
        {32, 16, 16, 16},
        {48, 16, 16, 16}
    };
    int frame_index = 0;
    float anim_timer = 0.15f;

    Rectangle rect;

    Key(Vector2 pos, Texture texture);

    void Update(float delta_time);

    void Draw();

    void Reset(Vector2 new_pos);

    void HandleEntityCollision(Entity* other);
    
    void HandleWallCollisions();
};


#endif