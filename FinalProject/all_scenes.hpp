/**
 * List of all scenes
 * 
 * ○ 0 Title Scene
 * ○ 1 Main Game Scene
 * ○ 2 Settings Scene
 * ○ 3 Leaderboard Scene
 * ○ 4 Pause Scene
 * ○ 5 End of the Day Scene/Redo the Day Scene
 * ○ 6 Endgame Scene
 * ○ 7 Name Entry Scene
 *      source: https://github.com/raysan5/raylib/blob/master/examples/text/text_input_box.c
 */

#include <raylib.h>
#include <iostream>
#include <fstream>
#include <string>

#include "scene_manager.hpp"
#include "ui.hpp"
#include "game_functions.hpp"

struct UiLibrary uiLibrary;

class TitleScene : public Scene {
    Texture bean;
    float counter;

public:
    void Begin() override {
        bean = ResourceManager::GetInstance()->GetTexture("bean.png");
        counter = 0.0f;
    }

    void End() override {}

    void Update() override {
        if (IsKeyPressed(KEY_ENTER)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(1);
            }
        }
        if (uiLibrary.Button(0, "Start game"))
        {
            std::cout << "Hello!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(1);
            }
        }
        if (uiLibrary.Button(1, "Settings"))
        {
            std::cout << "Hello!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(2);
            }
        }
        if (uiLibrary.Button(2, "Leaderboard"))
        {
            std::cout << "Hi!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(3);
            }
        }
    }

    void Draw() override {
        DrawTexturePro(bean, {counter * 16, 0, 16, 16}, {250, 250, 300, 300}, {0, 0}, 0.0f, WHITE);
        DrawText("R@Nd0M\n  cafe!", 290, 350, 60, WHITE);
    }
};

class SettingsScene : public Scene {
public:
    bool play_music = true;

    void Begin() override {}

    void End() override {}

    void Update() override {
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(0);
            }
        }
        if (uiLibrary.Button(0, "Toggle Audio", 100.0f))
        {
            play_music = !play_music;
        }
        if (uiLibrary.Button(1, "Back to Start", 100.0f))
        {
            std::cout << "Hi!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(0);
            }
        }
    }

    void Draw() override {
        DrawText("Settings", 330, 30, 30, BLACK);
    }
};

class LeaderboardScene : public Scene {
    std::string temp, top3, leaderboard;

public:
    void Begin() override {
        std::ifstream in("leaderboard.txt");
        top3 = "";
        leaderboard = "";

        int counter = 1;

        while (getline (in, temp)) {
            if(counter < 4) {
                top3 += temp + "\n";
            }
            else {
                leaderboard += temp + "\n";
            }
            counter++;
        }

        in.close();
    }

    void End() override {}

    void Update() override {
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(0);
            }
        }
        if (uiLibrary.Button(0, "Back to Start", 500.0f))
        {
            std::cout << "Hello!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(0);
            }
        }
    }

    void Draw() override {
        DrawText("Leaderboard", 300, 30, 30, BLACK);
        DrawText(top3.c_str(), 270, 90, 40, RED);
        DrawText(leaderboard.c_str(), 340, 210, 20, BLACK);
    }
};

class GameScene : public Scene {
    Texture pause;
    float accumulator;

public:
    void Begin() override {
        pause = ResourceManager::GetInstance()->GetTexture("pause.png");

        srand(time(0));

        SetTargetFPS(FPS);
        init_textures();
        init_entities(registry, player, spawn_timer);
        reserve_memory();
        accumulator = 0;
        day_score = 0;
        button_name = "";

        customers_not_served = 0;
        customers_so_far = 0;
    }   

    void End() override {}

    void Update() override {
        float delta_time = GetFrameTime();

        read_player_input(registry, player);

        // Physics Step
        accumulator += delta_time;
        while(accumulator >= TIMESTEP)
        {
            update_customers(registry);
            affect_velocities(registry);
            move_entities(registry);
            handle_collisions(registry);
            get_hot_items(registry);
            update_timers(registry, spawn_timer);

            accumulator -= TIMESTEP;
        }

        if (uiLibrary.ButtonIcon(0, {770, 30}, pause))
        {
            std::cout << "Hello!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(4);
            }
        }

        if (button_name != "")
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                if (GetSceneManager() != nullptr) {
                    GetSceneManager()->SwitchScene(5);
                }
            }
        }
    }

    void Draw() override {
        draw_level(registry, player);

        if (button_name != "")
        {
            DrawText("Press 'Enter' to End Day", 300, 550, 18, BLACK);
        }

        // DrawText(TextFormat("Orders: %04i", balls.size()), 20, 20, 20, WHITE);
        // DrawTexturePro(raylib_logo, {0, 0, 256, 256}, {logo_position.x, logo_position.y, 200, 200}, {0, 0}, 0.0f, WHITE);
    }
};

class PauseScene : public Scene {
public:
    void Begin() override {}

    void End() override {}

    void Update() override {
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(1);
            }
        }
        if (uiLibrary.Button(0, "Resume Game"))
        {
            std::cout << "Hello!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(5);
            }
        }
        if (uiLibrary.Button(1, "Main Menu"))
        {
            std::cout << "Hi!" << std::endl;
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(6);
            }
        }
    }

    void Draw() override {
        DrawText("Game Paused", 300, 300, 30, BLACK);
    }
};

class DayEndScene : public Scene {
    float move_dir_x = 1;
    float position_x = 400.0f;

public:
    void Begin() override {}

