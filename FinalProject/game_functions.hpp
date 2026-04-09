#include <raymath.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "entt.hpp"
#include "components.hpp"

const float FPS = 60;
const float TIMESTEP = 1/FPS;
const float FRICTION = 1.5f;
const float e = 0.25f;

const float GRID_SIZE = 50;
const float radius = 16.0f;
const float item_radius = 12.0f;
const float interact_range = GRID_SIZE * 1.5f;
const int fail_threshold = 3;
const float time_per_day = 180.0f;
const float head_start_time = 15.0f;

float brew_time = 15.0f;
float consume_time = 15.0f;

int day = 1;
int total_days = 5;

int customers_not_served = 0;
float total_customers_today[6] = {0, 8, 10, 12, 15, 18};
float customers_so_far = 0;

float score = 0;
float day_score = 0;

std::string button_name = "";

// TEXTURES
Texture bean;
Texture hot_coffee;
Texture coffee_tools;
Texture iced_coffee;


entt::registry registry;
entt::entity player;
entt::entity spawn_timer;

std::string drinks[4] = {"water", "espresso", "americano", "cappuccino"};
int drinks_on_menu = 2;

std::map< std::pair<std::string, std::string>, std::string > combine =
{
    {std::make_pair("empty", "water"),         "water"},
    {std::make_pair("espresso", "hot water"),  "americano"},
    {std::make_pair("espresso", "milk"),       "cappuccino"}
};

std::map<std::string, int> price =
{
    {"water", 5},
    {"espresso", 8},
    {"americano", 10},
    {"cappucino", 12},
};

std::vector<entt::entity> queue;
std::vector<entt::entity> available_tables;

void init_textures()
{
    bean = ResourceManager::GetInstance()->GetTexture("bean.png");
    hot_coffee = ResourceManager::GetInstance()->GetTexture("hot_coffee.png");
    iced_coffee = ResourceManager::GetInstance()->GetTexture("iced_coffe.png");
    coffee_tools = ResourceManager::GetInstance()->GetTexture("coffee_tools.png");
}

