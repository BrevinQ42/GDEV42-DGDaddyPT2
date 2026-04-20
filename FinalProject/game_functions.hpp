#include <raymath.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "customer.cpp"

const float FPS = 60;
const float TIMESTEP = 1/FPS;
const float FRICTION = 1.5f;
const float e = 0.25f;

const float GRID_SIZE = 48.0f;
const float item_radius = 12.0f;
const float interact_range = GRID_SIZE * 1.5f;
const int fail_threshold = 3;
const float head_start_time = 15.0f;
const float customer_spawn_time = 25.0f;

bool is_unpaused = false;

float brew_time = 15.0f;

int day = 1;
int total_days = 5;
bool is_first_run = true;

int customers_not_served = 0;
float total_customers_today[6] = {0, 4, 6, 8, 10, 12};
float customers_so_far = 0;

float score = 0;
float day_score = 0;

std::string button_name = "";

// TEXTURES
Texture bean;
Texture hot_coffee;
Texture coffee_tools;
Texture iced_coffee;
Texture user;
Texture kitchen;
Texture water;
Texture espresso;
Texture americano;
Texture cappuccino;
Texture order;
Texture recipes;

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

std::vector<Customer*> customers;

void init_textures()
{
    bean = ResourceManager::GetInstance()->GetTexture("bean.png");
    hot_coffee = ResourceManager::GetInstance()->GetTexture("hot_coffee.png");
    iced_coffee = ResourceManager::GetInstance()->GetTexture("iced_coffee.png");
    coffee_tools = ResourceManager::GetInstance()->GetTexture("coffee_tools.png");
    user = ResourceManager::GetInstance()->GetTexture("player.png");
    kitchen = ResourceManager::GetInstance()->GetTexture("kitchen.png");
    water = ResourceManager::GetInstance()->GetTexture("water.png");
    espresso = ResourceManager::GetInstance()->GetTexture("espresso.png");
    americano = ResourceManager::GetInstance()->GetTexture("americano.png");
    cappuccino = ResourceManager::GetInstance()->GetTexture("cappuccino.png");
    order = ResourceManager::GetInstance()->GetTexture("order.png");
    recipes = ResourceManager::GetInstance()->GetTexture("recipes.png");
}

