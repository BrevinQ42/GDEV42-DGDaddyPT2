#include <raymath.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "player.cpp"
#include "customer.cpp"

const float FPS = 60;
const float TIMESTEP = 1/FPS;

const float FRICTION = 1.5f;
const float e = 0.25f;

const float item_radius = 12.0f;
const int fail_threshold = 3;
const float head_start_time = 25.0f;
const float customer_spawn_time = 30.0f;

Camera2D camera_view;
Vector2 max = {24 * GRID_SIZE, 24 * GRID_SIZE};
Vector2 min = Vector2Zero();

std::vector<Rectangle> tiles;
int grid[24][24];

bool is_unpaused = false;

int day = 1;
int total_days = 5;
bool is_first_run = true;

int customers_not_served = 0;
float total_customers_today[6] = {0, 4, 6, 8, 10, 12};
float customers_so_far = 0;

std::string button_name = "";

Player* player = nullptr;
entt::entity spawn_timer;

std::string drinks[4] = {"water", "espresso", "americano", "cappuccino"};
int drinks_on_menu = 2;

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
    textures[0] = ResourceManager::GetInstance()->GetTexture("bean.png");
    textures[1] = ResourceManager::GetInstance()->GetTexture("hot_coffee.png");
    textures[2] = ResourceManager::GetInstance()->GetTexture("coffee_tools.png");
    textures[3] = ResourceManager::GetInstance()->GetTexture("iced_coffee.png");
    textures[4] = ResourceManager::GetInstance()->GetTexture("player.png");
    textures[5] = ResourceManager::GetInstance()->GetTexture("kitchen.png");
    textures[6] = ResourceManager::GetInstance()->GetTexture("water.png");
    textures[7] = ResourceManager::GetInstance()->GetTexture("espresso.png");
    textures[8] = ResourceManager::GetInstance()->GetTexture("americano.png");
    textures[9] = ResourceManager::GetInstance()->GetTexture("cappuccino.png");
    textures[10] = ResourceManager::GetInstance()->GetTexture("order.png");
    textures[11] = ResourceManager::GetInstance()->GetTexture("recipes.png");
    textures[12] = ResourceManager::GetInstance()->GetTexture("pause.png");
}

void init_tilemap()
{
    tiles.reserve(10);

    std::ifstream settings("settings.ini");
    std::string line;

    while (std::getline(settings,line))
    {
        if (line[0] == '/') continue;

        tiles.emplace_back();

        // x
        int index0 = line.find(' ');
        tiles.back().x = std::stoi(line.substr(0, index0));

        // y
        std::string sub = line.substr(index0 + 1);
        int index1 = sub.find(' ');
        tiles.back().y = std::stoi(sub.substr(0, index1));

        // width
        std::string sub2 = sub.substr(index1 + 1);
        int index2 = sub2.find(' ');
        tiles.back().width = std::stoi(sub2.substr(0, index2));
        
        // height
        std::string sub3 = sub2.substr(index2 + 1);
        int index3 = sub3.find(' ');
        tiles.back().height = std::stoi(sub3.substr(0, index3));
    }

    // get random layout
    int layout_num = GetRandomValue(1, 3);

    std::cout << "== LAYOUT " << layout_num << " ==\n";

    int counter = 0; // counter to skip grid or count row number for chosen grid

    std::ifstream layouts("layout.ini");

    std::cout << "Opened file\n";

    while(std::getline(layouts,line))
    {
        if (line[0] == '/') continue;

        if (counter == 0)
        {
            // if found chosen layout, start tracking row number
            // else, start counting rows to be skipped

            if (std::stoi(line.substr(0)) == layout_num)
            {
                std::cout << "Found grid\n";
                counter = 1;
            }
            else
            {
                std::cout << "Found " << line.substr(0) << "\n";
                counter = -1;
            }
        }
        else if (counter > 0) // if has started tracking row number, fill in grid
        {
            int index = -1;
            std::string sub = line;

            for (int i = 0; i < 24; i++)
            {
                std::string sub_i = sub.substr(index + 1);
                index = sub_i.find(' ');
                grid[i][counter-1] = std::stoi(sub_i.substr(0, index));
                sub = sub_i;
            }

            counter++;
            if (counter == 25) break;
        }
        else // else if has started counting rows to be skipped, skip rows
        {
            counter--;
            if (counter == -25) counter = 0;   
        }
    }
    
    std::cout << "Configured layout\n";
}

