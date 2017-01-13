#include "Game/Systems/Room_Generator.hpp"

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::generate_entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Dimensions;
using Nito::Sprite;

// Nito/APIs/Graphics.hpp
using Nito::get_pixels_per_unit;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class Tile_Types
{
    FLOOR,
    WALL_LEFT,
    WALL_TOP,
    WALL_RIGHT,
    WALL_BOTTOM,
    WALL_TOP_LEFT,
    WALL_TOP_RIGHT,
    WALL_BOTTOM_RIGHT,
    WALL_BOTTOM_LEFT,
    DOOR_LEFT,
    DOOR_TOP,
    DOOR_RIGHT,
    DOOR_BOTTOM,
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const auto ROOM_WIDTH = 13u;
static const auto ROOM_HEIGHT = 9u;
static Tile_Types tile_map[ROOM_WIDTH * ROOM_HEIGHT];


static const map<Tile_Types, const string> TILE_TYPE_TEXTURE_PATHS
{
    { Tile_Types::FLOOR             , "resources/textures/floor.png"             },
    { Tile_Types::WALL_LEFT         , "resources/textures/wall_left.png"         },
    { Tile_Types::WALL_TOP          , "resources/textures/wall_top.png"          },
    { Tile_Types::WALL_RIGHT        , "resources/textures/wall_right.png"        },
    { Tile_Types::WALL_BOTTOM       , "resources/textures/wall_bottom.png"       },
    { Tile_Types::WALL_TOP_LEFT     , "resources/textures/wall_top_left.png"     },
    { Tile_Types::WALL_TOP_RIGHT    , "resources/textures/wall_top_right.png"    },
    { Tile_Types::WALL_BOTTOM_RIGHT , "resources/textures/wall_bottom_right.png" },
    { Tile_Types::WALL_BOTTOM_LEFT  , "resources/textures/wall_bottom_left.png"  },
    { Tile_Types::DOOR_LEFT         , "resources/textures/floor.png"/*door_left.png"*/         },
    { Tile_Types::DOOR_TOP          , "resources/textures/floor.png"/*door_top.png"*/          },
    { Tile_Types::DOOR_RIGHT        , "resources/textures/floor.png"/*door_right.png"*/        },
    { Tile_Types::DOOR_BOTTOM       , "resources/textures/floor.png"/*door_bottom.png"*/       },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void create_tile(const vec3 & position, const string & texture_path)
{
    static const vector<string> TILE_SYSTEMS
    {
        "sprite_dimensions_handler",
        "renderer",
    };

    static const float ROOM_Z = 100.0f;

    auto dimensions = new Dimensions { 0.0f, 0.0f, vec3(0.0f) };
    auto transform = new Transform { vec3(), vec3(1.0f), 0.0f };

    generate_entity(
        {
            { "render_layer" , new string("world")                    },
            { "transform"    , transform                              },
            { "dimensions"   , dimensions                             },
            { "sprite"       , new Sprite { texture_path, "texture" } },
        },
        TILE_SYSTEMS);

    transform->position = position * (vec3(dimensions->width, dimensions->height, 0.0f) / get_pixels_per_unit());
    transform->position.z = ROOM_Z;
}


static void iterate_tile_map(const function<void(int, int, Tile_Types & tile_type)> & callback)
{
    for (auto x = 0u; x < ROOM_WIDTH; x++)
    {
        for (auto y = 0u; y < ROOM_HEIGHT; y++)
        {
            callback(x, y, tile_map[(y * ROOM_WIDTH) + x]);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void room_generator_subscribe(Entity /*entity*/)
{
    iterate_tile_map([](int x, int y, Tile_Types & tile_type) -> void
    {
        // Left wall
        if (x == 0 && y != 0 && y != ROOM_HEIGHT - 1)
        {
            tile_type = Tile_Types::WALL_LEFT;
        }
        // Top wall
        else if (y == ROOM_HEIGHT - 1 && x != 0 && x != ROOM_WIDTH - 1)
        {
            tile_type = Tile_Types::WALL_TOP;
        }
        // Right wall
        else if (x == ROOM_WIDTH - 1 && y != 0 && y != ROOM_HEIGHT - 1)
        {
            tile_type = Tile_Types::WALL_RIGHT;
        }
        // Bottom wall
        else if (y == 0 && x != 0 && x != ROOM_WIDTH - 1)
        {
            tile_type = Tile_Types::WALL_BOTTOM;
        }
        // Top left wall
        else if (x == 0 && y == ROOM_HEIGHT - 1)
        {
            tile_type = Tile_Types::WALL_TOP_LEFT;
        }
        // Top right wall
        else if (x == ROOM_WIDTH - 1 && y == ROOM_HEIGHT - 1)
        {
            tile_type = Tile_Types::WALL_TOP_RIGHT;
        }
        // Bottom right wall
        else if (x == ROOM_WIDTH - 1 && y == 0)
        {
            tile_type = Tile_Types::WALL_BOTTOM_RIGHT;
        }
        // Bottom left wall
        else if (x == 0 && y == 0)
        {
            tile_type = Tile_Types::WALL_BOTTOM_LEFT;
        }
        // Floor
        else
        {
            tile_type = Tile_Types::FLOOR;
        }
    });

    iterate_tile_map([&](int x, int y, const Tile_Types & tile_type) -> void
    {
        create_tile(vec3(x, y, 0.0f), TILE_TYPE_TEXTURE_PATHS.at(tile_type));
    });
}


void room_generator_unsubscribe(Entity /*entity*/) {}


} // namespace Game
