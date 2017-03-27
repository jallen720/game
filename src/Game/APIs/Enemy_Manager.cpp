#include "Game/APIs/Enemy_Manager.hpp"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Utilities.hpp"
#include "Game/Components.hpp"


using std::vector;
using std::string;

// glm/glm.hpp
using glm::vec3;

// Nito/Components.hpp
using Nito::Sprite;
using Nito::Dimensions;
using Nito::Transform;
using Nito::Circle_Collider;

// Nito/Collider_Component.hpp
using Nito::Collider;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::flag_entity_for_deletion;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


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
static const vec3 * room_tile_texture_scale;
static vector<Entity> enemy_entities;


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
    room_tile_texture_scale = &get_room_tile_texture_scale();

    iterate_array_2d<Enemies>(enemies, enemies_width, enemies_height, [](int x, int y, Enemies & enemy) -> void
    {
        enemy = Enemies::NONE;

        if (get_room_tile(x, y).type == Tile_Types::FLOOR)
        {
            if (random(1, 50) == 7)
            {
                enemy = Enemies::TURRET;
            }
        }
    });

    iterate_array_2d<Enemies>(enemies, enemies_width, enemies_height, [](int x, int y, Enemies & enemy) -> void
    {
        if (enemy == Enemies::TURRET)
        {
            Entity turret = load_blueprint("turret");
            ((Transform *)get_component(turret, "transform"))->position = vec3(x, y, 0) * *room_tile_texture_scale;
            enemy_entities.push_back(turret);
        }
    });
}


void destroy_enemies()
{
    delete[] enemies;
    for_each(enemy_entities, flag_entity_for_deletion);
    enemy_entities.clear();
}


} // namespace Game
