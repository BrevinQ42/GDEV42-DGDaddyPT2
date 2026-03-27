#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <string>

#include "Player.cpp"
#include "Enemy.cpp"
#include "Key.cpp"
#include "Lock.cpp"
#include "DunGen.cpp"

const float TARGET_FPS = 60.0f;
const float TIMESTEP = 1.0f / TARGET_FPS;

Texture tilemap;

int tile_count;
int tiles_loaded;
std::vector<Rectangle> tiles;

int current_room_index;

Camera2D camera_view;
bool is_camera_moving;
float camera_slide_speed;

void init_level();
void init_map();
void draw_level();

void setup_lock(Lock* lock);

void update_camera_state(Vector2 player_pos);
void slide_camera_to_new_room(float delta_time);

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BenitoQueRedoble_Homework04");
    init_level();
    dungen_main(); // dungen.cpp
    init_map();

    current_room_index = 0;

    // init entities
    Player player( rooms[current_room_index].center(), 20.0f, 100.0f);
    Enemy boss( rooms[boss_room_index].center(), 50.0f);

    Key key( Vector2Subtract(rooms[key_room_index].center(), {8.0f, 8.0f}), LoadTexture("key-silver.png") );

    // if key is in starting room, move it to top left corner of room
    if (key_room_index == current_room_index)
    {
        key.Reset(
            Vector2Subtract(key.position, {64.0f * 4.0f + 56.0f, 64.0f * 3.0f + 56.0f})
        );
    }

    Lock lock;
    lock.sprite_sheet = LoadTexture("lock.png");
    setup_lock(&lock);

    // set up camera
    camera_view = {0};
    camera_view.target = rooms[current_room_index].center();
    camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    camera_view.zoom = 1.0f;

    is_camera_moving = false;
    camera_slide_speed = 1000.0f;

    SetTargetFPS(TARGET_FPS);

    float accumulator = 0.0f;

    bool endGame = false;

    while (!WindowShouldClose()){
        float delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_P))
        {
            std::cout << "Player at " << player.position.x << ", " << player.position.y << "\n"
                        << "Boss at " << boss.position.x << ", " << boss.position.y << "\n"
                        << "Lock at " << lock.position.x << ", " << lock.position.y << "\n\n";
        }

        if (is_camera_moving)
            slide_camera_to_new_room(delta_time);
        else
        {
            if (IsKeyPressed(KEY_R))
            {
                // generate new dungeon
                dungen_main();
                init_map();

                current_room_index = 0;

                // reset entities
                player.Reset( rooms[current_room_index].center() );
                boss.Reset(rooms[boss_room_index].center());
                key.Reset( Vector2Subtract(rooms[key_room_index].center(), {8.0f, 8.0f}) );

                // if key is in starting room, move it to top left corner of room
                if (key_room_index == current_room_index)
                {
                    key.Reset(
                        Vector2Subtract(key.position, {64.0f * 4.0f + 32.0f, 64.0f * 3.0f + 32.0f})
                    );
                }

                // setup lock again
                setup_lock(&lock);

                // setup camera
                camera_view.target = rooms[current_room_index].center();
                camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};

                endGame = false;
            }

            accumulator += delta_time;

            while(accumulator >= TIMESTEP && !endGame) {
                if (current_room_index == boss_room_index)
                    player.HandleEntityCollision(&boss);
                
                player.Update(TIMESTEP);
                update_camera_state(player.position);
                
                if (is_key_active && current_room_index == key_room_index) {
                    player.HandleEntityCollision(&key);
                }

                key.Update(TIMESTEP);

                if (is_lock_active && current_room_index == rooms[locked_room_index].previous_room->index) {
                    player.HandleEntityCollision(&lock);
                }

                if (current_room_index == boss_room_index)
                {
                    boss.HandleEntityCollision(&player);
                    boss.Update(TIMESTEP);
                }

                accumulator -= TIMESTEP;
            }
        }

        BeginDrawing();

        if (player.HP <= 0) {
            ClearBackground(RED);
            DrawText("GAME OVER - YOU DIED", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2, 30, WHITE);
            EndDrawing();
            endGame = true;

            continue;
        }

        if (boss.HP <= 0) {
            ClearBackground(GREEN);
            DrawText("VICTORY - BOSS DEFEATED", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2, 30, WHITE);
            EndDrawing();
            endGame = true;

            continue;
        }

        ClearBackground(BLACK);
        BeginMode2D(camera_view);

        draw_level();
        lock.Draw();
        player.Draw();
        key.Draw();
        boss.Draw();
        
        EndMode2D();
        EndDrawing();

    }

    for (int i = 0; i < col_count; i++)
        delete[] grid[i];
    delete[] grid;

    UnloadTexture(tilemap);
    UnloadTexture(lock.sprite_sheet);
    UnloadTexture(key.sprite_sheet);

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

            counter++;
            continue;
        }
        else if (counter == 1)
        {
            tile_count = std::stoi(line);
            tiles.reserve(tile_count);

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
            
            // is_wall
            std::string sub4 = sub3.substr(index3 + 1);
            int index4 = sub4.find(' ');
            
            // if 1, it is a wall tile
            if(std::stoi(sub4.substr(0, index4)) == 1)
            {
                // Source: https://www.geeksforgeeks.org/cpp/set-in-cpp-stl/
                walls.insert(tiles_loaded);
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
            }

            rows_loaded++;
        }

    tile_size = tiles[0].width;
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