void init_entities(entt::registry& registry, entt::entity& player, entt::entity& spawn_timer)
{
    // player
    player = registry.create();
    registry.emplace<CircleComponent>(player, radius);
    registry.emplace<PositionComponent>(player, Vector2{8.5f * GRID_SIZE, 7.5f * GRID_SIZE});
    registry.emplace<MoveComponent>(player, Vector2Zero());
    registry.emplace<AccelerationComponent>(player, Vector2Zero());
    registry.emplace<PhysicsComponent>(player, 1.0f, 1 / 1.0f);
    registry.emplace<DirectionComponent>(player, Vector2{0.0f, 1.0f});
    registry.emplace<InteractorComponent>(player, entt::null);
    registry.emplace<HolderComponent>(player, entt::null);
    registry.emplace<ColorComponent>(player, BLUE);

    // spawn timer for customers
    spawn_timer = registry.create();
    registry.emplace<TimerComponent>(spawn_timer, head_start_time); // time before first customer

//FOR TESTING
    // counters
    entt::entity counter1 = registry.create();
    registry.emplace<SquareComponent>(counter1, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter1, Vector2{4.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter1, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter1, false, false);
    registry.emplace<TableComponent>(counter1, true);
    registry.emplace<ColorComponent>(counter1, DARKBROWN);

    entt::entity counter2 = registry.create();
    registry.emplace<SquareComponent>(counter2, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter2, Vector2{5.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter2, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter2, false, false);
    registry.emplace<TableComponent>(counter2, true);
    registry.emplace<ColorComponent>(counter2, DARKBROWN);

    entt::entity counter3 = registry.create();
    registry.emplace<SquareComponent>(counter3, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter3, Vector2{6.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter3, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter3, false, false);
    registry.emplace<TableComponent>(counter3, true);
    registry.emplace<ColorComponent>(counter3, DARKBROWN);

    entt::entity counter4 = registry.create();
    registry.emplace<SquareComponent>(counter4, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter4, Vector2{7.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter4, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter4, false, false);
    registry.emplace<TableComponent>(counter4, true);
    registry.emplace<ColorComponent>(counter4, DARKBROWN);

    entt::entity counter5 = registry.create();
    registry.emplace<SquareComponent>(counter5, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter5, Vector2{9.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter5, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter5, true, false);
    registry.emplace<TableComponent>(counter5, true);
    registry.emplace<ColorComponent>(counter5, DARKBROWN);

    entt::entity counter6 = registry.create();
    registry.emplace<SquareComponent>(counter6, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter6, Vector2{10.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter6, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter6, false, false);
    registry.emplace<TableComponent>(counter6, true);
    registry.emplace<ColorComponent>(counter6, DARKBROWN);

    entt::entity counter7 = registry.create();
    registry.emplace<SquareComponent>(counter7, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter7, Vector2{11.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter7, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter7, false, false);
    registry.emplace<TableComponent>(counter7, true);
    registry.emplace<ColorComponent>(counter7, DARKBROWN);

    // customer-side obstacles
    entt::entity chair1 = registry.create();
    registry.emplace<SquareComponent>(chair1, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair1, Vector2{7.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair1, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair1, entt::null);
    registry.emplace<ColorComponent>(chair1, BEIGE);

    entt::entity dining_table = registry.create();
    registry.emplace<SquareComponent>(dining_table, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table, Vector2{7.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table, true, false);
    registry.emplace<TableComponent>(dining_table, false);
    registry.emplace<DiningTableComponent>(dining_table, chair1);
    registry.emplace<ColorComponent>(dining_table, BROWN);

    entt::entity chair2 = registry.create();
    registry.emplace<SquareComponent>(chair2, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair2, Vector2{5.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair2, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair2, entt::null);
    registry.emplace<ColorComponent>(chair2, BEIGE);

    entt::entity dining_table2 = registry.create();
    registry.emplace<SquareComponent>(dining_table2, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table2, Vector2{5.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table2, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table2, true, false);
    registry.emplace<TableComponent>(dining_table2, false);
    registry.emplace<DiningTableComponent>(dining_table2, chair2);
    registry.emplace<ColorComponent>(dining_table2, BROWN);

    entt::entity chair3 = registry.create();
    registry.emplace<SquareComponent>(chair3, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair3, Vector2{3.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair3, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair3, entt::null);
    registry.emplace<ColorComponent>(chair3, BEIGE);

    entt::entity dining_table3 = registry.create();
    registry.emplace<SquareComponent>(dining_table3, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table3, Vector2{3.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table3, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table3, true, false);
    registry.emplace<TableComponent>(dining_table3, false);
    registry.emplace<DiningTableComponent>(dining_table3, chair3);
    registry.emplace<ColorComponent>(dining_table3, BROWN);

    entt::entity chair4 = registry.create();
    registry.emplace<SquareComponent>(chair4, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair4, Vector2{9.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair4, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair4, entt::null);
    registry.emplace<ColorComponent>(chair4, BEIGE);

    entt::entity dining_table4 = registry.create();
    registry.emplace<SquareComponent>(dining_table4, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table4, Vector2{9.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table4, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table4, true, false);
    registry.emplace<TableComponent>(dining_table4, false);
    registry.emplace<DiningTableComponent>(dining_table4, chair4);
    registry.emplace<ColorComponent>(dining_table4, BROWN);

    entt::entity chair5 = registry.create();
    registry.emplace<SquareComponent>(chair5, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair5, Vector2{11.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair5, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair5, entt::null);
    registry.emplace<ColorComponent>(chair5, BEIGE);

    entt::entity dining_table5 = registry.create();
    registry.emplace<SquareComponent>(dining_table5, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table5, Vector2{11.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table5, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table5, true, false);
    registry.emplace<TableComponent>(dining_table5, false);
    registry.emplace<DiningTableComponent>(dining_table5, chair5);
    registry.emplace<ColorComponent>(dining_table5, BROWN);

    // item
    entt::entity stack_of_cups = registry.create();
    registry.emplace<PositionComponent>(stack_of_cups, Vector2{4.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(stack_of_cups, true, false);
    registry.emplace<StackComponent>(stack_of_cups, "cup");
    registry.emplace<ColorComponent>(stack_of_cups, ORANGE);

    entt::entity coffee_machine = registry.create();
    registry.emplace<PositionComponent>(coffee_machine, Vector2{5.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(coffee_machine, true, false);
    registry.emplace<TableComponent>(coffee_machine, false);
    registry.emplace<CoffeeMachineComponent>(coffee_machine, false, false, entt::null);
    registry.emplace<TimerComponent>(coffee_machine, 0.0f);
    registry.emplace<ColorComponent>(coffee_machine, BLACK);

    entt::entity coffee_bean_container = registry.create();
    registry.emplace<PositionComponent>(coffee_bean_container, Vector2{6.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(coffee_bean_container, true, false);
    registry.emplace<StackComponent>(coffee_bean_container, "ingredient");
    registry.emplace<IngredientComponent>(coffee_bean_container, "coffee bean", false);
    registry.emplace<ColorComponent>(coffee_bean_container, PINK);

    entt::entity water_pitcher = registry.create();
    registry.emplace<PositionComponent>(water_pitcher, Vector2{7.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(water_pitcher, true, false);
    registry.emplace<HoldableComponent>(water_pitcher, false);
    registry.emplace<PlaceableComponent>(water_pitcher, counter4);
    registry.emplace<IngredientComponent>(water_pitcher, "water", true);
    registry.emplace<ColorComponent>(water_pitcher, SKYBLUE);

    entt::entity kettle = registry.create();
    registry.emplace<PositionComponent>(kettle, Vector2{10.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(kettle, true, false);
    registry.emplace<HoldableComponent>(kettle, false);
    registry.emplace<PlaceableComponent>(kettle, counter6);
    registry.emplace<IngredientComponent>(kettle, "hot water", true);
    registry.emplace<ColorComponent>(kettle, RED);


    entt::entity milk_jug = registry.create();
    registry.emplace<PositionComponent>(milk_jug, Vector2{11.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(milk_jug, true, false);
    registry.emplace<HoldableComponent>(milk_jug, false);
    registry.emplace<PlaceableComponent>(milk_jug, counter7);
    registry.emplace<IngredientComponent>(milk_jug, "milk", true);
    registry.emplace<ColorComponent>(milk_jug, WHITE);
}

void reserve_memory()
{
    queue.reserve(total_customers_today[5]);
    available_tables.reserve(5);
}

void read_player_input(entt::registry& registry, entt::entity& player)
{
    //MOVEMENT
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

    AccelerationComponent& a = registry.get<AccelerationComponent>(player);
    PhysicsComponent& p1_phy = registry.get<PhysicsComponent>(player);
    // Does Vector - Scalar multiplication with the sum of all forces and the inverse mass of the ball
    a.acceleration = Vector2Scale(forces, p1_phy.inverse_mass);

    if (Vector2Length(forces) > 0)
    {
        DirectionComponent& dir = registry.get<DirectionComponent>(player);
        dir.forward = Vector2Normalize(forces);
    }

    //INTERACT
    InteractorComponent& interactor = registry.get<InteractorComponent>(player);

    if(IsKeyPressed(KEY_X) && interactor.hot_item != entt::null)
    {
        MoneyComponent* payment = registry.try_get<MoneyComponent>(interactor.hot_item);
        if (payment)
        {
            // add payment to score
            score += payment->amount;
            day_score += payment->amount;

            // update table's status
            PlaceableComponent& placeable = registry.get<PlaceableComponent>(interactor.hot_item);
            TableComponent& table = registry.get<TableComponent>(placeable.table);
            table.hasItemOnTop = false;

            InteractableComponent& i = registry.get<InteractableComponent>(placeable.table);
            i.isEnabled = true;

            // update placeable's "table" to null
            placeable.table = entt::null;

            // destroy money object
            registry.destroy(interactor.hot_item);
            
            // set hot item to null
            interactor.hot_item = entt::null;
            
            return;
        }

        HolderComponent& holder = registry.get<HolderComponent>(player);

        // if there is no held item
        if (holder.held_item == entt::null)
        {
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

                // update placeable's "table" to null
                placeable.table = entt::null;

                // set hot item to null
                interactor.hot_item = entt::null;

                DrinkComponent* drink = registry.try_get<DrinkComponent>(holder.held_item);
                if (drink)
                    std::cout << "Got " << drink->name << "\n";
                else
                {
                    IngredientComponent* ingredient = registry.try_get<IngredientComponent>(holder.held_item);
                    if (ingredient)
                        std::cout << "Got " << ingredient->name << "\n";
                }
                
                return;
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

                    registry.emplace<SpriteComponent>(new_entity, hot_coffee,
                                                        std::vector<Rectangle>{
                                                            {112,0,16,16}
                                                        }, 0);

                    registry.emplace<ColorComponent>(new_entity, MAROON);

                    std::cout << "Got empty cup\n"; 
                }
                else if (stack->type == "ingredient")
                {
                    IngredientComponent& ingredient = registry.get<IngredientComponent>(interactor.hot_item);
                    registry.emplace<IngredientComponent>(new_entity, ingredient.name);

                    registry.emplace<SpriteComponent>(new_entity, bean,
                                                        std::vector<Rectangle>{
                                                            {0,0,16,16}
                                                        }, 0);

                    if (ingredient.name == "coffee bean")
                        registry.emplace<ColorComponent>(new_entity, YELLOW);
                    
                    std::cout << "Got " << ingredient.name << "\n";
                }

                // set held item to new entity
                holder.held_item = new_entity;

                // set hot item to null
                interactor.hot_item = entt::null;
                
                return;
            }
        }

        // else if there is a held item
        else
        {
            HolderComponent& holder = registry.get<HolderComponent>(player);
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

                return;
            }

            CustomerComponent* customer = registry.try_get<CustomerComponent>(interactor.hot_item);

            // else if the hot item is a customer
            if (customer)
            {
                DrinkComponent* drink = registry.try_get<DrinkComponent>(holder.held_item);

                // if holding drink and drink is the customer's order
                if (drink && drink->name == customer->order)
                {
                    // make customer not interactable
                    InteractableComponent& i = registry.get<InteractableComponent>(interactor.hot_item);
                    i.isEnabled = false;
                    i.isHot = false;

                    // put drink on customer
                    PositionComponent& drink_pos = registry.get<PositionComponent>(holder.held_item);
                    PositionComponent& customer_pos = registry.get<PositionComponent>(interactor.hot_item);
                    drink_pos.position = Vector2Add(customer_pos.position, {radius / 1.5f, radius / 1.5f});

                    customer->drink = holder.held_item;

                    // let customer eat
                    customer->state = "Eating";

                    TimerComponent& timer = registry.get<TimerComponent>(interactor.hot_item);
                    timer.time = consume_time;

                    // remove item from hands of holder
                    HoldableComponent& holdable = registry.get<HoldableComponent>(holder.held_item);
                    holdable.isHeld = false;

                    holder.held_item = entt::null;

                    // set hot item to null
                    interactor.hot_item = entt::null;

                    std::cout << "Served customer with " << customer->order << "\n";

                    return;
                }
            }

            TableComponent* table = registry.try_get<TableComponent>(interactor.hot_item);

            // else if the hot item is a table,
            if (table)
            {
                table->hasItemOnTop = true;

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

                return;
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

                    std::cout << " into " << drink->name << "\n";

                    // set hot item to null
                    interactor.hot_item = entt::null;

                    return;
                }
            }
        }
    }
}

void update_customers(entt::registry& registry)
{
    int customer_count = 0;

    auto customers = registry.view<CustomerComponent>();
    for (auto entity : customers)
    {
        customer_count++;

        CustomerComponent& customer = registry.get<CustomerComponent>(entity);

        if (customer.state == "Queuing")
        {
            if (available_tables.size() > 0)
            {
                std::cout << "There is a free table!\n";

                // assign table
                int index = GetRandomValue(0, available_tables.size());
                customer.table = available_tables[index];

                DiningTableComponent* dining_table = registry.try_get<DiningTableComponent>(available_tables[index]);

                if (!dining_table)
                {
                    std::cout << "Failed to get dining table\n";
                    continue;
                }

                std::cout << "Assigned customer to table";

                // put customer on table's chair
                PositionComponent& customer_pos = registry.get<PositionComponent>(entity);
                PositionComponent& chair_pos = registry.get<PositionComponent>(dining_table->chair1);
                customer_pos.position = chair_pos.position;

                ChairComponent& chair = registry.get<ChairComponent>(dining_table->chair1);
                chair.customer = entity;

                std::cout << ", teleported them to their seat";

                // select an order and set state to ordering
                int i = GetRandomValue(0, drinks_on_menu-1);

                std::cout << ", rng worked";

                // source: https://www.w3schools.com/cpp/cpp_exceptions.asp
                try {
                    customer.order = drinks[i];
                }
                catch (...) {
                    std::cout << ", error occurred with getting the drink\n";
                    continue;
                }
                
                customer.state = "Ordering";

                std::cout << ", and customer orders " << drinks[i] << "\n";

                // make customer interactable
                InteractableComponent& interactable = registry.get<InteractableComponent>(entity);
                interactable.isEnabled = true;

                // make table unavailable
                available_tables.erase(available_tables.begin() + index);

                std::cout << "Table not available anymore\n";

                continue;
            }

            customer.patience -= TIMESTEP;

            if (customer.patience <= 0.0f)
            {
                // remove first customer in queue
                // (they will definitely be the first to lose patience)
                // source: https://www.w3schools.com/cpp/ref_vector_erase.asp
                queue.erase(queue.begin());

                // customer leaves
                registry.destroy(entity);

                std::cout << "Customer lost patience\n";

                customers_not_served++;

                if (customers_not_served == fail_threshold)
                {
                    std::cout << "Too many customers left\n";
                    // lose
                    button_name = "Redo Day";

                    score -= day_score;
                    score -= 25;
                }
            }
        }
        else if (customer.state == "Ordering")
        {
            customer.patience -= TIMESTEP;

            if (customer.patience <= 0.0f)
            {
                DiningTableComponent& dining_table = registry.get<DiningTableComponent>(customer.table);
                ChairComponent& chair = registry.get<ChairComponent>(dining_table.chair1);

                chair.customer = entt::null;

                // customer leaves
                registry.destroy(entity);

                std::cout << "Customer lost patience\n";

                customers_not_served++;

                if (customers_not_served == fail_threshold)
                {
                    std::cout << "Too many customers left\n";
                    // lose
                    button_name = "Redo Day";

                    score -= day_score;
                    score -= 25;
                }
            }
        }
        // else, is eating
    }

    if (customer_count == 0 && customers_so_far == total_customers_today[day])
    {
        // end day / win
        if (day == total_days)
            button_name = "End Game";
        else
            button_name = "Next Day";
    }
}

void affect_velocities(entt::registry& registry)
{
    // make acceleration and friction affect velocity
    auto affect_velocity = registry.view<AccelerationComponent, PhysicsComponent>();
    for (auto entity : affect_velocity)
    {
        AccelerationComponent& a = registry.get<AccelerationComponent>(entity);
        PhysicsComponent& phy = registry.get<PhysicsComponent>(entity);
        MoveComponent& m = registry.get<MoveComponent>(entity);
        
        m.velocity = Vector2Add(m.velocity, Vector2Scale(a.acceleration, TIMESTEP));
        m.velocity = Vector2Subtract(m.velocity, Vector2Scale(m.velocity, FRICTION * phy.inverse_mass * TIMESTEP));
    }
}

void move_entities(entt::registry& registry)
{
    auto move = registry.view<MoveComponent>();
    for (auto entity : move)
    {
        MoveComponent& m = registry.get<MoveComponent>(entity);
        PositionComponent& pos = registry.get<PositionComponent>(entity);

        pos.position = Vector2Add(pos.position, Vector2Scale(m.velocity, TIMESTEP));
    }
}

void circle_rectangle_collision(entt::registry& registry, entt::entity& circle, entt::entity& rectangle)
{
    // circle components
    PhysicsComponent& c_phy = registry.get<PhysicsComponent>(circle);
    PositionComponent& c_pos = registry.get<PositionComponent>(circle);
    CircleComponent& c_rad = registry.get<CircleComponent>(circle);

    // rectangle components
    PhysicsComponent& r_phy = registry.get<PhysicsComponent>(rectangle);
    PositionComponent& r_pos = registry.get<PositionComponent>(rectangle);
    SquareComponent& r_size = registry.get<SquareComponent>(rectangle);


    // collision proper
    
    // get the point on the border that is closest to the ball
    Vector2 closestPoint = {
        Clamp(c_pos.position.x, r_pos.position.x - r_size.half_size, r_pos.position.x + r_size.half_size),
        Clamp(c_pos.position.y, r_pos.position.y - r_size.half_size, r_pos.position.y + r_size.half_size)
    };

    Vector2 collisionVector = Vector2Subtract(c_pos.position, closestPoint);
    float cvMagnitude = Vector2Length(collisionVector);

    // if distance between closest point and the ball is greater than the ball's radius,
    // no collision
    if (cvMagnitude > c_rad.radius) return;

    MoveComponent* c_m = registry.try_get<MoveComponent>(circle);
    MoveComponent* r_m = registry.try_get<MoveComponent>(rectangle);

    Vector2 velocityRel = Vector2Zero();

    // guaranteed that at least one of circle and rectangle is moving
    // (assuming it is not the circle moving)

    // if circle is moving, temporarily set relative velocity to circle's velocity
    if (c_m)
        velocityRel = c_m->velocity;

    // if rectangle is moving, subtract its velocity from vector stored in relative velocity
    if (r_m)
        velocityRel = Vector2Subtract(velocityRel, r_m->velocity);

    float dotProduct = Vector2DotProduct(collisionVector, velocityRel);

    // if collision normal and relative velocity are towards roughly the same direction, no collision
    if (dotProduct >= 0) return;

    float iNum = (1 + e) * dotProduct;
    float iDenom = pow(cvMagnitude, 2)
        * (c_phy.inverse_mass + r_phy.inverse_mass);
    float impulse = -(iNum/iDenom);

    if (c_m)
    {
        c_m->velocity = Vector2Add(c_m->velocity, 
            Vector2Scale(collisionVector, impulse/c_phy.mass) );
    }

    if (r_m)
    {
        r_m->velocity = Vector2Subtract(r_m->velocity, 
            Vector2Scale(collisionVector, impulse/r_phy.mass) );
    }
}

void handle_collisions(entt::registry& registry)
{
    // moving circle colliding with squares
    auto moving_physics = registry.view<PhysicsComponent, MoveComponent>();
    auto physics = registry.view<PhysicsComponent>();

    for (auto e1 : moving_physics)
    {
        // check for collisions with other physics objects
        for (auto e2 : physics)
        {
            // ignore if it is the same physics object
            if (e1 == e2)
                continue;

            circle_rectangle_collision(registry, e1, e2);
        }
    }
}

void get_hot_items(entt::registry& registry)
{
    auto interactors = registry.view<InteractorComponent>();
    for (auto e : interactors)
    {
        InteractorComponent& interactor = registry.get<InteractorComponent>(e);

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

        PositionComponent& pos = registry.get<PositionComponent>(e);
        DirectionComponent& dir = registry.get<DirectionComponent>(e);

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
}

void update_timers(entt::registry& registry, entt::entity& spawn_timer)
{
    auto timer = registry.view<TimerComponent>();
    for (auto entity : timer)
    {
        TimerComponent& ent_timer = registry.get<TimerComponent>(entity);

        if (!FloatEquals(ent_timer.time, 0.0f))
        {
            ent_timer.time -= TIMESTEP;

            if (ent_timer.time <= 0.0f)
            {
                ent_timer.time = 0.0f;

                if (entity == spawn_timer)
                {
                    // bring customer to queue
                    entt::entity new_customer = registry.create();
                    registry.emplace<CircleComponent>(new_customer, radius);
                    registry.emplace<PositionComponent>(new_customer, Vector2{-radius, -radius});
                    registry.emplace<MoveComponent>(new_customer, Vector2Zero());
                    registry.emplace<DirectionComponent>(new_customer, Vector2{0.0f, 1.0f});
                    registry.emplace<InteractableComponent>(new_customer, false, false);
                    registry.emplace<TimerComponent>(new_customer, 0.0f);
                    registry.emplace<CustomerComponent>(new_customer, 100.0f, "Queuing", "", entt::null, entt::null);

                    queue.push_back(new_customer);

                    customers_so_far++;

                    // set timer for next customer
                    if (total_customers_today[day] - customers_so_far > 0)
                        ent_timer.time = (time_per_day - head_start_time) / (total_customers_today[day] - customers_so_far);

                    std::cout << "Customer joined the queue\n";

                    continue;
                }

                CoffeeMachineComponent* machine = registry.try_get<CoffeeMachineComponent>(entity);
                if (machine)
                {
                    // espresso has been made
                    DrinkComponent& drink = registry.get<DrinkComponent>(machine->drink);
                    drink.name = "espresso";

                    // make the drink interactable
                    InteractableComponent& drink_in = registry.get<InteractableComponent>(machine->drink);
                    drink_in.isEnabled = true;

                    // detach it from coffee machine setup
                    machine->drink = entt::null;

                    std::cout << "Espresso ready!\n";

                    continue;
                }

                CustomerComponent* customer = registry.try_get<CustomerComponent>(entity);
                if (customer)
                {
                    TableComponent& table = registry.get<TableComponent>(customer->table);
                    table.hasItemOnTop = true;

                    InteractableComponent& i = registry.get<InteractableComponent>(customer->table);
                    i.isEnabled = false;
                    i.isHot = false;

                    PositionComponent& table_pos = registry.get<PositionComponent>(customer->table);

                    // put payment on table
                    entt::entity payment = registry.create();
                    registry.emplace<PositionComponent>(payment, table_pos.position);
                    registry.emplace<InteractableComponent>(payment, true, false);
                    registry.emplace<MoneyComponent>(payment, price[customer->order] * (1.0f + customer->patience / 100.0f));
                    registry.emplace<PlaceableComponent>(payment, customer->table);

                    registry.emplace<SpriteComponent>(payment, coffee_tools,
                                                        std::vector<Rectangle>{
                                                            {32,0,16,16}
                                                        }, 0);
                    // destroy drink
                    registry.destroy(customer->drink);

                    // remove customer from chair
                    DiningTableComponent& dining_table = registry.get<DiningTableComponent>(customer->table);
                    ChairComponent& chair = registry.get<ChairComponent>(dining_table.chair1);
                    chair.customer = entt::null;

                    // destroy customer
                    registry.destroy(entity);

                    continue;
                }
            }
        }
    }
}

void draw_level(entt::registry& registry, entt::entity& player)
{
    // with sprites, do: view<sprite, __> where __ is the type of thing it is
    // (e.g. floor, object, interactable, customer, player) or smth like that

    // level layout
    for (int i = 0; i < WINDOW_WIDTH / GRID_SIZE; i++)
    {
        for (int j = 0; j < WINDOW_HEIGHT / GRID_SIZE; j++)
        {
            Vector2 position = {i * GRID_SIZE, j * GRID_SIZE};

            DrawLineV(position, Vector2Add(position, {GRID_SIZE, 0.0f}), BLACK);
            DrawLineV(position, Vector2Add(position, {0.0f, GRID_SIZE}), BLACK);
            DrawLineV(Vector2Add(position, {GRID_SIZE, GRID_SIZE}), Vector2Add(position, {GRID_SIZE, 0.0f}), BLACK);
            DrawLineV(Vector2Add(position, {GRID_SIZE, GRID_SIZE}), Vector2Add(position, {0.0f, GRID_SIZE}), BLACK);
        }
    }

    // obstacles
    available_tables.clear();

    auto obstacle = registry.view<TableComponent>();
    for (auto entity : obstacle)
    {
        CoffeeMachineComponent* machine = registry.try_get<CoffeeMachineComponent>(entity);
        if (machine) continue;

        PositionComponent& p = registry.get<PositionComponent>(entity);
        InteractableComponent& item = registry.get<InteractableComponent>(entity);
        SquareComponent& square = registry.get<SquareComponent>(entity);
        ColorComponent& clr = registry.get<ColorComponent>(entity);

        Color color;
        if (item.isHot) color = BLUE;
        else color = clr.color;

        DrawRectangleV(Vector2Subtract(p.position, {square.half_size, square.half_size}),
                        {square.half_size * 2.0f, square.half_size * 2.0f}, color);

        DiningTableComponent* dining = registry.try_get<DiningTableComponent>(entity);
        if (dining)
        {
            ChairComponent& chair = registry.get<ChairComponent>(dining->chair1);
            TableComponent& table = registry.get<TableComponent>(entity);

            // if no customer and nothing on the table
            if (chair.customer == entt::null && !table.hasItemOnTop)
                available_tables.push_back(entity);
        }
    }

    auto chair = registry.view<ChairComponent>();
    for (auto entity : chair)
    {
        PositionComponent& p = registry.get<PositionComponent>(entity);
        SquareComponent& square = registry.get<SquareComponent>(entity);

        DrawRectangleV(Vector2Subtract(p.position, {square.half_size, square.half_size}),
                        {square.half_size * 2.0f, square.half_size * 2.0f}, BLUE);
    }

    auto machine = registry.view<CoffeeMachineComponent>();
    for (auto entity : machine)
    {
        PositionComponent& p = registry.get<PositionComponent>(entity);
        InteractableComponent& i = registry.get<InteractableComponent>(entity);

        
        ColorComponent& clr = registry.get<ColorComponent>(entity);

        Color color;
        if (i.isHot) color = BLUE;
        else color = clr.color;

        DrawRectangleV(Vector2Subtract(p.position, {GRID_SIZE * 0.375f, GRID_SIZE * 0.375f}),
                        {GRID_SIZE * 0.75f, GRID_SIZE * 0.75f}, color);
    }

    // interactables
    auto interactable = registry.view<InteractableComponent>();
    for (auto entity : interactable)
    {
        CoffeeMachineComponent* machine = registry.try_get<CoffeeMachineComponent>(entity);
        if (machine) continue;

        CustomerComponent* customer = registry.try_get<CustomerComponent>(entity);
        if (customer) continue;

        PositionComponent& p = registry.get<PositionComponent>(entity);
        InteractableComponent& item = registry.get<InteractableComponent>(entity);

        SquareComponent* square = registry.try_get<SquareComponent>(entity);
        HoldableComponent* holdable = registry.try_get<HoldableComponent>(entity);

        // if it is not an obstacle and it is not being held
        if (!square && (!holdable || !holdable->isHeld))
        {
            SpriteComponent* sprite = registry.try_get<SpriteComponent>(entity);
            if (sprite)
            {
                Rectangle rec = sprite->frames[sprite->frame_number];

                DrawTexturePro(sprite->sprite_sheet, sprite->frames[sprite->frame_number],
                                {p.position.x, p.position.y, int(item_radius), int(item_radius)},
                                {0, 0}, 0.0f, WHITE);

                sprite->frame_number = (sprite->frame_number + 1) & sprite->frames.size();

                continue;
            }

            
            ColorComponent& clr = registry.get<ColorComponent>(entity);

            Color color;
            if (item.isHot) DrawCircleV(p.position, radius / 2.0f, BLUE);
            else DrawCircleV(p.position, radius / 2.0f, clr.color);
        }
    }

    // customers
    auto customer = registry.view<CustomerComponent>();
    for (auto entity : customer)
    {
        PositionComponent& pos = registry.get<PositionComponent>(entity);
        CircleComponent& rad = registry.get<CircleComponent>(entity);
        InteractableComponent& i = registry.get<InteractableComponent>(entity);
        CustomerComponent& c = registry.get<CustomerComponent>(entity);

        if (i.isHot) DrawCircleV(pos.position, rad.radius, PURPLE);
        else DrawCircleV(pos.position, rad.radius, DARKPURPLE);

        if (c.state == "Ordering")
            DrawText(TextFormat("%s", c.order.c_str()), pos.position.x - 10, pos.position.y - 20, 20, BLACK);
    }

    // player
    PositionComponent& pos = registry.get<PositionComponent>(player);
    CircleComponent& rad = registry.get<CircleComponent>(player);
    DrawCircleV(pos.position, rad.radius, BLUE);

    //[TEMP?] draw held item
    HolderComponent& holder = registry.get<HolderComponent>(player);
    if (holder.held_item != entt::null)
    {
        ColorComponent& clr = registry.get<ColorComponent>(holder.held_item);

        DrawCircleV(Vector2Add(pos.position, {rad.radius / 1.5f, rad.radius / 1.5f}),
                    rad.radius / 2.0f, clr.color);
    }

    // score
    DrawText(TextFormat("Score: %04i",int(score)), 300, 30, 30, BLACK);
}