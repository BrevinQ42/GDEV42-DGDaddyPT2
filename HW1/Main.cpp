#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <string>
#include <iostream>

const float WINDOW_WIDTH = 800;
const float WINDOW_HEIGHT = 600;
const float radius = 20;

Texture image;
std::string object_names[5];
Vector2 object_positions[5];
float object_radii[5];

Color object_text_color[5] = {BLACK, BLACK, BLACK, BLACK, BLACK};
int objects_found = 0;

Vector2 min;
Vector2 max;
Vector2 camera_window;

float drift_factor;
float zoom_factor;

void init_settings();

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BenitoQueRedoble_Homework01");

    // init settings
    init_settings();

    Vector2 position = {(max.x + min.x) / 2, (max.y + min.y) / 2};
    
    Camera2D camera_view = {0};
    camera_view.target = position;
    camera_view.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    camera_view.zoom = 1.0f;

    bool isEdgeSnapped[2] = {false, false};

    Texture win_screen = LoadTexture("win_screen.jpg");

    SetTargetFPS(60.0f);

    while (!WindowShouldClose())
    {
        if (objects_found == 5)
        {
            BeginDrawing();
            ClearBackground(BLACK);

            DrawTexturePro(win_screen, {0.0f, 0.0f, 780, 438}, {0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT}, {0, 0}, 0, WHITE);
            DrawText("CONGRATULATIONS!", WINDOW_WIDTH / 2 - 225, 50.0f, 50, YELLOW);
        
            EndDrawing();
            
            continue;
        }

        float delta_time = GetFrameTime();

        if (IsKeyDown(KEY_W)){
            position.y -= 150.0f * delta_time;

            if (position.y - radius <= min.y)
                position.y = min.y + radius;
        }
        if (IsKeyDown(KEY_A)){
            position.x -= 150.0f * delta_time;

            if (position.x - radius <= min.x)
                position.x = min.x + radius;
        }
        if (IsKeyDown(KEY_S)){
            position.y += 150.0f * delta_time;

            if (position.y + radius >= max.y)
                position.y = max.y - radius;
        }
        if (IsKeyDown(KEY_D)){
            position.x += 150.0f * delta_time;

            if (position.x + radius >= max.x)
                position.x = max.x - radius;
        }

        if (IsKeyPressed(KEY_T))
            std::cout << position.x << " / " <<  WINDOW_WIDTH - camera_view.offset.x << "\n";

        if (IsKeyPressed(KEY_ENTER)){
            if (camera_view.zoom == 1.0f)
            {
                camera_view.zoom = zoom_factor;
            }
            else
            {
                for (int i = 0; i < 5; i++)
                {
                    // if colored red already (meaning, already found)
                    if (object_text_color[i].r == 230)
                        continue;

                    if (Vector2Length(Vector2Subtract(object_positions[i], position)) <= object_radii[i])
                    {
                        object_text_color[i] = RED;
                        objects_found++;
                        break;
                    }
                }
            }
        }
        else if (IsKeyDown(KEY_TAB) && camera_view.zoom == zoom_factor)
            camera_view.zoom = 1.0f;

        // if zoomed out, make camera move accordingly
        if (camera_view.zoom == 1.0f)
        {
            // camera window
            Vector2 old_target = camera_view.target;
            camera_view.target = position;

            camera_view.offset.x = Clamp(camera_view.offset.x + position.x - old_target.x,
                                        WINDOW_WIDTH / 2 - camera_window.x / 2, WINDOW_WIDTH / 2 + camera_window.x / 2);

            camera_view.offset.y = Clamp(camera_view.offset.y + position.y - old_target.y,
                                        WINDOW_HEIGHT / 2 - camera_window.y / 2, WINDOW_HEIGHT / 2 + camera_window.y / 2);

            // position snapping (also accounting for edge snapping)
            Vector2 drift_vector = {WINDOW_WIDTH / 2 - camera_view.offset.x, WINDOW_HEIGHT / 2 - camera_view.offset.y};

            if (isEdgeSnapped[0]) drift_vector.x = 0;
            if (isEdgeSnapped[1]) drift_vector.y = 0;

            float drift_vec_mag = Vector2Length(drift_vector);
            camera_view.offset = Vector2Add(camera_view.offset, Vector2Normalize(drift_vector) * fminf(drift_factor, drift_vec_mag));

            // edge snapping
            if (position.x - camera_view.offset.x <= min.x)
            {
                camera_view.offset.x = position.x - min.x;

                if (camera_view.offset.x < WINDOW_WIDTH / 2)
                    isEdgeSnapped[0] = true;
                else
                    isEdgeSnapped[0] = false;
            }
            else if (position.x + (WINDOW_WIDTH - camera_view.offset.x) >= max.x)
            {
                camera_view.offset.x = position.x + WINDOW_WIDTH - max.x;
                
                if (camera_view.offset.x > WINDOW_WIDTH / 2)
                    isEdgeSnapped[0] = true;
                else
                    isEdgeSnapped[0] = false;
            }
            else
                isEdgeSnapped[0] = false;

            if (position.y - camera_view.offset.y <= min.y)
            {
                camera_view.offset.y = position.y - min.y;
                
                if (camera_view.offset.y < WINDOW_HEIGHT / 2)
                    isEdgeSnapped[1] = true;
                else
                    isEdgeSnapped[1] = false;
            }
            else if (position.y + (WINDOW_HEIGHT - camera_view.offset.y) >= max.y)
            {
                camera_view.offset.y = position.y + WINDOW_HEIGHT - max.y;
                
                if (camera_view.offset.y > WINDOW_HEIGHT / 2)
                    isEdgeSnapped[1] = true;
                else
                    isEdgeSnapped[1] = false;
            }
            else
                isEdgeSnapped[1] = false;
        }
        // else if zoomed in, static camera
        else
        {
             if (max.x - position.x <= WINDOW_WIDTH / 2)
                camera_view.offset.x = WINDOW_WIDTH - (max.x - position.x);
            else if (position.x - min.x <= WINDOW_WIDTH / 2)
                camera_view.offset.x = position.x - min.x;
            else
                camera_view.offset.x = WINDOW_WIDTH / 2;

            if (max.y - position.y <= WINDOW_HEIGHT / 2)
                camera_view.offset.y = WINDOW_HEIGHT - (max.y - position.y);
            else if (position.y - min.y <= WINDOW_HEIGHT / 2)
                camera_view.offset.y = position.y - min.y;
            else
                camera_view.offset.y = WINDOW_HEIGHT / 2;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        
        BeginMode2D(camera_view);

        DrawTexturePro(image, {0.0f, 0.0f, max.x-min.x, max.y-min.y}, {min.x, min.y, max.x-min.x, max.y-min.y}, {0, 0}, 0, WHITE);
        DrawCircle(position.x, position.y, radius, RED);

        EndMode2D();

        // camera window lines

        DrawLineEx({WINDOW_WIDTH / 2 - camera_window.x / 2, WINDOW_HEIGHT / 2 - camera_window.y / 2},
                   {WINDOW_WIDTH / 2 + camera_window.x / 2, WINDOW_HEIGHT / 2 - camera_window.y / 2}, 2.5f, BLUE);
        DrawLineEx({WINDOW_WIDTH / 2 - camera_window.x / 2, WINDOW_HEIGHT / 2 - camera_window.y / 2},
                   {WINDOW_WIDTH / 2 - camera_window.x / 2, WINDOW_HEIGHT / 2 + camera_window.y / 2}, 2.5f, BLUE);
        DrawLineEx({WINDOW_WIDTH / 2 + camera_window.x / 2, WINDOW_HEIGHT / 2 + camera_window.y / 2},
                   {WINDOW_WIDTH / 2 + camera_window.x / 2, WINDOW_HEIGHT / 2 - camera_window.y / 2}, 2.5f, BLUE);
        DrawLineEx({WINDOW_WIDTH / 2 + camera_window.x / 2, WINDOW_HEIGHT / 2 + camera_window.y / 2},
                   {WINDOW_WIDTH / 2 - camera_window.x / 2, WINDOW_HEIGHT / 2 + camera_window.y / 2}, 2.5f, BLUE);

        // objects
        DrawRectangle(0.0f, 0.0f, 125, 150, {200, 200, 200, 128});

        for (int i = 0; i < 5; i++)
            DrawText(object_names[i].c_str(), 10.0f, 25 * i + 10, 20, object_text_color[i]);

        EndDrawing();
    }

    CloseWindow();
    UnloadTexture(win_screen);
    UnloadTexture(image);
    return 0;
}