void setup_lock(Lock* lock)
{
    Room locked_room = rooms[locked_room_index];
    Room room_with_lock = *(locked_room.previous_room);

    // if lock is to the left
    if (room_with_lock.x > locked_room.x)
    {
        // take note of door locations in grid
        lock->doors[0] = {(float) room_with_lock.x * 12, (float) room_with_lock.y * 10 + 4};
        lock->doors[1] = {(float) room_with_lock.x * 12, (float) room_with_lock.y * 10 + 5};

        // put walls on door locations
        grid[(int) lock->doors[0].x][(int) lock->doors[0].y] = 5;
        grid[(int) lock->doors[1].x][(int) lock->doors[1].y] = 5;

        // put lock to the right of the doors
        lock->Reset(
            {WORLD_MIN.x + (lock->doors[0].x + 1) * tile_size - 16.0f,
                WORLD_MIN.y + lock->doors[1].y * tile_size - 16.0f}
        );

        return;
    }

    // if lock is to the right
    if (room_with_lock.x < locked_room.x)
    {
        // take note of door locations in grid
        lock->doors[0] = {(float) room_with_lock.x * 12 + 11, (float) room_with_lock.y * 10 + 4};
        lock->doors[1] = {(float) room_with_lock.x * 12 + 11, (float) room_with_lock.y * 10 + 5};

        // put walls on door locations
        grid[(int) lock->doors[0].x][(int) lock->doors[0].y] = 9;
        grid[(int) lock->doors[1].x][(int) lock->doors[1].y] = 9;

        // put lock to the left of the doors
        lock->Reset(
            {WORLD_MIN.x + lock->doors[0].x * tile_size - 16.0f,
                WORLD_MIN.y + lock->doors[1].y * tile_size - 16.0f}
        );

        return;
    }

    // if lock is to the top
    if (room_with_lock.y > locked_room.y)
    {
        // take note of door locations in grid
        lock->doors[0] = {(float) room_with_lock.x * 12 + 5, (float) room_with_lock.y * 10};
        lock->doors[1] = {(float) room_with_lock.x * 12 + 6, (float) room_with_lock.y * 10};

        // put walls on door locations
        grid[(int) lock->doors[0].x][(int) lock->doors[0].y] = 3;
        grid[(int) lock->doors[1].x][(int) lock->doors[1].y] = 3;

        // put lock to the bot of the doors
        lock->Reset(
            {WORLD_MIN.x + lock->doors[1].x * tile_size - 16.0f,
                WORLD_MIN.y + (lock->doors[0].y + 1) * tile_size - 16.0f}
        );

        return;
    }

    // if lock is to the bottom
    if (room_with_lock.y < locked_room.y)
    {
        // take note of door locations in grid
        lock->doors[0] = {(float) room_with_lock.x * 12 + 5, (float) room_with_lock.y * 10 + 9};
        lock->doors[1] = {(float) room_with_lock.x * 12 + 6, (float) room_with_lock.y * 10 + 9};

        // put walls on door locations
        grid[(int) lock->doors[0].x][(int) lock->doors[0].y] = 7;
        grid[(int) lock->doors[1].x][(int) lock->doors[1].y] = 7;

        // put lock to the bot of the doors
        lock->Reset(
            {WORLD_MIN.x + lock->doors[1].x * tile_size - 16.0f,
                WORLD_MIN.y + lock->doors[0].y * tile_size - 16.0f}
        );

        return;
    }
}

