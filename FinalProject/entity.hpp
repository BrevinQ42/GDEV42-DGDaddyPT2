#ifndef ENTITY
#define ENTITY

#include <list>
#include "components.hpp"

class Observer
{
public:
    virtual ~Observer() {}

    virtual void OnNotify(entt::entity entity) = 0;
};

class ScoreObserver : public Observer
{
public:
    ScoreObserver() {}

    void OnNotify(entt::entity entity);
};

class CustomerArrivalObserver : public Observer
{
    bool has_notified;
public:
    CustomerArrivalObserver()
    {
        has_notified = false;
    }

    void OnNotify(entt::entity entity);
};

class EntityState
{
public:
	virtual void Enter() = 0;
    virtual void Update(float delta_time) = 0;
    virtual void Exit() = 0;
};

#endif