/*
 |---------------------------------|
 |      Customer State Machine     |
 |---------------------------------|
*/

#include <string>
#include "customer.hpp"

std::vector<Customer*> queue;

std::string customer_notif = "";

float consume_time = 15.0f;

void Customer::Update(float delta_time)
{
	if (current_state != nullptr)
		current_state->Update(delta_time);
}

void CustomerArrivalObserver::OnNotify(entt::entity entity)
{
	if (has_notified) return;

	// add notif that a customer arrived
	CustomerComponent& c = registry.get<CustomerComponent>(entity);
	if (c.patience <= 52.5f)
	{
		customer_notif = "";
		has_notified = true;
	}
	else
		customer_notif = "A Customer Arrived!";
}

Customer::Customer(std::string order)
{
	entity = registry.create();
    registry.emplace<CircleComponent>(entity, radius);
    registry.emplace<PositionComponent>(entity, Vector2{-1000.0f, -1000.0f});
    registry.emplace<DirectionComponent>(entity, Vector2{0.0f, 1.0f});
    registry.emplace<InteractableComponent>(entity, false, false);
    registry.emplace<CustomerComponent>(entity, 0.0f, order, entt::null, entt::null);

    CustomerArrivalObserver* cao = new CustomerArrivalObserver();
	AddObserver(cao);

	queuing.customer = this;
	ordering.customer = this;
	eating.customer = this;

	SetState(&queuing);
}

void Customer::SetState(CustomerState* state)
{
	if (current_state != nullptr)
		current_state->Exit();

	current_state = state;
	current_state->Enter();
}

std::string Customer::GetCurrentState()
{
	if (current_state == &queuing)
		return "Queuing";
	if (current_state == &ordering)
		return "Ordering";
	if (current_state == &eating)
		return "Eating";

	return "null";
}

void Customer::AddObserver(Observer* o)
{
    observers.push_back(o);
}

void Customer::RemoveObserver(Observer* o)
{
	delete o;
	o = nullptr;

    observers.remove(o);
}

void Customer::Notify(entt::entity entity)
{
    for(auto o = observers.begin(); o != observers.end(); o++)
    {
        (*o)->OnNotify(entity);
    }
}

void CustomerOrdering::assign_customer_to_table()
{
	CustomerComponent& c = registry.get<CustomerComponent>(customer->entity);

	// assign table
    int index = GetRandomValue(0, available_tables.size());
    available_tables.erase(available_tables.begin() + index);

	// last checks in case tables unavailable
    DiningTableComponent* dining_table = registry.try_get<DiningTableComponent>(available_tables[index]);
    if (!dining_table) return;

    ChairComponent& chair = registry.get<ChairComponent>(dining_table->chair1);
    if (chair.customer != entt::null) return;

    TableComponent& table = registry.get<TableComponent>(available_tables[index]);
    if (table.hasItemOnTop) return;

    // passed last checks

    c.table = available_tables[index];
    chair.customer = customer->entity;

    std::cout << "Assigned customer to table";

    // put customer on table's chair
    PositionComponent& customer_pos = registry.get<PositionComponent>(customer->entity);
    PositionComponent& chair_pos = registry.get<PositionComponent>(dining_table->chair1);
    customer_pos.position = chair_pos.position;

    std::cout << ", teleported them to their seat\n";

	// make customer interactable
    InteractableComponent& interactable = registry.get<InteractableComponent>(customer->entity);
    interactable.isEnabled = true;

    std::cout << "Table not available anymore\n";

    // source: https://www.w3schools.com/cpp/ref_vector_erase.asp
    queue.erase(queue.begin());

    is_assigned_to_table = true;

    std::cout << "Customer successfully assigned to table\n";
}

void CustomerQueuing::Enter()
{
	CustomerComponent& c = registry.get<CustomerComponent>(customer->entity);
	c.patience = 60.0f;

	customer->Notify(customer->entity);
}

void CustomerOrdering::Enter()
{
	is_assigned_to_table = false;
	assign_customer_to_table();
}

void CustomerEating::Enter()
{
	// make customer not interactable
    InteractableComponent& i = registry.get<InteractableComponent>(customer->entity);
    i.isEnabled = false;
    i.isHot = false;

	// add timer for eating
	registry.emplace<TimerComponent>(customer->entity, consume_time);
}

void CustomerQueuing::Update(float delta_time)
{
	if (available_tables.size() > 0)
    {
        std::cout << "There is a free table!\n";

        customer->SetState(&customer->ordering);
    }

	CustomerComponent& c = registry.get<CustomerComponent>(customer->entity);
	c.patience -= delta_time;

	if (c.patience <= 52.5f)
		customer->Notify(customer->entity);

	if (c.patience <= 0.0f)
	{
		int index = -1;

		// remove customer in queue
		for (int i = 0; i < queue.size(); i++)
		{
			if (queue[i] == customer)
			{
				index = i;
				break;
			}
		}

		if (index > -1)
        	queue.erase(queue.begin() + index);

		customer->has_left = true;
	}
}

void CustomerOrdering::Update(float delta_time)
{
	if (!is_assigned_to_table)
	{
		assign_customer_to_table();
		return;
	}

	CustomerComponent& c = registry.get<CustomerComponent>(customer->entity);
	if (c.drink != entt::null)
	{
		customer->SetState(&customer->eating);
	}

	c.patience -= delta_time;

	if (c.patience <= 52.5f)
		customer->Notify(customer->entity);

	if (c.patience <= 0.0f)
	{	
		// remove customer from chair
		DiningTableComponent& dining_table = registry.get<DiningTableComponent>(c.table);
        ChairComponent& chair = registry.get<ChairComponent>(dining_table.chair1);

        chair.customer = entt::null;

        customer->has_left = true;
	}
}

void CustomerEating::Update(float delta_time)
{
	TimerComponent& eat_timer = registry.get<TimerComponent>(customer->entity);

	if (eat_timer.time <= 0.0f)
	{
		CustomerComponent& c = registry.get<CustomerComponent>(customer->entity);

		// make their table not interactable (since payment is on top)
        InteractableComponent& i = registry.get<InteractableComponent>(c.table);
        i.isEnabled = false;
        i.isHot = false;

        // destroy drink
        registry.destroy(c.drink);

        // remove customer from chair
        DiningTableComponent& dining_table = registry.get<DiningTableComponent>(c.table);
        ChairComponent& chair = registry.get<ChairComponent>(dining_table.chair1);
        chair.customer = entt::null;

        customer->has_left = true;
	}
}

void CustomerQueuing::Exit() {}

void CustomerOrdering::Exit() {}

void CustomerEating::Exit() {}