#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <string>
#include <fstream>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// MAKE IT BACK TO 10 BY 10
const int row_cells_count = 10;
const int col_cells_count = 10;

const int min_room = 5;
const int max_room = 10;

struct Room
{
    int x;
    int y;
    Color color;

    int distance_from_start;
    int index;

    Room* previous_room;

    std::string layout[10];

    Vector2 center()
    {
        return {WORLD_MIN.x + x * WINDOW_WIDTH + WINDOW_WIDTH / 2,
                WORLD_MIN.y + y * WINDOW_HEIGHT + WINDOW_HEIGHT / 2};
    }
};

std::set<std::string> active_rooms;
std::vector<Room> rooms;
int room_count;

std::set<int> end_rooms; // will contain indices of end rooms in the rooms vector

// Source: https://www.geeksforgeeks.org/cpp/queue-cpp-stl/
std::queue<int> rooms_queue; // will contain the indices of the rooms in the queue

int boss_room_index;
int key_room_index;
int locked_room_index;

std::string final_layout[row_cells_count * 10];

void initialize_room(std::string (&layout)[10]);
bool is_valid_new_room(int room_x, int room_y);
void add_new_room(int x, int y, Color color, int distance, Room* pointerToPrevious);
bool try_add_room(int room_x, int room_y, int previous_index);
void locate_boss_room();
void add_key_and_lock();
void generate_dungeon();
int find_room(int x, int y);
void generate_map();
void print_map();

int dungen_main()
{
    srand(0);
    generate_dungeon();
    generate_map();

    return 0;
}

void initialize_room(std::string (&layout)[10])
{
    for (int i = 0; i < 10; i++)
    {
        if (i == 0) 
            layout[i] = "02 03 03 03 03 03 03 03 03 03 03 04"; // pure walls
        else if (i == 9)
            layout[i] = "06 07 07 07 07 07 07 07 07 07 07 08"; // pure walls
        else 
            layout[i] = "05 15 15 15 15 15 15 15 15 15 15 09"; // floor tiles w/ walls at both ends
    }
}

bool is_valid_new_room(int room_x, int room_y)
{
    // if room is an active room, invalid new room
    if (active_rooms.count(std::to_string(room_x) + ',' + std::to_string(room_y)))
        return false;

    int neighbor_count = 0;

    // check top
    if (room_y > 0 && active_rooms.count(std::to_string(room_x) + ',' + std::to_string(room_y-1)))
        neighbor_count++;

    // check left
    if (room_x > 0 && active_rooms.count(std::to_string(room_x-1) + ',' + std::to_string(room_y)))
        neighbor_count++;

    // check bottom
    if (room_y < row_cells_count-1 && active_rooms.count(std::to_string(room_x) + ',' + std::to_string(room_y+1)))
        neighbor_count++;

    // check right
    if (room_x < col_cells_count-1 && active_rooms.count(std::to_string(room_x+1) + ',' + std::to_string(room_y)))
        neighbor_count++;

    // if neighbor count is 1 or less, its valid
    // else, invalid
    return (neighbor_count <= 1);
}

void add_new_room(int x, int y, Color color, int distance, Room* pointerToPrevious)
{
    rooms.emplace_back();
    rooms.back().x = x;
    rooms.back().y = y;
    rooms.back().color = color;
    rooms.back().distance_from_start = distance;
    rooms.back().index = rooms.size()-1;
    rooms.back().previous_room = pointerToPrevious;
    initialize_room(rooms.back().layout);

    active_rooms.insert(std::to_string(rooms.back().x) + ',' + std::to_string(rooms.back().y));
}

// returns true if room is added, otherwise false
bool try_add_room(int room_x, int room_y, int previous_index)
{
    // if room is outside bounds of grid, don't add room
    if (room_x < 0 || room_x > col_cells_count-1 || room_y < 0 || room_y > row_cells_count-1)
        return false;

    // if room is valid,
    if (is_valid_new_room(room_x, room_y))
    {
        // choose between 0 and 1
        // if 1, we add a new room
        if (GetRandomValue(0,1))
        {
            // remove previous room as end room if it was an end room
            end_rooms.erase(previous_index);

            // if previous room is not starting room, make it gray
            if (previous_index > 0)
                rooms[previous_index].color = GRAY;

            add_new_room(room_x, room_y, DARKGRAY, rooms[previous_index].distance_from_start+1, &rooms[previous_index]);

            // categorize newest room as end room
            end_rooms.insert(rooms.size()-1);

            // add it to the queue
            rooms_queue.push(rooms.size()-1);

            return true;
        }

        // else (if 0)
        return false;
    }

    // else if invalid
    return false;
}