    void End() override {}

    void Update() override {
        float delta_time = GetFrameTime();

        position_x += 100 * delta_time * move_dir_x;

        if (position_x >= 800) {
            position_x = -350.0f;
        }
        else if (position_x + 350.0f <= 0) {
            position_x = 800.0f;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(1);
            }
        }
        if (button_name == "Next Day" || button_name == "Redo Day")
        {
            if (uiLibrary.Button(0, button_name, 250.0f))
            {
                if (button_name == "Next Day")
                    day++;
                
                registry.clear();

                if (GetSceneManager() != nullptr) {
                    GetSceneManager()->SwitchScene(1);
                }
            }
        }
        else if (button_name == "End Game")
        {
            if (uiLibrary.Button(0, button_name, 250.0f))
            {
                if (GetSceneManager() != nullptr) {
                    GetSceneManager()->SwitchScene(6);
                }
            }
        }
    }

    void Draw() override {
        if (button_name == "Next Day" || button_name == "End Game")
        {
            DrawText("Yummy!", position_x, 30, 100, BLACK);
        }
        else
        {
            DrawText("Ermm..", position_x, 30, 100, BLACK);
        }

        DrawText(TextFormat("Total Score: %04i", int(score)), 300, 150, 30, BLACK);
        // DrawText(TextFormat("Orders: %04i", balls.size()), 20, 20, 20, WHITE);
    }
};

class EndGameScene : public Scene {
    Vector2 text_position;
    float move_dir_x = 1;
    float move_dir_y = 1;

public:
    void Begin() override {
        text_position = {300, 200};
    }   

    void End() override {}

    void Update() override {
        float delta_time = GetFrameTime();

        text_position.x += 100 * delta_time * move_dir_x;
        text_position.y += 100 * delta_time * move_dir_y;

        if (text_position.x + 320 >= 800 || text_position.x <= 0) {
            move_dir_x *= -1;
        }
        if (text_position.y + 100 >= 600 || text_position.y <= 0) {
            move_dir_y *= -1;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(7);
            }
        }
    }

    void Draw() override {
        DrawText("Yummy!", text_position.x, text_position.y, 100, BLACK);
        DrawText(TextFormat("Total Score: %04i", int(score)), 250, 150, 30, BLACK);
        DrawText("Press 'Enter' to type in your name for the leaderboard!", 150, 550, 18, BLACK);
    }
};

class NameEntryScene : public Scene {
    char name[5] = "\0";
    int letter_count = 0;

    Rectangle text_box = {WINDOW_WIDTH/2.0f - 100.0f, 250.0f, 200.0f, 100.0f};
    bool mouseOnText = false;

    int framesCounter = 0;

public:
    void Begin() override {
    }   

    void End() override {}

    void Update() override {
        if(CheckCollisionPointRec(GetMousePosition(), text_box)){
            mouseOnText = true;
        } else {
            mouseOnText = false;
        }

        if (mouseOnText) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
            
            int key = GetCharPressed();

            while (key > 0){
                if ((key >= 32) && (key <= 125) && (letter_count < 4)){
                    name[letter_count] = (char)key;
                    letter_count++;
                }

                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)){
                letter_count--;
                    if (letter_count < 0){
                        letter_count = 0;
                    }
                name[letter_count] = '\0';
            }

        }
        else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        if (mouseOnText){
            framesCounter++;
        }
        else {
            framesCounter = 0;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            std::ifstream in("leaderboard.txt");

            std::string line;
            std::string new_leaderboard = "";

            int offset = 0;

            while (getline (in, line)) {
                // rank
                int index0 = line.find(' ');
                int rank = std::stoi(line.substr(0, index0-1));

                if (offset == 0) {
                    // name
                    std::string sub = line.substr(index0 + 1);
                    int index1 = sub.find(' ');

                    // score
                    std::string old_score = sub.substr(index1 + 1);

                    try {
                        int old_score_value = std::stoi(old_score);

                        if (score > old_score_value) {
                            std::string new_score = "";
                            if (score < 10000)
                                new_score += "0";
                            if (score < 1000)
                                new_score += "0";
                            if (score < 100)
                                new_score += "0";
                            if (score < 10)
                                new_score += "0";

                            new_score += std::to_string(int(score));

                            new_leaderboard += std::to_string(rank) + ". " + name + " " + new_score + "\n";
                            offset = 1;
                        }
                    }
                    catch (...) {
                        std::string new_score = "";
                        if (score < 10000)
                            new_score += "0";
                        if (score < 1000)
                            new_score += "0";
                        if (score < 100)
                            new_score += "0";
                        if (score < 10)
                            new_score += "0";

                        new_score += std::to_string(int(score));

                        new_leaderboard += std::to_string(rank) + ". " + name + " " + new_score + "\n";
                        offset = 1; 
                    }
                }
                else if (rank + offset > 10)
                    break;

                new_leaderboard += std::to_string(rank+offset) + '.' + line.substr(index0) + "\n";
            }

            in.close();

            std::ofstream file("leaderboard.txt");
            file << new_leaderboard;
            file.close();

            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(3);
            }
        }
    }

    void Draw() override {
        DrawRectangleRec(text_box, LIGHTGRAY);
        DrawText("Press 'Enter' to submit your name for the leaderboard!", 150, 550, 18, BLACK);
        DrawText(name, (int)text_box.x + 10, (int)text_box.y +10, 40, BLACK);
    }
};