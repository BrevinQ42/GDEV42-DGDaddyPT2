/*
|------------------------------------------------------|
|                     ENTITY HEADER                     |              
|------------------------------------------------------|
|   This is the header file (.hpp) file that contains  |
|   declarations for the Entity class along with the   | 
|   EntityState class and its subclasses.              |
|------------------------------------------------------|
*/

#ifndef ENTITY
#define ENTITY

#include <raylib.h>
#include <raymath.h>

class Entity;

class EntityState {
public:
    virtual ~EntityState() {};
    virtual void Enter() = 0;
    virtual void Update(float delta_time) = 0;
    virtual void Exit() = 0;
};

class Entity {
public:
    bool isPlayer = false;

    Vector2 position;
    Vector2 min;
    Vector2 max;

    Color color;
    float HP = 0.0f;

    Vector2 velocity = {0.0f, 0.0f};
    Vector2 acceleration = {0.0f, 0.0f};
    float speed = 0.0f;

    virtual ~Entity() {};

    virtual void Update(float delta_time) = 0;

    virtual void Draw() = 0;

    virtual void HandleCollision(Entity* other) = 0;
};

#endif