#pragma once


#include <string>
#include <functional>
#include <glm/glm.hpp>
#include "Nito/APIs/ECS.hpp"


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
    FLOOR_LEDGE,
    FLOOR_HOLE,
    NEXT_FLOOR,
    NONE,
};


struct Tile
{
    Tile_Types type;
    float rotation;
    int room;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void floor_manager_api_init();
void generate_floor(int floor_size);
void destroy_floor();
const glm::vec2 & get_spawn_position();
int get_room(int x, int y);
int get_room(const glm::vec3 & position);
const Room_Data & get_room_data(int room);
const Tile & get_room_tile(int x, int y);
void iterate_rooms(const std::function<void(int, int, int &)> & callback);
void iterate_room_tiles(const std::function<void(int, int, Tile &)> & callback);
int get_floor_size();
int get_room_tile_width();
int get_room_tile_height();
const glm::vec3 & get_room_tile_unit_size();
int get_max_room_id();
int get_spawn_room_id();
void add_enemy(int room_id, Nito::Entity enemy);
void remove_enemy(int room_id, Nito::Entity enemy);
int get_enemy_room(Nito::Entity enemy);
glm::ivec2 get_room_tile_coordinates(const glm::vec2 & position);
glm::vec2 get_room_tile_position(const glm::ivec2 & coordinates);
void add_floor_generated_handler(const std::string & id, const std::function<void()> & handler);
void remove_floor_generated_handler(const std::string & id);


} // namespace Game
