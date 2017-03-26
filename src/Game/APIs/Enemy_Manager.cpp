#include "Game/APIs/Enemy_Manager.hpp"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/ECS.hpp"
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
using Nito::generate_entity;
using Nito::flag_entity_for_deletion;

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
            if (random(1, 25) == 7)
            {
                enemy = Enemies::TURRET;
            }
        }
    });

    iterate_array_2d<Enemies>(enemies, enemies_width, enemies_height, [](int x, int y, Enemies & enemy) -> void
    {
        if (enemy == Enemies::TURRET)
        {
            static const vector<string> TURRET_SYSTEMS
            {
                "renderer",
                "sprite_dimensions_handler",
                "circle_collider",
                "health",
                "orientation_handler",
                "turret",
                "depth_handler",
            };

            auto transform = new Transform { vec3(x, y, 0) * *room_tile_texture_scale, vec3(1), 0 };

            generate_entity(
                {
                    { "layer"           , new string("enemy")                                                },
                    { "render_layer"    , new string("world")                                                },
                    { "sprite"          , new Sprite { true, "resources/textures/turret_up.png", "texture" } },
                    { "dimensions"      , new Dimensions { 0, 0, vec3(0.5f, 0.5f, 0) }                       },
                    { "transform"       , transform                                                          },
                    { "collider"        , new Collider { true, true, false, {} }                             },
                    { "circle_collider" , new Circle_Collider { 0.2f }                                       },
                    { "health"          , new Health { 100, 100, {} }                                        },
                    {
                        "orientation_handler",
                        new Orientation_Handler
                        {
                            Orientation::DOWN,
                            vec3(0, -1, 0),
                            {
                                { Orientation::LEFT  , "resources/textures/turret_left.png"  },
                                { Orientation::UP    , "resources/textures/turret_up.png"    },
                                { Orientation::RIGHT , "resources/textures/turret_right.png" },
                                { Orientation::DOWN  , "resources/textures/turret_down.png"  },
                            }
                        }
                    },
                },
                TURRET_SYSTEMS);
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
