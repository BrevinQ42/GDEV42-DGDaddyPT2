/*
 |--------------------------|
 |      Customer Header     |
 |--------------------------|
*/

#ifndef CUSTOMER
#define CUSTOMER

#include "entity.hpp"

class Customer;

class CustomerState : public EntityState
{
public:
	Customer* customer;
};

class CustomerQueuing : public CustomerState
{
public:
	void Enter();
    void Update(float delta_time);
    void Exit();
};

class CustomerOrdering : public CustomerState
{
	bool is_assigned_to_table;
	void assign_customer_to_table();

public:
	void Enter();
    void Update(float delta_time);
    void Exit();
};

class CustomerEating : public CustomerState
{
public:
	void Enter();
    void Update(float delta_time);
    void Exit();
};

class Customer
{
	CustomerState* current_state = nullptr;
public:
	entt::entity entity;

	bool has_left = false;

	CustomerQueuing queuing;
	CustomerOrdering ordering;
	CustomerEating eating;

	Customer(std::string order);

	void Update(float delta_time);

	void SetState(CustomerState* state);

	std::string GetCurrentState();
};

#endif