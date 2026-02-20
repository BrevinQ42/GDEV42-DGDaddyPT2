#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "Player.hpp"
#include "Enemy.hpp"

const float WINDOW_WIDTH = 1280.0f;
const float WINDOW_HEIGHT = 720.0f;

const float TARGET_FPS = 60.0f;
const float TIMESTEP = 1.0f / TARGET_FPS;

const Vector2 min = {-500, -500};
const Vector2 max = {1300, 1100};

const int numEntities = 3;

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BenitoQueRedoble_Homework02");

    Vector2 position = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};

    Camera2D camera_view = {0};
    camera_view.target = position;
    camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    camera_view.zoom = 1.0f;

    Player player(position, 20.0f, 100.0f);
    Enemy e1({position.x + 400, position.y + 400}, 50.0f);
    Enemy e2({position.x - 400, position.y - 400}, 50.0f);

    Entity* entities[numEntities] = {&player, &e1, &e2};

    SetTargetFPS(TARGET_FPS);

    float accumulator = 0.0f;

    while (!WindowShouldClose()){
        float delta_time = GetFrameTime();

        accumulator += delta_time;

        while(accumulator >= TIMESTEP) {
            for (int i = 0; i < numEntities; i++) { // Could be simplified but this works for the bonus
                for (int j = 0; j < numEntities; j++) {
                    if (i != j) {
                        entities[i]->HandleCollision(entities[j]);
                    }
                }

                entities[i]->Update(TIMESTEP);
            }

            player.UpdateCamera(&camera_view);

            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(camera_view);

        // Mark boundaries (sana all)
        DrawText(TextFormat("x"), min.x + (max.x-min.x)/2, min.y + (max.y-min.y)/2, 20, RED);
        DrawText(TextFormat("x"), min.x + 20, min.y + 10, 20, RED);
        DrawText(TextFormat("x"), min.x + 20, max.y - 30, 20, RED);
        DrawText(TextFormat("x"), max.x - 30, min.y + 10, 20, RED);
        DrawText(TextFormat("x"), max.x - 30, max.y - 30, 20, RED);

        for (int i = 0; i < numEntities; i++) {
            entities[i]->Draw();
        }
        
        std::cout << "Player position: (" << player.position.x << ", " << player.position.y << ")\n";
        std::cout << delta_time << " seconds since last frame\n";
        
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