void update_camera_state(Vector2 player_pos)
{
    Room current_room = rooms[current_room_index];

    // if player went out of the room towards the left,
    if (current_room.center().x - player_pos.x >= WINDOW_WIDTH / 2)
    {
        // update current room
        current_room_index = find_room(current_room.x-1, current_room.y);

        // setup camera for slide
        camera_view.target = rooms[current_room_index].center();
        camera_view.offset = {WINDOW_WIDTH / 2 - WINDOW_WIDTH, WINDOW_HEIGHT / 2};

        // start camera movement
        is_camera_moving = true;

        return;
    }

    // if player went out of the room towards the right,
    if (player_pos.x - current_room.center().x >= WINDOW_WIDTH / 2)
    {
        // update current room
        current_room_index = find_room(current_room.x+1, current_room.y);

        // setup camera for slide
        camera_view.target = rooms[current_room_index].center();
        camera_view.offset = {WINDOW_WIDTH / 2 + WINDOW_WIDTH, WINDOW_HEIGHT / 2};

        // start camera movement
        is_camera_moving = true;

        return;
    }

    // if player went out of the room towards the top,
    if (current_room.center().y - player_pos.y >= WINDOW_HEIGHT / 2)
    {
        // update current room
        current_room_index = find_room(current_room.x, current_room.y-1);

        // setup camera for slide
        camera_view.target = rooms[current_room_index].center();
        camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - WINDOW_HEIGHT};

        // start camera movement
        is_camera_moving = true;

        return;
    }



    // if player went out of the room towards the bottom,
    if (player_pos.y - current_room.center().y >= WINDOW_HEIGHT / 2)
    {
        // update current room
        current_room_index = find_room(current_room.x, current_room.y+1);

        // setup camera for slide
        camera_view.target = rooms[current_room_index].center();
        camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + WINDOW_HEIGHT};

        // start camera movement
        is_camera_moving = true;

        return;
    }

    // else, no camera movement
}

void slide_camera_to_new_room(float delta_time)
{
    Vector2 slide_vector = Vector2Subtract({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2}, camera_view.offset);

    // if slide vector is slower than camera speed at current frame,
    if (Vector2Length(slide_vector) < camera_slide_speed * delta_time)
    {
        // move camera based on the slide vector to make camera centered on new room
        camera_view.offset = Vector2Add(camera_view.offset, slide_vector);
        
        // stop moving camera
        is_camera_moving = false;
    }

    // else if slide vector would be faster,
    else
    {
        // move camera based on slide speed at current frame along the slide vector
        slide_vector = Vector2Scale(Vector2Normalize(slide_vector), camera_slide_speed * delta_time);
        camera_view.offset = Vector2Add(camera_view.offset, slide_vector);

        // continue moving camera
    }
}