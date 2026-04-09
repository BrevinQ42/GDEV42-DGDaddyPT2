#include <raylib.h>
#include <string>

const float WINDOW_WIDTH = 800.0f, WINDOW_HEIGHT = 600.0f;

// Struct to encapsulate our UI library
struct UiLibrary
{
    // ID of the currently hot widget
    int hot = -1;

    // ID of the currently active widget
    int active = -1;

    // y of the first element
    float base_y = 40.0f;

    // Creates a button with the specified text and bounds
    // Returns true if this button was clicked in this frame
    bool Button(int id, const std::string& text, float y_offset = 40.0f)
    {
        bool result = false;
        Color col = Color{254, 250, 224, 255};
        Color txt = Color{96, 108, 56, 255};

        float width = text.length() * 20.0f;
        float x = WINDOW_WIDTH/2.0f - width/2.0f;
        float y = y_offset + id * base_y * 2;

        Rectangle edge_bounds = {x, y, 18.0f, base_y};
        Rectangle bounds = {x, y, width, base_y};

        // If this button is the currently active widget, that means
        // the user is currently interacting with this widget
        if (id == active)
        {
            col = Color{40, 54, 24, 255};
            txt = Color{254, 250, 224, 255};
            // If the user released the mouse button while we are active,
            // register as a click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Make sure that we are still the hot widget
                // (User might have dragged mouse to another widget, in which case we will not count as a button click)
                if (id == hot)
                {
                    result = true;
                }

                // Set ourselves to not be active anymore
                active = -1;
            }
        }

        // If we are currently the hot widget
        else if (id == hot)
        {
            col = Color{96, 108, 56, 255};
            txt = Color{254, 250, 224, 255};
            // If the user pressed the left mouse button, that means the user started
            // interacting with this widget, so we set this widget as active
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                active = id;
            }
        }

        // If the mouse cursor is hovering within our boundaries
        if (CheckCollisionPointRec(GetMousePosition(), bounds))
        {
            // Set this widget to be the hot widget
            hot = id;
        }
        // If the mouse cursor is not on top of this widget, and this widget
        // was previously the hot widget, set the hot widget to -1
        // We check if this widget was the previously hot widget since there is a possibility that
        // the hot widget is now a different widget, and we don't want to overwrite that with -1
        else if (hot == id)
        {
            hot = -1;
            col = Color{254, 250, 224, 255};
        }

        // Draw our button regardless of what happens
        
        DrawRectangleRec(bounds, Color{254, 250, 224, 80});
        DrawRectangleRec(edge_bounds, col);
        DrawText(text.c_str(), x + width/8.0f, y + base_y/3.0f - 7.0f, 28, txt);

        return result;
    }

    bool ButtonIcon(int id, const Vector2& position, const Texture& icon)
    {
        bool result = false;
        float scale = 1.0f, icon_w = (float)icon.width, icon_h = (float)icon.height;

        Rectangle icon_bounds = {0.0f, 0.0f, icon_w, icon_h};
        Vector2 origin = {icon_w * scale/2.0f, icon_h * scale/2.0f};
        
        Rectangle dest_bounds;

        // If this button is the currently active widget, that means
        // the user is currently interacting with this widget
        if (id == active)
        {
            scale = 0.8f;
            // If the user released the mouse button while we are active,
            // register as a click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Make sure that we are still the hot widget
                // (User might have dragged mouse to another widget, in which case we will not count as a button click)
                if (id == hot)
                {
                    result = true;
                }

                // Set ourselves to not be active anymore
                active = -1;
            }
        }

        // If we are currently the hot widget
        else if (id == hot)
        {
            scale = 1.2f;
            // If the user pressed the left mouse button, that means the user started
            // interacting with this widget, so we set this widget as active
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                active = id;
            }
        }

        // If the mouse cursor is hovering within our boundaries
        if (CheckCollisionPointRec(GetMousePosition(), 
            {position.x - origin.x, position.y - origin.y, icon_w * scale, icon_h * scale}))
        {
            // Set this widget to be the hot widget
            hot = id;
        }
        // If the mouse cursor is not on top of this widget, and this widget
        // was previously the hot widget, set the hot widget to -1
        // We check if this widget was the previously hot widget since there is a possibility that
        // the hot widget is now a different widget, and we don't want to overwrite that with -1
        else if (hot == id)
        {
            hot = -1;
            scale = 1.0f;
        }

        // Draw our button regardless of what happens
        // adjusted_position.x = position.x + icon.width - ((float)icon.width * scale);
        // adjusted_position.y = position.y + icon.height - ((float)icon.height * scale);
        // adjusted_position.y = adjusted_position.y < 0.0f ? 0.0f : adjusted_position.y;
        dest_bounds = {position.x, position.y, icon_w * scale, icon_h * scale};
        origin = {icon_w * scale/2.0f, icon_h * scale/2.0f};

        //DrawTextureEx(icon, adjusted_position, 0.0f, scale, WHITE);
        DrawTexturePro(icon, icon_bounds, dest_bounds, origin, 0.0f, WHITE);
        
        return result;
    }

    bool Checkbox(int id, bool checked, const std::string& text, const Rectangle& bounds)
    {
        if(id == active)
        {
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                checked = !checked;
                active = -1;
            }
        }

        else if(id == hot)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                active = id;
            }
        }

        if (CheckCollisionPointRec(GetMousePosition(), bounds)){
            hot = id;
        }
        else if (hot == id){
            hot = -1;
        }

        DrawRectangleRec(bounds, GRAY);
        DrawRectangleLines(bounds.x + 10, bounds.y + 10, 10, 10, BLACK);
        if (checked){
            DrawRectangle(bounds.x + 12, bounds.y + 12, 6, 6, RED);
        }
        DrawText(text.c_str(), bounds.x + 30, bounds.y + 10, 14, BLACK);

        return checked;
    }

    void Label(const std::string& text, const Rectangle& bounds)
    {
        DrawText(text.c_str(), bounds.x, bounds.y, 14, BLACK);
        return;
    }
};