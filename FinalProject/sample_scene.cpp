#include <raylib.h>

#include "scene_manager.hpp"
#include "all_scenes.hpp"

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Random Cafe");

    InitAudioDevice();

    SceneManager scene_manager;

    TitleScene title_scene;
    title_scene.SetSceneManager(&scene_manager);

    GameScene game_scene;
    game_scene.SetSceneManager(&scene_manager);

    SettingsScene settings_scene;
    settings_scene.SetSceneManager(&scene_manager);

    LeaderboardScene leaderboard_scene;
    leaderboard_scene.SetSceneManager(&scene_manager);

    PauseScene pause_scene;
    pause_scene.SetSceneManager(&scene_manager);

    DayEndScene day_end_scene;
    day_end_scene.SetSceneManager(&scene_manager);

    EndGameScene end_game_scene;
    end_game_scene.SetSceneManager(&scene_manager);

    NameEntryScene name_entry_scene;
    name_entry_scene.SetSceneManager(&scene_manager);

    scene_manager.RegisterScene(&title_scene, 0);
    scene_manager.RegisterScene(&game_scene, 1);
    scene_manager.RegisterScene(&settings_scene, 2);
    scene_manager.RegisterScene(&leaderboard_scene, 3);
    scene_manager.RegisterScene(&pause_scene, 4);
    scene_manager.RegisterScene(&day_end_scene, 5);
    scene_manager.RegisterScene(&end_game_scene, 6);
    scene_manager.RegisterScene(&name_entry_scene, 7);

    scene_manager.SwitchScene(0);

    Music main = LoadMusicStream("main.mp3");

    float time_played = 0.0f;
    bool play_music;

    while(!WindowShouldClose()) {
       Scene* active_scene = scene_manager.GetActiveScene();

        play_music = settings_scene.play_music;

        BeginDrawing();
        ClearBackground(Color{221, 161, 94, 255});

        if (active_scene != nullptr) {
            active_scene->Update();
            active_scene->Draw();
        }

        if (play_music) {
            if (!IsMusicStreamPlaying(main)) {
                PlayMusicStream(main);
            }
        }
        else {
            if (IsMusicStreamPlaying(main)) {
                StopMusicStream(main);
            }
        }

        if (IsMusicStreamPlaying(main)){
            UpdateMusicStream(main);
        }

        time_played = GetMusicTimePlayed(main)/GetMusicTimeLength(main);

        if (time_played > 1.0f){
            time_played = 0.0f;
            StopMusicStream(main);
            settings_scene.play_music = false;
        } 

        EndDrawing();
    }

    Scene* active_scene = scene_manager.GetActiveScene();
    if (active_scene != nullptr) {
        active_scene->End();
    }

    UnloadMusicStream(main);

    ResourceManager::GetInstance()->UnloadAllTextures();
    
    CloseAudioDevice();
    
    CloseWindow();
    
    return 0;
}