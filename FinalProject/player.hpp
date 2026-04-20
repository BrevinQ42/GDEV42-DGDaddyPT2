/*
 |------------------------|
 |      Player Header     |
 |------------------------|
*/

#ifndef PLAYER
#define PLAYER

#include "entity.hpp"

class Player;

class PlayerState : public EntityState
{
public:
	Player* player;
};

class PlayerIdle : public PlayerState
{
public:
	void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerMoving : public PlayerState
{
public:
	void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerRoaming : public PlayerState
{
public:
	void Enter();
    void Update(float delta_time);
    void Exit();
};

class PlayerHoldingItem : public PlayerState
{
public:
	void Enter();
    void Update(float delta_time);
    void Exit();
};

class Player
{
	PlayerState* current_movement_state = nullptr;
	PlayerState* current_holding_state = nullptr;
public:
	entt::entity entity;

	// Movement states
	PlayerIdle idle;
	PlayerMoving moving;

	// Holding states
	PlayerRoaming roaming; // not holding item
	PlayerHoldingItem holding_item;

	Player();
	~Player();

	void Update(float delta_time);

	void SetMovementState(PlayerState* state);
	void SetHoldingState(PlayerState* state);

	std::string GetCurrentMovementState();
	std::string GetCurrentHoldingState();

	void get_hot_item();
};

#endif