/*
 |-------------------------------|
 |      Player State Machine     |
 |-------------------------------|
*/

#include <string>
#include <map>

#include "player.hpp"

const float interact_range = GRID_SIZE * 1.5f;

std::map< std::pair<std::string, std::string>, std::string > combine =
{
    {std::make_pair("empty", "water"),         "water"},
    {std::make_pair("espresso", "hot water"),  "americano"},
    {std::make_pair("espresso", "milk"),       "cappuccino"}
};

void Player::Update(float delta_time)
{
	if (current_movement_state != nullptr)
		current_movement_state->Update(delta_time);

	if (current_holding_state != nullptr)
	{
		get_hot_item();
		current_holding_state->Update(delta_time);
	}
}

Player::Player()
{
	// player
    entity = registry.create();
    registry.emplace<CircleComponent>(entity, radius);
    registry.emplace<PositionComponent>(entity, Vector2{8.5f * GRID_SIZE, 7.5f * GRID_SIZE});
    registry.emplace<MoveComponent>(entity, Vector2Zero());
    registry.emplace<AccelerationComponent>(entity, Vector2Zero());
    registry.emplace<PhysicsComponent>(entity, 1.0f, 1 / 1.0f);
    registry.emplace<DirectionComponent>(entity, Vector2{0.0f, 1.0f});
    registry.emplace<InteractorComponent>(entity, entt::null);
    registry.emplace<HolderComponent>(entity, entt::null);

	idle.player = this;
	moving.player = this;

	roaming.player = this;
	holding_item.player = this;

	SetMovementState(&idle);
	SetHoldingState(&roaming);
}

Player::~Player()
{
	registry.destroy(entity);
}

void Player::SetMovementState(PlayerState* state)
{
	if (current_movement_state != nullptr)
		current_movement_state->Exit();

	current_movement_state = state;
	current_movement_state->Enter();
}

void Player::SetHoldingState(PlayerState* state)
{
	if (current_holding_state != nullptr)
		current_holding_state->Exit();

	current_holding_state = state;
	current_holding_state->Enter();
}

std::string Player::GetCurrentMovementState()
{
	if (current_movement_state == &idle)
		return "Idle";
	if (current_movement_state == &moving)
		return "Moving";

	return "null";
}

std::string Player::GetCurrentHoldingState()
{
	if (current_holding_state == &roaming)
		return "Roaming";
	if (current_holding_state == &holding_item)
		return "Holding Item";

	return "null";
}

void Player::get_hot_item()
{
	InteractorComponent& interactor = registry.get<InteractorComponent>(entity);

    // if there was a previous hot item, reset its status
    if (interactor.hot_item != entt::null)
    {
        InteractableComponent& i = registry.get<InteractableComponent>(interactor.hot_item);
        i.isHot = false;
        i.isEnabled = true;

        interactor.hot_item = entt::null;
    }

    float highest_dot = 0; // highest dot product = closest to forward direction of interactor
    float minDistance = -1;

    PositionComponent& pos = registry.get<PositionComponent>(entity);
    DirectionComponent& dir = registry.get<DirectionComponent>(entity);

    auto interactable = registry.view<InteractableComponent>();
    for (auto entity : interactable)
    {
        InteractableComponent& i = registry.get<InteractableComponent>(entity);
        if (!i.isEnabled) continue;

        PositionComponent& item_pos = registry.get<PositionComponent>(entity);
        Vector2 interactor_to_item = Vector2Subtract(item_pos.position, pos.position);
        float distance = Vector2Length(interactor_to_item);

        // if item is within range of interactor,
        if (distance <= interact_range)
        {
            float dotProduct = Vector2DotProduct(dir.forward, Vector2Normalize(interactor_to_item));
            
            // if item is within 90 degrees of interactor's fov and has higher dot product than the last hot item
            // (no need to check for > 0 since initial value of highest_dot is 0)
            if (dotProduct > highest_dot)
            {
                highest_dot = dotProduct;
                minDistance = distance;
                interactor.hot_item = entity;
            }

            // else if this has the same dot product as the highest so far (greater than 0), the closer will be the hot item
            else if (dotProduct == highest_dot && dotProduct > 0 && distance < minDistance)
            {
                minDistance = distance;
                interactor.hot_item = entity;                
            }
        }
    }

    // if there is a new hot item, set it to hot
    if (interactor.hot_item != entt::null)
    {
        InteractableComponent& i = registry.get<InteractableComponent>(interactor.hot_item);
        i.isHot = true;
    }
}

void PlayerIdle::Enter() {}

