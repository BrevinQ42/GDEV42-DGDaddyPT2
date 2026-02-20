#include <raylib.h>
#include <raymath.h>
#include <iostream>

#include "Player.cpp"
#include "Enemy.cpp"

const float TARGET_FPS = 60.0f;
const float TIMESTEP = 1.0f / TARGET_FPS;

const int numEntities = 3;

Texture image;

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BenitoQueRedoble_Homework02");

    image = LoadTexture("bg.jpg");

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

    bool endGame = false;

    while (!WindowShouldClose()){

        float delta_time = GetFrameTime();

        accumulator += delta_time;

        while(accumulator >= TIMESTEP && !endGame) {
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

        if (player.HP <= 0) {
            ClearBackground(RED);
            DrawText("You Lose!", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 20, 40, WHITE);
            EndDrawing();
            endGame = true;
        }

        else if (e1.HP <= 0 && e2.HP <= 0) {
            ClearBackground(GREEN);
            DrawText("You Win!", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 20, 40, WHITE);
            EndDrawing();
            endGame = true;
        }

        else {
            ClearBackground(WHITE);

            BeginMode2D(camera_view);

            DrawTexturePro(image, {0.0f, 0.0f, 1800.0f, 1600.0f}, {WORLD_MIN.x, WORLD_MIN.y, 1800.0f, 1600.0f}, {0, 0}, 0, WHITE);

            // Mark boundaries (sana all)
            DrawText(TextFormat("x"), WORLD_MIN.x + (WORLD_MAX.x-WORLD_MIN.x)/2, WORLD_MIN.y + (WORLD_MAX.y-WORLD_MIN.y)/2, 20, RED);
            DrawText(TextFormat("x"), WORLD_MIN.x + 20, WORLD_MIN.y + 10, 20, RED);
            DrawText(TextFormat("x"), WORLD_MIN.x + 20, WORLD_MAX.y - 30, 20, RED);
            DrawText(TextFormat("x"), WORLD_MAX.x - 30, WORLD_MIN.y + 10, 20, RED);
            DrawText(TextFormat("x"), WORLD_MAX.x - 30, WORLD_MAX.y - 30, 20, RED);

            for (int i = 0; i < numEntities; i++) {
                entities[i]->Draw();
            }
            
            EndMode2D();
            EndDrawing();
        }

    }

    UnloadTexture(image);
    CloseWindow();
    return 0;
}

