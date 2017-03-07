#pragma once


#include <functional>
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


} // namespace Game