void PlayerMoving::Enter() {}

void PlayerRoaming::Enter() {}

void PlayerHoldingItem::Enter() {}

void PlayerIdle::Update(float delta_time)
{
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D))
		player->SetMovementState(&player->moving);
}

void PlayerMoving::Update(float delta_time)
{
	Vector2 forces = Vector2Zero(); // every frame set the forces to a 0 vector

    // Adds forces with the magnitude of 200 in the direction given by WASD inputs
    if(IsKeyDown(KEY_W)) {
        forces = Vector2Add(forces, {0, -200});
    }
    if(IsKeyDown(KEY_A)) {
        forces = Vector2Add(forces, {-200, 0});
    }
    if(IsKeyDown(KEY_S)) {
        forces = Vector2Add(forces, {0, 200});
    }
    if(IsKeyDown(KEY_D)) {
        forces = Vector2Add(forces, {200, 0});
    }

    AccelerationComponent& a = registry.get<AccelerationComponent>(player->entity);
    PhysicsComponent& p1_phy = registry.get<PhysicsComponent>(player->entity);
    
    // Does Vector - Scalar multiplication with the sum of all forces and the inverse mass of the ball
    a.acceleration = Vector2Scale(forces, p1_phy.inverse_mass);

    if (Vector2Length(forces) > 0)
    {
        DirectionComponent& dir = registry.get<DirectionComponent>(player->entity);
        dir.forward = Vector2Normalize(forces);
    }
    else
    {
    	MoveComponent& m = registry.get<MoveComponent>(player->entity);

    	if (Vector2Length(m.velocity) <= 5.0f)
    		player->SetMovementState(&player->idle);
    }
}

void PlayerRoaming::Update(float delta_time)
{
	InteractorComponent& interactor = registry.get<InteractorComponent>(player->entity);
	
	// if no hot item, dont continue processing
	if (interactor.hot_item == entt::null) return;

	// interact button
	if (IsKeyPressed(KEY_X))
	{
		MoneyComponent* payment = registry.try_get<MoneyComponent>(interactor.hot_item);
        if (payment)
        {
////////////////// OBSERVER PATTERN OPT (also for end day)
        	////// - remove from read_player_input this part
            // add payment to score
            // score += payment->amount;
            // day_score += payment->amount;
            // ....
            
            return;
        }

        HolderComponent& holder = registry.get<HolderComponent>(player->entity);
        HoldableComponent* holdable = registry.try_get<HoldableComponent>(interactor.hot_item);

        // if hot item is holdable
        if (holdable)
        {
            // set held item to hot item
            holder.held_item = interactor.hot_item;
            holdable->isHeld = true;

            // make held item not interactable and not hot
            InteractableComponent& item = registry.get<InteractableComponent>(interactor.hot_item);
            item.isEnabled = false;
            item.isHot = false;

            // update table's status
            PlaceableComponent& placeable = registry.get<PlaceableComponent>(interactor.hot_item);
            TableComponent& table = registry.get<TableComponent>(placeable.table);
            table.hasItemOnTop = false;

            InteractableComponent& i = registry.get<InteractableComponent>(placeable.table);
            i.isEnabled = true;

            // make table available if dining table
            DiningTableComponent* dining = registry.try_get<DiningTableComponent>(placeable.table);
            if (dining)
                available_tables.push_back(placeable.table);

            // update placeable's "table" to null
            placeable.table = entt::null;

            DrinkComponent* drink = registry.try_get<DrinkComponent>(holder.held_item);
            if (drink)
                std::cout << "Got " << drink->name << "\n";
            else
            {
                IngredientComponent* ingredient = registry.try_get<IngredientComponent>(holder.held_item);
                if (ingredient)
                    std::cout << "Got " << ingredient->name << "\n";
            }
            
            player->SetHoldingState(&player->holding_item);
        }

        StackComponent* stack = registry.try_get<StackComponent>(interactor.hot_item);

        // else if hot item is stack
        if (stack)
        {
            // create a new entity (an object from the stack)
            entt::entity new_entity = registry.create();

            registry.emplace<PositionComponent>(new_entity, Vector2Zero());     // position doesnt matter if held
            registry.emplace<InteractableComponent>(new_entity, false, false);  // not enabled, not hot
            registry.emplace<HoldableComponent>(new_entity, true);              // is held
            registry.emplace<PlaceableComponent>(new_entity, entt::null);       // not placed on anything

            if (stack->type == "cup")
            {
                registry.emplace<DrinkComponent>(new_entity, "empty");

                registry.emplace<SpriteComponent>(new_entity, kitchen, 
                    std::vector<Rectangle>{{48, 960, 48, 48}}, 0, Vector2{24.0f, 48.0f});
                

                std::cout << "Got empty cup\n"; 
            }
            else if (stack->type == "ingredient")
            {
                IngredientComponent& ingredient = registry.get<IngredientComponent>(interactor.hot_item);
                registry.emplace<IngredientComponent>(new_entity, ingredient.name);

                registry.emplace<SpriteComponent>(new_entity, bean,
                    std::vector<Rectangle>{{0, 0, 16, 16}, {16, 0, 16, 16}, {32, 0, 16, 16},
                    {48, 0, 16, 16}, {64, 0, 16, 16}, {80, 0, 16, 16}, {96, 0, 16, 16}, 
                    {112, 0, 16, 16}}, 0, Vector2{24.0f, 32.0f});
                
                std::cout << "Got " << ingredient.name << "\n";
            }

            // set held item to new entity
            holder.held_item = new_entity;
            
            player->SetHoldingState(&player->holding_item);
        }
	}
}

