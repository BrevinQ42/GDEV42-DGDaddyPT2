#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <string>

#include "Player.cpp"
#include "Enemy.cpp"
#include "DunGen.cpp"

const float TARGET_FPS = 60.0f;
const float TIMESTEP = 1.0f / TARGET_FPS;

Texture tilemap;

int tile_count;
int tiles_loaded;
std::vector<Rectangle> tiles;

int enemy_count;
std::vector<Enemy> enemies;
Player* player;

void init_level();
void init_map();
void init_entities();
void draw_level();
void save_entities_positions();

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BenitoQueRedoble_Homework04");
    init_level();
    dungen_main(); // dungen.cpp
    init_map();

    float roomWidthInPixels = 10.0f * tile_size;
    Vector2 playerSpawn = {
        (rooms[0].x * roomWidthInPixels + roomWidthInPixels / 2.0f) + WORLD_MIN.x,
        (rooms[0].y * roomWidthInPixels + roomWidthInPixels / 2.0f) + WORLD_MIN.y
    };

    Vector2 bossSpawn = {
        (rooms[boss_room_index].x * roomWidthInPixels + roomWidthInPixels / 2.0f) + WORLD_MIN.x,
        (rooms[boss_room_index].y * roomWidthInPixels + roomWidthInPixels / 2.0f) + WORLD_MIN.y
    };

    player = new Player(playerSpawn, 0.0f, 0.0f);
    enemies.push_back(Enemy(bossSpawn, 0.0f, 0.0f));

    player = new Player(Vector2Zero(), 0.0f, 0.0f);
    init_entities();

    Camera2D camera_view = {0};
    camera_view.target = player->position;
    camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    camera_view.zoom = 1.0f;

    SetTargetFPS(TARGET_FPS);

    float accumulator = 0.0f;

    bool endGame = false;

    while (!WindowShouldClose()){

        float delta_time = GetFrameTime();

        accumulator += delta_time;

        while(accumulator >= TIMESTEP && !endGame) {
            for (int i = 0; i < enemies.size(); i++) {
                player->HandleEntityCollision(&enemies[i]);
            }

            player->Update(TIMESTEP);

            for (int i = 0; i < enemies.size(); i++) {
                enemies[i].HandleEntityCollision(player);

                enemies[i].Update(TIMESTEP);
            }

            player->UpdateCamera(&camera_view);

            accumulator -= TIMESTEP;
        }

        BeginDrawing();

        if (player->HP <= 0) {
            ClearBackground(RED);
            DrawText("You Lose!", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 20, 40, WHITE);
            EndDrawing();
            endGame = true;

            continue;
        }

        bool are_all_enemies_unalive = true;
        for (int i = 0; i < enemies.size(); i++)
        {
            if (enemies[i].HP > 0)
            {
                are_all_enemies_unalive = false;
                break;
            }
        }

        if (are_all_enemies_unalive) {
            ClearBackground(GREEN);
            DrawText("You Win!", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 20, 40, WHITE);
            EndDrawing();
            endGame = true;

            continue;
        }

        ClearBackground(WHITE);

        BeginMode2D(camera_view);

        draw_level();
        // DrawTexturePro(image, {0.0f, 0.0f, 1600.0f, 1600.0f}, {WORLD_MIN.x, WORLD_MIN.y, 1600.0f, 1600.0f}, {0, 0}, 0, WHITE);

        // Mark boundaries (sana all)
        DrawText(TextFormat("x"), WORLD_MIN.x + (WORLD_MAX.x-WORLD_MIN.x)/2, WORLD_MIN.y + (WORLD_MAX.y-WORLD_MIN.y)/2, 20, RED);
        DrawText(TextFormat("x"), WORLD_MIN.x + 20, WORLD_MIN.y + 10, 20, RED);
        DrawText(TextFormat("x"), WORLD_MIN.x + 20, WORLD_MAX.y - 30, 20, RED);
        DrawText(TextFormat("x"), WORLD_MAX.x - 30, WORLD_MIN.y + 10, 20, RED);
        DrawText(TextFormat("x"), WORLD_MAX.x - 30, WORLD_MAX.y - 30, 20, RED);

        player->Draw();

        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].Draw();
        }
        
        EndMode2D();
        EndDrawing();

    }

    save_entities_positions();

    delete player;

    for (int i = 0; i < col_count; i++)
        delete[] grid[i];
    delete[] grid;

    UnloadTexture(tilemap);
    CloseWindow();
    return 0;
}