void locate_boss_room()
{
    boss_room_index = 0;
    int furthest_distance = 0;

    // for each end room
    for (int room_index : end_rooms)
    {
        // if end room's distance is further than furthest distance so far
        if (rooms[room_index].distance_from_start > furthest_distance)
        {
            // update boss room and furthest distance so far
            boss_room_index = room_index;
            furthest_distance = rooms[room_index].distance_from_start;
        }

        // else if equal
        else if (rooms[room_index].distance_from_start == furthest_distance)
        {
            // choose between 0 and 1
            // if 1, switch to this new room
            if (GetRandomValue(0, 1))
            {
                boss_room_index = room_index;
            }

            // else (if 0), retain current boss room
        }

        // else, end room's distance is nearer than furthest distance so far
    }

    // set boss room color to red
    rooms[boss_room_index].color = RED;
}

void add_key_and_lock()
{
    // Get path from starting room to boss room
    // traversing from boss room to starting room

    std::set<int> path_to_boss; // indices of rooms in the path from starting to boss room
                                // excluding index 0 (starting rom)

    Room current_room = rooms[boss_room_index];

    // while current room is not yet the starting room, continue traversal
    while(current_room.index != 0)
    {        
        path_to_boss.insert(current_room.index);

        current_room = *(current_room.previous_room);
    }

    // Choosing Location of Key

    // choosing random end room

    // Source: https://www.geeksforgeeks.org/cpp/how-to-access-elements-in-set-by-index-in-cpp/
    int end_room_index = *std::next(end_rooms.begin(), GetRandomValue(0, end_rooms.size()-1));
    Room chosen_room = rooms[end_room_index];

    // then choosing a random room in the path from starting room to end room
    // - excluding starting room among the choices for key location

    int max_key_distance = chosen_room.distance_from_start; // maximum possible distance from key to start

    // if chosen end room is same as boss room,
    // exclude the boss room and the room before it among choices for key location
    if (end_room_index == boss_room_index)
        max_key_distance = chosen_room.distance_from_start - 2;

    int chosen_distance = GetRandomValue(1, max_key_distance);

    int min_lock_distance = 1; // minimum possible distance from locked room to start

    while(chosen_room.distance_from_start != chosen_distance)
    {
        chosen_room = *(chosen_room.previous_room);
    }

    // if chosen key room is located on path from boss room to starting room
    if (path_to_boss.count(chosen_room.index))
    {
        // if chosen key room is right outside the boss room,
        if (chosen_room.distance_from_start == path_to_boss.size()-1)
        {
            // update min lock distance to make the room right outside boss room the locked room
            min_lock_distance = chosen_room.distance_from_start;
            
            // move key room one step towards starting room
            chosen_room = *(chosen_room.previous_room);
        }

        // otherwise,
        else
        {
            // update min lock distance to make locked room further than key room
            min_lock_distance = chosen_room.distance_from_start + 1;
        }
    }

    // else, try to find if path from key to start overlaps with path from boss to start
    else
    {
        Room curr_room = chosen_room;

        // while current room is not the starting room or a room in the path from boss to start,
        // continue traversing
        while(curr_room.index != 0 && !path_to_boss.count(curr_room.index))
        {
            curr_room = *(curr_room.previous_room);
        }

        if (curr_room.index > 0)
        {
            // if current room is right outside the boss room,
            if (curr_room.distance_from_start == path_to_boss.size()-1)
            {
                // update min lock distance to make the room right outside boss room the locked room
                min_lock_distance = curr_room.distance_from_start;
                
                // move key room to the room before current room
                chosen_room = *(curr_room.previous_room);
            }

            // otherwise,
            else
            {
                // update min lock distance to make locked room further than current room
                min_lock_distance = curr_room.distance_from_start + 1;
            }
        }

        // else if found starting room, it does not overlap with path from boss to start, no issues
    }

    // establish chosen room as key room
    key_room_index = chosen_room.index;

    // Choosing Location of Lock
    
    // choosing a random room in the path from starting room to boss room
    // - excluding starting room and boss room
    chosen_distance = GetRandomValue(min_lock_distance, path_to_boss.size()-1);

    Room& locked_room = rooms[*std::next(path_to_boss.begin(), chosen_distance-1)];

    // establish locked room
    locked_room_index = locked_room.index;
}

