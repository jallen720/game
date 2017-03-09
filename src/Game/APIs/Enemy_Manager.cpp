#include "Game/APIs/Enemy_Manager.hpp"

#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Utilities.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class Enemies
{
    TURRET,
    NONE,
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static Enemies * enemies;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generate_enemies()
{
    const int floor_size = get_floor_size();
    const int enemies_width = floor_size * get_room_tile_width();
    const int enemies_height = floor_size * get_room_tile_height();
    enemies = new Enemies[enemies_width * enemies_height];

    iterate_array_2d<Enemies>(enemies, enemies_width, enemies_height, [](int /*x*/, int /*y*/, Enemies & enemy) -> void
    {
        enemy = Enemies::NONE;
    });

    puts("generate_enemies()");
}


void destroy_enemies()
{
    delete[] enemies;
    puts("destroy_enemies()");
}


} // namespace Game
