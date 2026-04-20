#ifndef ENTITY
#define ENTITY

#include "components.hpp"

class EntityState
{
public:
	virtual void Enter() = 0;
    virtual void Update(float delta_time) = 0;
    virtual void Exit() = 0;
};

#endif