void init_settings()
{
    std::ifstream settings("settings.ini");
    std::string line;

    int counter = 1;

    while (std::getline(settings, line)) {
        if (line[0] == '/') continue;

        if (counter == 1)
        {
            image = LoadTexture(line.c_str());
        }
        else if (counter < 7)
        {
            // object_name x y r 

            // name
            int index0 = line.find(' ');
            object_names[counter-2] = line.substr(0, index0);

            // x
            std::string sub = line.substr(index0 + 1);
            int index1 = sub.find(' ');
            object_positions[counter-2].x = std::stof(sub.substr(0, index1));

            // y
            std::string sub2 = sub.substr(index1 + 1);
            int index2 = sub2.find(' ');
            object_positions[counter-2].y = std::stof(sub2.substr(0, index2));

            // r
            object_radii[counter-2] = std::stof(sub2.substr(index2));
        }
        else if (counter == 7)
        {
            // min.x min.y max.x max.y

            // min.x
            int index0 = line.find(' ');
            min.x = std::stof(line.substr(0, index0));

            // min.y
            std::string sub = line.substr(index0 + 1);
            int index1 = sub.find(' ');
            min.y = std::stof(sub.substr(0, index1));

            // max.x
            std::string sub2 = sub.substr(index1 + 1);
            int index2 = sub2.find(' ');
            max.x = std::stof(sub2.substr(0, index2));

            // max.y
            max.y = std::stof(sub2.substr(index2));
        }
        else if (counter == 8)
        {
            // camera window (x, y)

            // x
            int index = line.find(' ');
            camera_window.x = std::stof(line.substr(0, index));

            // y
            camera_window.y = std::stof(line.substr(index));
        }
        else if (counter == 9)
            drift_factor = std::stof(line);
        else if (counter == 10)
            zoom_factor = std::stof(line);

        counter++;
    }
}