void init_entities()
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
    registry.emplace<SpriteComponent>(player, user,
        std::vector<Rectangle>{{0, 0, 48, 96}, {48, 0, 48, 96}, {96, 0, 48, 96}, {144, 0, 48, 96}},
        4, Vector2{24.0f, 86.0f}); // THINK ABT ROTATING LATER

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
    registry.emplace<SpriteComponent>(counter1, kitchen, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

    entt::entity counter2 = registry.create();
    registry.emplace<SquareComponent>(counter2, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter2, Vector2{5.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter2, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter2, false, false);
    registry.emplace<TableComponent>(counter2, true);
    registry.emplace<SpriteComponent>(counter2, kitchen, std::vector<Rectangle>{{240, 288, 48, 96}}, 0,Vector2{24.0f, 48.0f});

    entt::entity counter3 = registry.create();
    registry.emplace<SquareComponent>(counter3, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter3, Vector2{6.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter3, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter3, false, false);
    registry.emplace<TableComponent>(counter3, true);
    registry.emplace<SpriteComponent>(counter3, kitchen, std::vector<Rectangle>{{240, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

    entt::entity counter4 = registry.create();
    registry.emplace<SquareComponent>(counter4, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter4, Vector2{7.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter4, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter4, false, false);
    registry.emplace<TableComponent>(counter4, true);
    registry.emplace<SpriteComponent>(counter4, kitchen, std::vector<Rectangle>{{288, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

    entt::entity counter5 = registry.create();
    registry.emplace<SquareComponent>(counter5, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter5, Vector2{9.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter5, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter5, true, false);
    registry.emplace<TableComponent>(counter5, false);
    registry.emplace<SpriteComponent>(counter5, kitchen, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

    entt::entity counter6 = registry.create();
    registry.emplace<SquareComponent>(counter6, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter6, Vector2{10.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter6, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter6, true, false);
    registry.emplace<TableComponent>(counter6, false);
    registry.emplace<SpriteComponent>(counter6, kitchen, std::vector<Rectangle>{{240, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

    entt::entity counter7 = registry.create();
    registry.emplace<SquareComponent>(counter7, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(counter7, Vector2{11.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(counter7, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(counter7, true, false);
    registry.emplace<TableComponent>(counter7, false);
    registry.emplace<SpriteComponent>(counter7, kitchen, std::vector<Rectangle>{{288, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

    // customer-side obstacles
    entt::entity chair1 = registry.create();
    registry.emplace<SquareComponent>(chair1, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair1, Vector2{7.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair1, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair1, entt::null);
    registry.emplace<SpriteComponent>(chair1, kitchen, std::vector<Rectangle>{{432, 720, 96, 48}}, 0, Vector2{48.0f, 24.0f});

    entt::entity dining_table = registry.create();
    registry.emplace<SquareComponent>(dining_table, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table, Vector2{7.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table, true, false);
    registry.emplace<TableComponent>(dining_table, false);
    registry.emplace<DiningTableComponent>(dining_table, chair1);
    registry.emplace<SpriteComponent>(dining_table, kitchen, std::vector<Rectangle>{{288, 2112, 48, 96}}, 0, Vector2{24.0f, 24.0f});

    entt::entity chair2 = registry.create();
    registry.emplace<SquareComponent>(chair2, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair2, Vector2{5.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair2, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair2, entt::null);
    registry.emplace<SpriteComponent>(chair2, kitchen, std::vector<Rectangle>{{432, 720, 96, 48}}, 0, Vector2{48.0f, 24.0f});

    entt::entity dining_table2 = registry.create();
    registry.emplace<SquareComponent>(dining_table2, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table2, Vector2{5.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table2, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table2, true, false);
    registry.emplace<TableComponent>(dining_table2, false);
    registry.emplace<DiningTableComponent>(dining_table2, chair2);
    registry.emplace<SpriteComponent>(dining_table2, kitchen, std::vector<Rectangle>{{288, 2112, 48, 96}}, 0, Vector2{24.0f, 24.0f});

    entt::entity chair3 = registry.create();
    registry.emplace<SquareComponent>(chair3, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair3, Vector2{3.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair3, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair3, entt::null);
    registry.emplace<SpriteComponent>(chair3, kitchen, std::vector<Rectangle>{{432, 720, 96, 48}}, 0, Vector2{48.0f, 24.0f});

    entt::entity dining_table3 = registry.create();
    registry.emplace<SquareComponent>(dining_table3, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table3, Vector2{3.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table3, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table3, true, false);
    registry.emplace<TableComponent>(dining_table3, false);
    registry.emplace<DiningTableComponent>(dining_table3, chair3);
    registry.emplace<SpriteComponent>(dining_table3, kitchen, std::vector<Rectangle>{{288, 2112, 48, 96}}, 0, Vector2{24.0f, 24.0f});

    entt::entity chair4 = registry.create();
    registry.emplace<SquareComponent>(chair4, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair4, Vector2{9.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair4, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair4, entt::null);
    registry.emplace<SpriteComponent>(chair4, kitchen, std::vector<Rectangle>{{432, 720, 96, 48}}, 0, Vector2{48.0f, 24.0f});

    entt::entity dining_table4 = registry.create();
    registry.emplace<SquareComponent>(dining_table4, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table4, Vector2{9.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table4, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table4, true, false);
    registry.emplace<TableComponent>(dining_table4, false);
    registry.emplace<DiningTableComponent>(dining_table4, chair4);
    registry.emplace<SpriteComponent>(dining_table4, kitchen, std::vector<Rectangle>{{288, 2112, 48, 96}}, 0, Vector2{24.0f, 24.0f});

    entt::entity chair5 = registry.create();
    registry.emplace<SquareComponent>(chair5, GRID_SIZE / 4.0f);
    registry.emplace<PositionComponent>(chair5, Vector2{11.5f * GRID_SIZE, 2.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(chair5, 1.0f, 0.0f);
    registry.emplace<ChairComponent>(chair5, entt::null);
    registry.emplace<SpriteComponent>(chair5, kitchen, std::vector<Rectangle>{{432, 720, 96, 48}}, 0, Vector2{48.0f, 24.0f});

    entt::entity dining_table5 = registry.create();
    registry.emplace<SquareComponent>(dining_table5, GRID_SIZE / 2.0f);
    registry.emplace<PositionComponent>(dining_table5, Vector2{11.5f * GRID_SIZE, 3.5f * GRID_SIZE});
    registry.emplace<PhysicsComponent>(dining_table5, 1.0f, 0.0f);
    registry.emplace<InteractableComponent>(dining_table5, true, false);
    registry.emplace<TableComponent>(dining_table5, false);
    registry.emplace<DiningTableComponent>(dining_table5, chair5);
    registry.emplace<SpriteComponent>(dining_table5, kitchen, std::vector<Rectangle>{{288, 2112, 48, 96}}, 0, Vector2{24.0f, 24.0f});

// MAKE DINING TABLES AVAILABLE
    available_tables.push_back(dining_table);
    available_tables.push_back(dining_table2);
    available_tables.push_back(dining_table3);
    available_tables.push_back(dining_table4);
    available_tables.push_back(dining_table5);

    // item
    entt::entity stack_of_cups = registry.create();
    registry.emplace<PositionComponent>(stack_of_cups, Vector2{4.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(stack_of_cups, true, false);
    registry.emplace<StackComponent>(stack_of_cups, "cup");
    registry.emplace<SpriteComponent>(stack_of_cups, kitchen, 
        std::vector<Rectangle>{{48, 960, 48, 48}}, 0, Vector2{24.0f, 48.0f});

    entt::entity coffee_machine = registry.create();
    registry.emplace<PositionComponent>(coffee_machine, Vector2{5.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(coffee_machine, true, false);
    registry.emplace<TableComponent>(coffee_machine, false);
    registry.emplace<CoffeeMachineComponent>(coffee_machine, false, false, entt::null);
    registry.emplace<TimerComponent>(coffee_machine, 0.0f);
    registry.emplace<SpriteComponent>(coffee_machine, kitchen, 
        std::vector<Rectangle>{{720, 1392, 48, 96}}, 0, Vector2{24.0f, 96.0f});
    //registry.emplace<ColorComponent>(coffee_machine, BLACK);

    entt::entity coffee_bean_container = registry.create();
    registry.emplace<PositionComponent>(coffee_bean_container, Vector2{6.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(coffee_bean_container, true, false);
    registry.emplace<StackComponent>(coffee_bean_container, "ingredient");
    registry.emplace<IngredientComponent>(coffee_bean_container, "coffee bean", false);
    registry.emplace<SpriteComponent>(coffee_bean_container, bean,
        std::vector<Rectangle>{{0, 0, 16, 16}, {16, 0, 16, 16}, {32, 0, 16, 16},
        {48, 0, 16, 16}, {64, 0, 16, 16}, {80, 0, 16, 16}, {96, 0, 16, 16}, 
        {112, 0, 16, 16}}, 0, Vector2{24.0f, 32.0f});

    entt::entity water_pitcher = registry.create();
    registry.emplace<PositionComponent>(water_pitcher, Vector2{7.5f * GRID_SIZE, 6.5f * GRID_SIZE});
    registry.emplace<InteractableComponent>(water_pitcher, true, false);
    registry.emplace<HoldableComponent>(water_pitcher, false);
    registry.emplace<PlaceableComponent>(water_pitcher, counter4);
    registry.emplace<IngredientComponent>(water_pitcher, "water", true);
    registry.emplace<SpriteComponent>(water_pitcher, kitchen, 
        std::vector<Rectangle>{{672, 624, 48, 96}}, 0, Vector2{24.0f, 72.0f});
    
    if (drinks_on_menu > 2)
    {
        if (drinks[2] == "americano" || drinks_on_menu == 4)
        {
            // put kettle
            entt::entity kettle = registry.create();
            registry.emplace<PositionComponent>(kettle, Vector2{10.5f * GRID_SIZE, 6.5f * GRID_SIZE});
            registry.emplace<InteractableComponent>(kettle, true, false);
            registry.emplace<HoldableComponent>(kettle, false);
            registry.emplace<PlaceableComponent>(kettle, counter6);
            registry.emplace<IngredientComponent>(kettle, "hot water", true);
            registry.emplace<SpriteComponent>(kettle, kitchen, 
                std::vector<Rectangle>{{672, 1104, 48, 96}}, 0, Vector2{24.0f, 96.0f});

            // update counter 6
            TableComponent& table = registry.get<TableComponent>(counter6);
            table.hasItemOnTop = true;

            InteractableComponent& i = registry.get<InteractableComponent>(counter6);
            i.isEnabled = false;
        }
        
        if (drinks[2] == "cappuccino" || drinks_on_menu == 4)
        {
            entt::entity milk_jug = registry.create();
            registry.emplace<PositionComponent>(milk_jug, Vector2{11.5f * GRID_SIZE, 6.5f * GRID_SIZE});
            registry.emplace<InteractableComponent>(milk_jug, true, false);
            registry.emplace<HoldableComponent>(milk_jug, false);
            registry.emplace<PlaceableComponent>(milk_jug, counter7);
            registry.emplace<IngredientComponent>(milk_jug, "milk", true);
            registry.emplace<SpriteComponent>(milk_jug, kitchen, 
                std::vector<Rectangle>{{720, 336, 48, 48}}, 0, Vector2{16.0f, 56.0f});

            // update counter 6
            TableComponent& table = registry.get<TableComponent>(counter7);
            table.hasItemOnTop = true;

            InteractableComponent& i = registry.get<InteractableComponent>(counter7);
            i.isEnabled = false;
        }
    }
}

void reserve_memory()
{
    if (is_first_run)
    {
        customers.reserve(total_customers_today[5]);
        queue.reserve(total_customers_today[5]);
        available_tables.reserve(5);   

        is_first_run = false;
    }
}

void read_player_input()
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

    // "CHEAT" control to auto end day (for demo)
    // if (IsKeyDown(KEY_P))
    // {
    //     if (day == total_days)
    //         button_name = "End Game";
    //     else
    //         button_name = "Next Day";
    // }

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

            // make table available
            available_tables.push_back(placeable.table);

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

                // make table available if dining table
                DiningTableComponent* dining = registry.try_get<DiningTableComponent>(placeable.table);
                if (dining)
                    available_tables.push_back(placeable.table);

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

                    return;
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
}

void update_customers()
{
    int customer_count = 0;

    for (int i = 0; i < customers.size(); i++)
    {
        if (customers[i] == nullptr) continue;

        customer_count++;

        Customer* customer = customers[i];
        customer->Update(TIMESTEP);
        
        if (customer->has_left)
        {
            if (customer->GetCurrentState() == "Eating")
            {
                CustomerComponent& c = registry.get<CustomerComponent>(customers[i]->entity);

                TableComponent& table = registry.get<TableComponent>(c.table);
                table.hasItemOnTop = true;

                PositionComponent& table_pos = registry.get<PositionComponent>(c.table);
             
                // put payment on table   
                entt::entity payment = registry.create();
                registry.emplace<PositionComponent>(payment, table_pos.position);
                registry.emplace<InteractableComponent>(payment, true, false);
                registry.emplace<MoneyComponent>(payment, price[c.order] * (1.0f + c.patience / 100.0f));
                registry.emplace<PlaceableComponent>(payment, c.table);

                registry.emplace<SpriteComponent>(payment, coffee_tools,
                                                        std::vector<Rectangle>{
                                                            {32,0,16,16}
                                                        }, 0, Vector2{16.0f, 16.0f});

                // customer leaves
                registry.destroy(customers[i]->entity);

                // delete pointer
                delete customers[i];
                customers[i] = nullptr;
            }
            else
            {
                std::cout << "Customer lost patience\n";

                // customer leaves
                registry.destroy(customers[i]->entity);

                // delete pointer
                delete customers[i];
                customers[i] = nullptr;

                customers_not_served++;

                if (customers_not_served == fail_threshold)
                {
                    std::cout << "Too many customers left\n";
                    // lose
                    button_name = "Redo Day";

                    queue.clear();

                    for (int i = 0; i < customers.size(); i++)
                    {
                        if (customers[i] != nullptr)
                        {
                            delete customers[i];
                            customers[i] = nullptr;
                        }
                    }

                    customers.clear();

                    score -= day_score;
                    score -= 25;
                }
            }
        }
    }

    if (customer_count == 0 && customers_so_far == total_customers_today[day])
    {
        customers.clear();

        // end day / win
        if (day == total_days)
            button_name = "End Game";
        else
            button_name = "Next Day";
    }
}

void affect_velocities()
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

void move_entities()
{
    auto move = registry.view<MoveComponent>();
    for (auto entity : move)
    {
        MoveComponent& m = registry.get<MoveComponent>(entity);
        PositionComponent& pos = registry.get<PositionComponent>(entity);

        pos.position = Vector2Add(pos.position, Vector2Scale(m.velocity, TIMESTEP));
    }
}

void circle_rectangle_collision(entt::entity& circle, entt::entity& rectangle)
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

void handle_collisions()
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

            circle_rectangle_collision(e1, e2);
        }
    }
}

void get_hot_items()
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

void update_timers()
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
                    std::cout << "Choosing customer's drink..\n";

                    int i = GetRandomValue(0, drinks_on_menu-1);

                    std::cout << "Spawning customer..\n";

                    // bring customer to queue
                    queue.emplace_back();
                    queue.back() = new Customer(drinks[i]);

                    std::cout << "Customer joined the queue\n";

                    customers.push_back(queue.back());

                    customers_so_far++;

                    // set timer for next customer
                    if (total_customers_today[day] - customers_so_far > 0)
                        ent_timer.time = customer_spawn_time;

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

                    SpriteComponent& drink_sprite = registry.get<SpriteComponent>(machine->drink);
                    drink_sprite.sprite_sheet = espresso;
                    drink_sprite.frames = std::vector<Rectangle>{{0, 0, 48, 48}};
                    drink_sprite.origin = Vector2{24.0f, 48.0f};

                    // detach it from coffee machine setup
                    machine->drink = entt::null;

                    std::cout << "Espresso ready!\n";

                    continue;
                }
            }
        }
    }
}

void draw_level()
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

    auto sprite = registry.view<SpriteComponent>();
    for (auto entity : sprite)
    {
        // rocky: why no work, question?!
        /* TableComponent* table = registry.try_get<TableComponent>(entity);
        ChairComponent* chair = registry.try_get<ChairComponent>(entity);
        if (!table || !chair) continue; */

        StackComponent* stack = registry.try_get<StackComponent>(entity);
        IngredientComponent* ingredient = registry.try_get<IngredientComponent>(entity);
        CoffeeMachineComponent* machine = registry.try_get<CoffeeMachineComponent>(entity);
        InteractorComponent* interactor = registry.try_get<InteractorComponent>(entity);
        if (stack || ingredient || machine || interactor) continue; 

        PositionComponent& p = registry.get<PositionComponent>(entity);
        SpriteComponent& s = registry.get<SpriteComponent>(entity);

        Rectangle src = s.frames[s.frame_number];

        DrawTexturePro(s.sprite_sheet, src, {p.position.x, p.position.y, src.width, src.height},
                        s.origin, 0.0f, WHITE);

        s.frame_number = s.frame_number + 1;
        if (s.frame_number >= s.frames.size()) s.frame_number = 0;
    }

    auto machine = registry.view<CoffeeMachineComponent>();
    for (auto entity : machine)
    {
        PositionComponent& p = registry.get<PositionComponent>(entity);
        InteractableComponent& i = registry.get<InteractableComponent>(entity);

        SpriteComponent& sprite = registry.get<SpriteComponent>(entity);

        Rectangle src = sprite.frames[sprite.frame_number];

        DrawTexturePro(sprite.sprite_sheet, src, {p.position.x, p.position.y, src.width, src.height},
                            sprite.origin, 0.0f, WHITE);
    }

    // interactables
    auto interactable = registry.view<InteractableComponent>();
    for (auto entity : interactable)
    {
        TableComponent* table = registry.try_get<TableComponent>(entity);
        if (table) continue;

        CoffeeMachineComponent* machine = registry.try_get<CoffeeMachineComponent>(entity);
        if (machine) continue;

        CustomerComponent* customer = registry.try_get<CustomerComponent>(entity);
        if (customer) continue;

        HolderComponent& holder = registry.get<HolderComponent>(player);

        if (holder.held_item == entity) continue;

        PositionComponent& p = registry.get<PositionComponent>(entity);
        InteractableComponent& item = registry.get<InteractableComponent>(entity);

        SquareComponent* square = registry.try_get<SquareComponent>(entity);
        HoldableComponent* holdable = registry.try_get<HoldableComponent>(entity);

        SpriteComponent* s = registry.try_get<SpriteComponent>(entity);

        if (s)
        {
            Rectangle src = s->frames[s->frame_number];

            float dest_width = src.width;
            float dest_height = src.height;
            float rotation = 0.0f;

            if (s->origin.x == 24.0f && s->origin.y == 32.0f) // if its the coffee bean
            {
                dest_width = 32.0f;
                dest_height = 32.0f;
                rotation = 30.0f;
            }

            DrawTexturePro(s->sprite_sheet, src, {p.position.x, p.position.y, dest_width, dest_height},
                            s->origin, rotation, WHITE);

            if (item.isHot) DrawCircleLinesV(p.position - s->origin / 2.0f, radius / 2.0f, WHITE);

            s->frame_number = s->frame_number + 1;
            if (s->frame_number >= s->frames.size()) s->frame_number = 0;
        }

        // if it is not an obstacle and it is not being held
        if (!sprite && !square && (!holdable || !holdable->isHeld))
        {            
            ColorComponent& clr = registry.get<ColorComponent>(entity);

            Color color;
            if (item.isHot) DrawCircleV(p.position, radius / 2.0f, BLUE);
            else DrawCircleV(p.position, radius / 2.0f, clr.color);
        }
    }

    // customers
    for (int i = 0; i < customers.size(); i++)
    {
        if (customers[i] == nullptr) continue;

        entt::entity entity = customers[i]->entity;

        PositionComponent& pos = registry.get<PositionComponent>(entity);
        CircleComponent& rad = registry.get<CircleComponent>(entity);
        InteractableComponent& inter = registry.get<InteractableComponent>(entity);
        CustomerComponent& c = registry.get<CustomerComponent>(entity);

        if (inter.isHot) DrawCircleV(pos.position, rad.radius, PURPLE);
        else DrawCircleV(pos.position, rad.radius, DARKPURPLE);

        if (customers[i]->GetCurrentState() == "Ordering")
        {
            Texture t;
            if (c.order == "water")
            {
                t = water;
            }
            if (c.order == "espresso")
            {
                t = espresso;
            }
            if (c.order == "americano")
            {
                t = americano;
            }
            if (c.order == "cappuccino")
            {
                t = cappuccino;
            }

            DrawTexturePro(order, {0, 0, 48, 48}, 
                    {pos.position.x - 10, pos.position.y - 20, 48, 48},
                        Vector2{24.0f, 24.0f}, 0.0f, WHITE);

            DrawTexturePro(t, {0, 0, 48, 48}, 
                    {pos.position.x - 10, pos.position.y - 20, 48, 48},
                        Vector2{24.0f, 32.0f}, 0.0f, WHITE);
        }
    }

    // player
    PositionComponent& pos = registry.get<PositionComponent>(player);
    CircleComponent& rad = registry.get<CircleComponent>(player);

    SpriteComponent& s = registry.get<SpriteComponent>(player);

    Rectangle src = s.frames[s.frame_number];

    DrawTexturePro(s.sprite_sheet, src, {pos.position.x, pos.position.y, src.width, src.height},
                    s.origin, 0.0f, WHITE);

    s.frame_number = s.frame_number + 1;
    if (s.frame_number >= s.frames.size()) s.frame_number = 0;

    // draw held item
    HolderComponent& holder = registry.get<HolderComponent>(player);
    if (holder.held_item != entt::null)
    {
        SpriteComponent* s = registry.try_get<SpriteComponent>(holder.held_item);

        Rectangle src = s->frames[s->frame_number];

        float dest_width = src.width;
        float dest_height = src.height;
        float rotation = 0.0f;

        if (s->origin.x == 24.0f && s->origin.y == 32.0f) // if its the coffee bean
        {
            dest_width = 32.0f;
            dest_height = 32.0f;
            rotation = 30.0f;
        }

        DrawTexturePro(s->sprite_sheet, src, {pos.position.x, pos.position.y, dest_width, dest_height},
                        s->origin, rotation, WHITE);

        s->frame_number = s->frame_number + 1;
        if (s->frame_number >= s->frames.size()) s->frame_number = 0;
    }

    // score
    DrawText(TextFormat("Score Today: %04i",int(day_score)), 250, 30, 30, BLACK);

    DrawText("Recipes!", 15, 632, 30, BLACK);
    DrawTexturePro(recipes, {0, 0, 768, 96}, {0, 672, 768, 96}, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}