void init_entities()
{
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 24; j++)
        {
            // skip:
            // - floor
            // - table (taken care of when creating chair)
            if (grid[i][j] == 9 || grid[i][j] == 12) continue;

            Vector2 position = {(i + 0.5f) * GRID_SIZE, (j + 0.5f) * GRID_SIZE};

            // walls
            if (grid[i][j] < 9)
            {
                Rectangle tile = tiles[ grid[i][j] ];

                if (grid[i][j] == 8)
                    tile = tiles[1];

                entt::entity wall = registry.create();
                registry.emplace<SquareComponent>(wall, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(wall, position);
                registry.emplace<PhysicsComponent>(wall, 1.0f, 0.0f);
                registry.emplace<SpriteComponent>(wall, 5, std::vector<Rectangle>{tile}, 0, Vector2{tile.width / 2, tile.height / 2});
            }

            // chair
            else if (grid[i][j] == 10)
            {
                entt::entity chair = registry.create();
                registry.emplace<SquareComponent>(chair, GRID_SIZE / 4.0f);
                registry.emplace<PositionComponent>(chair, position);
                registry.emplace<PhysicsComponent>(chair, 1.0f, 0.0f);
                registry.emplace<ChairComponent>(chair, entt::null);
                registry.emplace<SpriteComponent>(chair, 5, std::vector<Rectangle>{{432, 720, 96, 48}}, 0, Vector2{48.0f, 24.0f});

                // make its corresponding table (in the cell below it)
                entt::entity dining_table = registry.create();
                registry.emplace<SquareComponent>(dining_table, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(dining_table, Vector2Add(position, {0, GRID_SIZE}));
                registry.emplace<PhysicsComponent>(dining_table, 1.0f, 0.0f);
                registry.emplace<InteractableComponent>(dining_table, true, false);
                registry.emplace<TableComponent>(dining_table, false);
                registry.emplace<DiningTableComponent>(dining_table, chair);
                registry.emplace<SpriteComponent>(dining_table, 5, std::vector<Rectangle>{{288, 2112, 48, 96}}, 0, Vector2{24.0f, 24.0f});

                // make dining table available
                available_tables.push_back(dining_table);
            }
            
            // counter
            else if (grid[i][j] == 11)
            {
                entt::entity counter = registry.create();
                registry.emplace<SquareComponent>(counter, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(counter, position);
                registry.emplace<PhysicsComponent>(counter, 1.0f, 0.0f);
                registry.emplace<InteractableComponent>(counter, true, false);
                registry.emplace<TableComponent>(counter, false);
                registry.emplace<SpriteComponent>(counter, 5, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});
            }

            // cosmetic chair
            else if (grid[i][j] == 13)
            {
                entt::entity chair = registry.create();
                registry.emplace<SquareComponent>(chair, GRID_SIZE / 4.0f);
                registry.emplace<PositionComponent>(chair, Vector2Add(position, {0, GRID_SIZE / 4.0f}));
                registry.emplace<PhysicsComponent>(chair, 1.0f, 0.0f);
                registry.emplace<SpriteComponent>(chair, 5, std::vector<Rectangle>{{432, 720, 96, 48}}, 0, Vector2{48.0f, 24.0f});
            }

            // player
            else if (grid[i][j] == 14)
            {
                player = new Player(i, j);
            }

            // coffee bean stack
            else if (grid[i][j] == 15)
            {
                // make counter first
                entt::entity counter = registry.create();
                registry.emplace<SquareComponent>(counter, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(counter, position);
                registry.emplace<PhysicsComponent>(counter, 1.0f, 0.0f);
                registry.emplace<SpriteComponent>(counter, 5, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

                entt::entity bean_stack = registry.create();
                registry.emplace<PositionComponent>(bean_stack, position);
                registry.emplace<InteractableComponent>(bean_stack, true, false);
                registry.emplace<StackComponent>(bean_stack, "ingredient");
                registry.emplace<IngredientComponent>(bean_stack, "coffee bean", false);
                registry.emplace<SpriteComponent>(bean_stack, 0,
                    std::vector<Rectangle>{{0, 0, 16, 16}, {16, 0, 16, 16}, {32, 0, 16, 16},
                    {48, 0, 16, 16}, {64, 0, 16, 16}, {80, 0, 16, 16}, {96, 0, 16, 16}, 
                    {112, 0, 16, 16}}, 0, Vector2{24.0f, 32.0f});
            }

            // coffee machine
            else if (grid[i][j] == 16)
            {
                // make counter first
                entt::entity counter = registry.create();
                registry.emplace<SquareComponent>(counter, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(counter, position);
                registry.emplace<PhysicsComponent>(counter, 1.0f, 0.0f);
                registry.emplace<SpriteComponent>(counter, 5, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

                entt::entity coffee_machine = registry.create();
                registry.emplace<PositionComponent>(coffee_machine, position);
                registry.emplace<InteractableComponent>(coffee_machine, true, false);
                registry.emplace<TableComponent>(coffee_machine, false);
                registry.emplace<CoffeeMachineComponent>(coffee_machine, false, false, entt::null);
                registry.emplace<TimerComponent>(coffee_machine, 0.0f);
                registry.emplace<SpriteComponent>(coffee_machine, 5, 
                    std::vector<Rectangle>{{720, 1392, 48, 96}}, 0, Vector2{24.0f, 96.0f});
            }

            // stack of cups
            else if (grid[i][j] == 17)
            {
                // make counter first
                entt::entity counter = registry.create();
                registry.emplace<SquareComponent>(counter, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(counter, position);
                registry.emplace<PhysicsComponent>(counter, 1.0f, 0.0f);
                registry.emplace<SpriteComponent>(counter, 5, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

                entt::entity stack_of_cups = registry.create();
                registry.emplace<PositionComponent>(stack_of_cups, position);
                registry.emplace<InteractableComponent>(stack_of_cups, true, false);
                registry.emplace<StackComponent>(stack_of_cups, "cup");
                registry.emplace<SpriteComponent>(stack_of_cups, 5, 
                    std::vector<Rectangle>{{48, 960, 48, 48}}, 0, Vector2{24.0f, 48.0f});
            }

            // water pitcher
            else if (grid[i][j] == 18)
            {
                // make counter first
                entt::entity counter = registry.create();
                registry.emplace<SquareComponent>(counter, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(counter, position);
                registry.emplace<PhysicsComponent>(counter, 1.0f, 0.0f);
                registry.emplace<InteractableComponent>(counter, false, false);
                registry.emplace<TableComponent>(counter, true);
                registry.emplace<SpriteComponent>(counter, 5, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

                entt::entity water_pitcher = registry.create();
                registry.emplace<PositionComponent>(water_pitcher, position);
                registry.emplace<InteractableComponent>(water_pitcher, true, false);
                registry.emplace<HoldableComponent>(water_pitcher, false);
                registry.emplace<PlaceableComponent>(water_pitcher, counter);
                registry.emplace<IngredientComponent>(water_pitcher, "water", true);
                registry.emplace<SpriteComponent>(water_pitcher, 5, 
                    std::vector<Rectangle>{{672, 624, 48, 96}}, 0, Vector2{24.0f, 72.0f});
            }

            // hot water pitcher
            else if (grid[i][j] == 19)
            {
                // make counter first
                entt::entity counter = registry.create();
                registry.emplace<SquareComponent>(counter, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(counter, position);
                registry.emplace<PhysicsComponent>(counter, 1.0f, 0.0f);
                registry.emplace<InteractableComponent>(counter, true, false);
                registry.emplace<TableComponent>(counter, false);
                registry.emplace<SpriteComponent>(counter, 5, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

                if (drinks_on_menu > 2)
                {
                    if (drinks[2] == "americano" || drinks_on_menu == 4)
                    {
                        // put kettle
                        entt::entity kettle = registry.create();
                        registry.emplace<PositionComponent>(kettle, position);
                        registry.emplace<InteractableComponent>(kettle, true, false);
                        registry.emplace<HoldableComponent>(kettle, false);
                        registry.emplace<PlaceableComponent>(kettle, counter);
                        registry.emplace<IngredientComponent>(kettle, "hot water", true);
                        registry.emplace<SpriteComponent>(kettle, 5, 
                            std::vector<Rectangle>{{672, 1104, 48, 96}}, 0, Vector2{24.0f, 96.0f});

                        // update counter
                        InteractableComponent& interactable = registry.get<InteractableComponent>(counter);
                        interactable.isEnabled = false;

                        TableComponent& table = registry.get<TableComponent>(counter);
                        table.hasItemOnTop = true;
                    }
                }
            }

            // milk pitcher
            else if (grid[i][j] == 20)
            {
                // make counter first
                entt::entity counter = registry.create();
                registry.emplace<SquareComponent>(counter, GRID_SIZE / 2.0f);
                registry.emplace<PositionComponent>(counter, position);
                registry.emplace<PhysicsComponent>(counter, 1.0f, 0.0f);
                registry.emplace<InteractableComponent>(counter, true, false);
                registry.emplace<TableComponent>(counter, false);
                registry.emplace<SpriteComponent>(counter, 5, std::vector<Rectangle>{{192, 288, 48, 96}}, 0, Vector2{24.0f, 48.0f});

                if (drinks_on_menu > 2)
                {
                    if (drinks[2] == "cappuccino" || drinks_on_menu == 4)
                    {
                        // put milk jug
                        entt::entity milk_jug = registry.create();
                        registry.emplace<PositionComponent>(milk_jug, position);
                        registry.emplace<InteractableComponent>(milk_jug, true, false);
                        registry.emplace<HoldableComponent>(milk_jug, false);
                        registry.emplace<PlaceableComponent>(milk_jug, counter);
                        registry.emplace<IngredientComponent>(milk_jug, "milk", true);
                        registry.emplace<SpriteComponent>(milk_jug, 5, 
                            std::vector<Rectangle>{{720, 336, 48, 48}}, 0, Vector2{16.0f, 56.0f});

                        // update counter
                        InteractableComponent& interactable = registry.get<InteractableComponent>(counter);
                        interactable.isEnabled = false;

                        TableComponent& table = registry.get<TableComponent>(counter);
                        table.hasItemOnTop = true;
                    }
                }
            }
        }
    }

    // spawn timer for customers
    spawn_timer = registry.create();
    registry.emplace<TimerComponent>(spawn_timer, head_start_time); // time before first customer
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

void setup_camera()
{
    PositionComponent& player_pos = registry.get<PositionComponent>(player->entity);

    camera_view = {0};
    camera_view.target = player_pos.position;
    camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    camera_view.zoom = 1.0f;
}

void update_player()
{
    player->Update(TIMESTEP);

    // "CHEAT" control to auto end day (for demo)
    // if (IsKeyDown(KEY_P))
    // {
    //     if (day == total_days)
    //         button_name = "End Game";
    //     else
    //         button_name = "Next Day";
    // }

    PositionComponent& player_pos = registry.get<PositionComponent>(player->entity);
    camera_view.target = player_pos.position;

    if (max.x - player_pos.position.x <= WINDOW_WIDTH / 2)
        camera_view.offset.x = WINDOW_WIDTH - (max.x - player_pos.position.x);
    else if (player_pos.position.x - min.x <= WINDOW_WIDTH / 2)
        camera_view.offset.x = player_pos.position.x - min.x;
    else
        camera_view.offset.x = WINDOW_WIDTH / 2;

    if (max.y - player_pos.position.y <= WINDOW_HEIGHT / 2)
        camera_view.offset.y = WINDOW_HEIGHT - (max.y - player_pos.position.y);
    else if (player_pos.position.y - min.y <= WINDOW_HEIGHT / 2)
        camera_view.offset.y = player_pos.position.y - min.y;
    else
        camera_view.offset.y = WINDOW_HEIGHT / 2;
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
                registry.emplace<MoneyComponent>(payment, price[c.order] * (1.0f + c.patience / 60.0f));
                registry.emplace<PlaceableComponent>(payment, c.table);

                registry.emplace<SpriteComponent>(payment, 2,
                                                        std::vector<Rectangle>{
                                                            {32,0,16,16}
                                                        }, 0, Vector2{16.0f, 16.0f});

                MoneyComponent& money = registry.get<MoneyComponent>(payment);

                while(money.amount <= 0.0f)
                {
                    money.amount = price[c.order] * (1.0f + c.patience / 60.0f);
                }

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
                    drink_sprite.sprite_id = 7;
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

    // draw floor all over the map
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 24; j++)
        {
            // disregard borders
            if (i == 0 || j == 0 || i == 23 || j == 24) continue;

            DrawTexturePro(textures[5], tiles[9], {(i + 0.5f) * GRID_SIZE, (j + 0.5f) * GRID_SIZE, GRID_SIZE, GRID_SIZE},
                            {24, 24}, 0.0f, WHITE);
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

        DrawTexturePro(textures[s.sprite_id], src, {p.position.x, p.position.y, src.width, src.height},
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

        DrawTexturePro(textures[sprite.sprite_id], src, {p.position.x, p.position.y, src.width, src.height},
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

        HolderComponent& holder = registry.get<HolderComponent>(player->entity);
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

            DrawTexturePro(textures[s->sprite_id], src, {p.position.x, p.position.y, dest_width, dest_height},
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
            int order_index = 0;
            if (c.order == "water")
            {
                order_index = 6;
            }
            if (c.order == "espresso")
            {
                order_index = 7;
            }
            if (c.order == "americano")
            {
                order_index = 8;
            }
            if (c.order == "cappuccino")
            {
                order_index = 9;
            }

            DrawTexturePro(textures[10], {0, 0, 48, 48}, 
                    {pos.position.x - 10, pos.position.y - 20, 48, 48},
                        Vector2{24.0f, 24.0f}, 0.0f, WHITE);

            DrawTexturePro(textures[order_index], {0, 0, 48, 48}, 
                    {pos.position.x - 10, pos.position.y - 20, 48, 48},
                        Vector2{24.0f, 32.0f}, 0.0f, WHITE);
        }
    }

    // player
    PositionComponent& pos = registry.get<PositionComponent>(player->entity);
    CircleComponent& rad = registry.get<CircleComponent>(player->entity);

    SpriteComponent& s = registry.get<SpriteComponent>(player->entity);

    Rectangle src = s.frames[s.frame_number];

    DrawTexturePro(textures[s.sprite_id], src, {pos.position.x, pos.position.y, src.width, src.height},
                    s.origin, 0.0f, WHITE);

    s.frame_number = s.frame_number + 1;
    if (s.frame_number >= s.frames.size()) s.frame_number = 0;

    // draw held item
    HolderComponent& holder = registry.get<HolderComponent>(player->entity);
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

        DrawTexturePro(textures[s->sprite_id], src, {pos.position.x, pos.position.y, dest_width, dest_height},
                        s->origin, rotation, WHITE);

        s->frame_number = s->frame_number + 1;
        if (s->frame_number >= s->frames.size()) s->frame_number = 0;
    }
}