void generate_dungeon()
{
    // clear data structures that contain rooms / room indices
    active_rooms.clear();
    rooms.clear();
    end_rooms.clear();

    while (!rooms_queue.empty())
        rooms_queue.pop();

    room_count = GetRandomValue(min_room, max_room);
    rooms.reserve(room_count);

    add_new_room(GetRandomValue(0, col_cells_count-1), GetRandomValue(0, row_cells_count-1), SKYBLUE, 0, nullptr);

    // procedural generation proper
    
    while (rooms.size() < room_count)
    {
        // if queue is empty
        if (rooms_queue.empty())
        {
            // push index of starting room (0)
            rooms_queue.push(0);

            // push the indices of each end room
            for (int room_index : end_rooms)
            {
                rooms_queue.push(room_index);
            }
        }

        int current_index = rooms_queue.front();
        Room room = rooms[current_index];
        rooms_queue.pop();

        // try adding room at top
        // if top room is added (depending on what try_add_room returns)
        if (try_add_room(room.x, room.y-1, current_index))
        {
            // modify layout[0] to add passage
            // also modify added room's layout[9] to add the corresponding passage
            // room.layout[0] = "00 00 00 00 00 15 15 00 00 00 00 00";
            rooms[current_index].layout[0].replace(15, 5, "15 15");
            rooms.back().layout[9].replace(15, 5, "15 15");

            // check if we reached the desired number of rooms
            // if reached, end here
            if (rooms.size() == room_count)
                break;
        }

        // try adding room to the left
        // if left room is added
        if (try_add_room(room.x-1, room.y, current_index))
        {
            // modify layout[4] and [5] to add passage to the left
            // also modify added room's layout[4] and [5] to add the corresponding passage
            rooms[current_index].layout[4].replace(0, 2, "15");
            rooms[current_index].layout[5].replace(0, 2, "15");
            rooms.back().layout[4].replace(33, 2, "15");
            rooms.back().layout[5].replace(33, 2, "15");

            // check if we reached the desired number of rooms
            // if reached, end here
            if (rooms.size() == room_count)
                break;
        }

        // try adding room at the bottom
        // if bottom room is added
        if (try_add_room(room.x, room.y+1, current_index))
        {
            // modify layout[9] to add passage
            // also modify added room's layout[0] to add the corresponding passage
            rooms[current_index].layout[9].replace(15, 5, "15 15");
            rooms.back().layout[0].replace(15, 5, "15 15");

            // check if we reached the desired number of rooms
            // if reached, end here
            if (rooms.size() == room_count)
                break;
        }

        // try adding room to the right
        // if right room is added
        if (try_add_room(room.x+1, room.y, current_index))
        {
            // modify layout[4] and [5] to add passage to the right
            // also modify added room's layout[4] and [5] to add the corresponding passage
            rooms[current_index].layout[4].replace(33, 2, "15");
            rooms[current_index].layout[5].replace(33, 2, "15");
            rooms.back().layout[4].replace(0, 2, "15");
            rooms.back().layout[5].replace(0, 2, "15");
        }
        
        // check for reaching desired number of rooms will be handled by while loop
        // unlike for previous attempts to add rooms    
    }

    locate_boss_room();
    add_key_and_lock();
}

int find_room(int x, int y)
{
    int index = 0;

    for (Room room : rooms)
    {
        if (room.x == x && room.y == y)
        {
            return index;
        }

        index++;
    }

    return -1;
}

void generate_map()
{
    // rest map
    for (int i = 0; i < row_cells_count * 10; i++)
        final_layout[i] = "";

    // generate map
    std::string empty_tile = "00 00 00 00 00 00 00 00 00 00 00 00";

    for (int x = 0; x < col_cells_count; x++) 
    {
        for (int y = 0; y < row_cells_count; y++)
        {
            int index = -1;

            if (active_rooms.count(std::to_string(x) + ',' + std::to_string(y)))
            {
                index = find_room(x, y);
            }

            for (int i = 0; i < 10; i++)
            {
                int layout_index = i + (y * 10);

                std::string append = "";

                if (index != -1)
                {
                    final_layout[layout_index] += rooms[index].layout[i];
                }
                else
                {
                    final_layout[layout_index] += empty_tile;
                }

                if (x == col_cells_count - 1) 
                {
                    append = "\n";
                }
                else
                {
                    append = " ";
                }

                final_layout[layout_index] += append;
            }
        }
    }

    print_map();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// MAKE IT 120 BY 100
void print_map()
{
    std::ofstream file("layout.ini");

    file << "// grid\n"
            << 120 << ' ' << 100 << '\n';

    for (int i = 0; i < row_cells_count * 10; i++)
    {
        file << final_layout[i];
    }

    file.close();
}