void PlayerHoldingItem::Update(float delta_time)
{
	if(IsKeyPressed(KEY_X))
	{
		InteractorComponent& interactor = registry.get<InteractorComponent>(player->entity);
		HolderComponent& holder = registry.get<HolderComponent>(player->entity);

		CoffeeMachineComponent* machine = registry.try_get<CoffeeMachineComponent>(interactor.hot_item);

		// if hot item is a coffee machine
		if (machine)
		{
		    IngredientComponent* ingredient = registry.try_get<IngredientComponent>(holder.held_item);

		    // if holding an ingredient
		    if (ingredient)
		    {
		        // if holding coffee bean / grounds and machine has no coffee yet
		        if (ingredient->name == "coffee bean" && !machine->hasCoffeeGrounds)
		        {
		            // fill machine with coffee
		            machine->hasCoffeeGrounds = true;

		            // destroy entity
		            registry.destroy(holder.held_item);

		            // remove it from the hands of holder
		            holder.held_item = entt::null;

		            std::cout << "Filled machine with coffee grounds\n";
		        }

		        // else if holding water pitcher and machine has no water yet
		        else if (ingredient->name == "water" && !machine->hasWater)
		        {
		            // fill machine with water
		            machine->hasWater = true;

		            std::cout << "Filled machine with water\n";
		        }
		    }
		    else
		    {
		        DrinkComponent* drink = registry.try_get<DrinkComponent>(holder.held_item);

		        // if held item is an empty cup, and the machine has no cup yet
		        if (drink && drink->name == "empty" && machine->drink == entt::null)
		        {
		            // set cup on coffee machine
		            PositionComponent& machine_pos = registry.get<PositionComponent>(interactor.hot_item);
		            PositionComponent& cup_pos = registry.get<PositionComponent>(holder.held_item);
		            cup_pos.position = Vector2Add(machine_pos.position, {0.0f, GRID_SIZE * 0.15f});

		            PlaceableComponent& placeable = registry.get<PlaceableComponent>(holder.held_item);
		            placeable.table = interactor.hot_item;

		            machine->drink = holder.held_item;

		            // keep cup not interactable, coffee machine interactable

		            // remove cup from hands of holder
		            HoldableComponent& holdable = registry.get<HoldableComponent>(holder.held_item);
		            holdable.isHeld = false;

		            holder.held_item = entt::null;

		            std::cout << "Placed cup in machine\n";
		        }
		    }

		    TimerComponent& timer = registry.get<TimerComponent>(interactor.hot_item);

		    // if timer has not been set, and coffee machine is all set up
		    if (FloatEquals(timer.time, 0.0f) &&
		        machine->hasCoffeeGrounds && machine->hasWater && machine->drink != entt::null)
		    {
		        // disable interactions with machine
		        InteractableComponent& i = registry.get<InteractableComponent>(interactor.hot_item);
		        i.isEnabled = false;
		        i.isHot = false;

		        // remove coffee grounds and water
		        machine->hasCoffeeGrounds = false;
		        machine->hasWater = false;

		        // set timer
		        timer.time = brew_time;

		        std::cout << "Activated coffee machine for " << timer.time << " seconds\n";
		    }

		    // set hot item to null
		    interactor.hot_item = entt::null;

		    // if not holding anything anymore, go back to roaming
		    if (holder.held_item == entt::null)
		    	player->SetHoldingState(&player->roaming);
		}

		CustomerComponent* customer = registry.try_get<CustomerComponent>(interactor.hot_item);

		// else if the hot item is a customer
		if (customer)
		{
		    DrinkComponent* drink = registry.try_get<DrinkComponent>(holder.held_item);

		    // if holding drink and drink is the customer's order
		    if (drink && drink->name == customer->order)
		    {
		        // put drink on customer
		        PositionComponent& drink_pos = registry.get<PositionComponent>(holder.held_item);
		        PositionComponent& customer_pos = registry.get<PositionComponent>(interactor.hot_item);
		        drink_pos.position = Vector2Add(customer_pos.position, {radius / 1.5f, radius / 1.5f});

		        customer->drink = holder.held_item;

		        // remove item from hands of holder
		        HoldableComponent& holdable = registry.get<HoldableComponent>(holder.held_item);
		        holdable.isHeld = false;

		        holder.held_item = entt::null;

		        // set hot item to null
		        interactor.hot_item = entt::null;

		        std::cout << "Served customer with " << customer->order << "\n";

		        player->SetHoldingState(&player->roaming);
		    }
		}

		TableComponent* table = registry.try_get<TableComponent>(interactor.hot_item);

		// else if the hot item is a table,
		if (table)
		{
		    table->hasItemOnTop = true;

		    int index = -1;

		    // make table not available
		    for (int i = 0; i < available_tables.size(); i++)
		    {
		        if (available_tables[i] == interactor.hot_item)
		        {
		            index = i;
		            break;
		        }
		    }

		    if (index > -1)
		        available_tables.erase(available_tables.begin() + index);

		    // make table not interactable
		    InteractableComponent& i = registry.get<InteractableComponent>(interactor.hot_item);
		    i.isEnabled = false;
		    i.isHot = false;

		    // set held item on top of table
		    PositionComponent& table_pos = registry.get<PositionComponent>(interactor.hot_item);
		    PositionComponent& item_pos = registry.get<PositionComponent>(holder.held_item);
		    item_pos.position = table_pos.position;

		    PlaceableComponent& placeable = registry.get<PlaceableComponent>(holder.held_item);
		    placeable.table = interactor.hot_item;

		    // make item interactable
		    InteractableComponent& item = registry.get<InteractableComponent>(holder.held_item);
		    item.isEnabled = true;

		    HoldableComponent& holdable = registry.get<HoldableComponent>(holder.held_item);
		    holdable.isHeld = false;

		    // remove item from hands of holder
		    holder.held_item = entt::null;

		    // set hot item to null
		    interactor.hot_item = entt::null;

		    player->SetHoldingState(&player->roaming);
		}

		DrinkComponent* drink = registry.try_get<DrinkComponent>(interactor.hot_item);

		// else if hot item is a drink
		if (drink)
		{
		    IngredientComponent* ingredient = registry.try_get<IngredientComponent>(holder.held_item);
		    
		    // if holding an ingredient (inside a pitcher), and
		    // if the combination of the drink and ingredient is valid / is in the map data structure
		    if ( ingredient && ingredient->isPitcher && combine.find( std::make_pair(drink->name, ingredient->name) ) != combine.end() )
		    {
		        std::cout << "Combined " << drink->name << " and " << ingredient->name;

		        // combine ingredient with drink
		        drink->name = combine[std::make_pair(drink->name, ingredient->name)];

		        // update visuals of drink
		        SpriteComponent* drink_sprite = registry.try_get<SpriteComponent>(interactor.hot_item);
		        if (drink_sprite)
		        {
		            if (drink->name == "water")
		            {
		                drink_sprite->sprite_sheet = water;
		            }
		            else if (drink->name == "americano")
		            {
		                drink_sprite->sprite_sheet = americano;
		            }
		            else if (drink->name == "cappuccino")
		            {
		                drink_sprite->sprite_sheet = cappuccino;
		            }

		            drink_sprite->frames = std::vector<Rectangle>{{0, 0, 48, 48}};
		            drink_sprite->origin = Vector2{24.0f, 48.0f};
		        }

		        std::cout << " into " << drink->name << "\n";

		        // set hot item to null
		        interactor.hot_item = entt::null;

		        return;
		    }
		}
	}
}

void PlayerIdle::Exit() {}

void PlayerMoving::Exit()
{
	MoveComponent& m = registry.get<MoveComponent>(player->entity);
	m.velocity = Vector2Zero();
}

void PlayerRoaming::Exit()
{
	InteractorComponent& interactor = registry.get<InteractorComponent>(player->entity);

	// set hot item to null
    interactor.hot_item = entt::null;
}

void PlayerHoldingItem::Exit() {}