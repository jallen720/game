#pragma once


#include <functional>
#include <string>
#include <glm/glm.hpp>


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Room_Data
{
    glm::vec2 origin;
    glm::vec2 bounds;
};


enum class Tile_Types
{
    WALL,
    WALL_CORNER,
    WALL_CORNER_INNER,
    LEFT_DOOR_WALL,
    RIGHT_DOOR_WALL,
    DOOR,
    FLOOR,
    NEXT_FLOOR,
    NONE,
};


struct Tile
{
    Tile_Types type;
    float rotation;
    const std::string * texture_path;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generate_floor(int floor_size);
void destroy_floor();
const glm::vec2 & get_spawn_position();
int get_room(int x, int y);
int get_room(const glm::vec3 & position);
const Room_Data & get_room_data(int room);
void iterate_rooms(const std::function<void(int, int, int &)> & callback);
int get_floor_size();
int get_room_tile_width();
int get_room_tile_height();


} // namespace Game