void init_level()
{
    tiles_loaded = 0;

    std::ifstream settings("settings.ini");
    std::string line;

    int counter = 0;

    while (std::getline(settings,line))
    {
        if (line[0] == '/') continue;

        if (counter == 0)
        {
            tilemap = LoadTexture(line.c_str());

            std::cout << "Loaded " << line << "\n";

            counter++;
            continue;
        }
        else if (counter == 1)
        {
            tile_count = std::stoi(line);
            tiles.reserve(tile_count);

            std::cout << "Tile Count: " << tile_count << "\n";

            counter++;
            continue;
        }

        if (tiles_loaded < tile_count)
        {
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

            std::cout << "Tile " << tiles_loaded << ": " <<
                        tiles[tiles_loaded].x << ", " << tiles[tiles_loaded].y << ", " <<
                        tiles[tiles_loaded].width << ", " << tiles[tiles_loaded].height << "\n";
            
            // is_wall
            std::string sub4 = sub3.substr(index3 + 1);
            int index4 = sub4.find(' ');
            
            // if 1, it is a wall tile
            if(std::stoi(sub4.substr(0, index4)) == 1)
            {
                // Source: https://www.geeksforgeeks.org/cpp/set-in-cpp-stl/
                walls.insert(tiles_loaded);
                std::cout << "\t- wall tile\n";
            }
            // else (if 0), it is just a floor tile

            tiles_loaded++;
        }
    }
}

void init_map()
{
    rows_loaded = 0;

    std::ifstream settings("layout.ini");
    std::string line;

    int counter = 0;

    while (std::getline(settings,line))
    {
        if (line[0] == '/') continue;

        if (counter == 0)
        {
            // x
            int index1 = line.find(' ');
            int x = std::stoi(line.substr(0, index1));

            // y
            std::string sub = line.substr(index1 + 1);
            int index2 = sub.find(' ');
            int y = std::stoi(sub.substr(0, index2));

            // Source: https://stackoverflow.com/questions/9219712/c-array-expression-must-have-a-constant-value
            grid = new int*[x];
            for (int i = 0; i < x; i++)
                grid[i] = new int[y];

            col_count = x;
            row_count = y;

            std::cout << "Initialized Grid: " << col_count << ", " << row_count << "\n";

            counter++;
        }
        else
        {
            int index = -1;
            std::string sub = line;

            for (int i = 0; i < col_count; i++)
            {
                std::string sub_i = sub.substr(index + 1);
                index = sub_i.find(' ');
                grid[i][rows_loaded] = std::stoi(sub_i.substr(0, index));
                sub = sub_i;

                if (grid[i][rows_loaded] < 10) std::cout << 0;

                std::cout << grid[i][rows_loaded] << " ";
            }

            rows_loaded++;
            std::cout << "\n";
        }

    tile_size = tiles[0].width;
    }
}

void init_entities()
{
    std::ifstream settings("entities_data.ini");
    std::string line;

    int counter = 0;

    while (std::getline(settings,line))
    {
        if (line[0] == '/') continue;

        if (counter == 1)
        {
            enemy_count = std::stoi(line);
            enemies.reserve(enemy_count);

            std::cout << "Reserved " << enemy_count << " for enemies\n";
        }
        else
        {
            // x
            int index0 = line.find(' ');
            float x = std::stof(line.substr(0, index0));

            // y
            std::string sub = line.substr(index0 + 1);
            int index1 = sub.find(' ');
            float y = std::stof(sub.substr(0, index1));
            
            if (counter == 0)
            {    
                player->position = {x, y};
                player->radius = 20.0f;
                player->speed = 100.0f;

                std::cout << "Initialized player: " << player->position.x << ", " << player->position.y
                            << " / " << player->radius << " / " << player->speed << "\n";
            }
            else
            {
                enemies.emplace_back();
                enemies.back().position = {x, y};
                enemies.back().speed = 50.0f;
                enemies.back().rect = {x, y, 50.0f, 50.0f};
                enemies.back().min = {x - 50.0f / 2.0f, y - 50.0f / 2.0f};
                enemies.back().max = {x + 50.0f / 2.0f, y + 50.0f / 2.0f};

                std::cout << "Initialized enemy: " << enemies.back().position.x << ", " << enemies.back().position.y
                            << " / " << enemies.back().speed << "\n";
            }
        }

        counter++;    
    }
}

void draw_level()
{
    for (int i = 0; i < col_count; i++)
    {
        for (int j = 0; j < row_count; j++)
        {
            int tile_num = grid[i][j];

            DrawTexturePro(tilemap, tiles[tile_num],
                            {WORLD_MIN.x + (float) i * tile_size, WORLD_MIN.y + (float) j * tile_size,
                            (float) tile_size, (float) tile_size}, {0,0}, 0, WHITE);
        }
    }
}

void save_entities_positions()
{
    // Source: https://www.w3schools.com/cpp/cpp_files.asp
    std::ofstream file("entities_data.ini");
    file << "// player\n"
            << player->position.x << ' ' << player->position.y << '\n'
            << "// enemy count\n"
            << enemy_count << '\n'
            << "// enemies\n";

    std::cout << "Player ended at " << player->position.x << ' ' << player->position.y << '\n';

    std::cout << "Enemies ended at:\n";

    for (int i = 0; i < enemies.size(); i++)
    {
        file << enemies[i].position.x << ' ' << enemies[i].position.y << '\n';
        std::cout << enemies[i].position.x << ' ' << enemies[i].position.y << '\n';
    }

    file.close();
}