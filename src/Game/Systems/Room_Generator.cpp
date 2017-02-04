#include "Game/Systems/Room_Generator.hpp"

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Graphics.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::Component;
using Nito::generate_entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Dimensions;
using Nito::Sprite;
using Nito::Line_Collider;
using Nito::Polygon_Collider;

// Nito/Collider_Component.hpp
using Nito::Collider;

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
    WALL_BOTTOM,
    WALL_LEFT,
    WALL_TOP,
    WALL_RIGHT,
    WALL_BOTTOM_LEFT,
    WALL_TOP_LEFT,
    WALL_TOP_RIGHT,
    WALL_BOTTOM_RIGHT,
    DOOR_BOTTOM,
    DOOR_LEFT,
    DOOR_TOP,
    DOOR_RIGHT,
    FLOOR,
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const int ROOM_WIDTH = 13;
static const int ROOM_HEIGHT = 9;
static const float ROOM_Z = 100.0f;
static Tile_Types tile_map[ROOM_WIDTH * ROOM_HEIGHT];


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static float get_tile_rotation(Tile_Types tile_type)
{
    switch (tile_type)
    {
        case Tile_Types::FLOOR:
        case Tile_Types::WALL_BOTTOM:
        case Tile_Types::WALL_BOTTOM_LEFT:
        case Tile_Types::DOOR_BOTTOM:
        {
            return 0.0f;
        }
        case Tile_Types::WALL_LEFT:
        case Tile_Types::WALL_TOP_LEFT:
        case Tile_Types::DOOR_LEFT:
        {
            return 270.0f;
        }
        case Tile_Types::WALL_TOP:
        case Tile_Types::WALL_TOP_RIGHT:
        case Tile_Types::DOOR_TOP:
        {
            return 180.0f;
        }
        case Tile_Types::WALL_RIGHT:
        case Tile_Types::WALL_BOTTOM_RIGHT:
        case Tile_Types::DOOR_RIGHT:
        {
            return 90.0f;
        }
        default:
        {
            return 0.0f;
        }
    }
}


static void create_tile(Tile_Types tile_type, const vec3 & position, const string & texture_path)
{
    const float x = position.x;
    const float y = position.y;
    auto dimensions = new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) };
    auto transform = new Transform { vec3(), vec3(1.0f), get_tile_rotation(tile_type) };

    map<string, Component> tile_components
    {
        { "render_layer" , new string("world")                    },
        { "transform"    , transform                              },
        { "dimensions"   , dimensions                             },
        { "sprite"       , new Sprite { texture_path, "texture" } },
    };

    vector<string> tile_systems
    {
        "sprite_dimensions_handler",
        "renderer",
    };


    // Give wall tiles a line collider.
    if (tile_type == Tile_Types::WALL_BOTTOM ||
        tile_type == Tile_Types::WALL_LEFT ||
        tile_type == Tile_Types::WALL_TOP ||
        tile_type == Tile_Types::WALL_RIGHT)
    {
        bool right_of_door = false;
        bool left_of_door = false;

        if (y == 0)
        {
            if (x == (ROOM_WIDTH - 2) / 2)
            {
                right_of_door = true;
            }
            else if (x == ((ROOM_WIDTH - 2) / 2) + 2)
            {
                left_of_door = true;
            }
        }
        else if (y == ROOM_HEIGHT - 1)
        {
            if (x == (ROOM_WIDTH - 2) / 2)
            {
                left_of_door = true;
            }
            else if (x == ((ROOM_WIDTH - 2) / 2) + 2)
            {
                right_of_door = true;
            }
        }
        else if (x == 0)
        {
            if (y == (ROOM_HEIGHT - 2) / 2)
            {
                left_of_door = true;
            }
            else if (y == ((ROOM_HEIGHT - 2) / 2) + 2)
            {
                right_of_door = true;
            }
        }
        else if (x == ROOM_WIDTH - 1)
        {
            if (y == (ROOM_HEIGHT - 2) / 2)
            {
                right_of_door = true;
            }
            else if (y == ((ROOM_HEIGHT - 2) / 2) + 2)
            {
                left_of_door = true;
            }
        }

        tile_components["collider"] = new Collider { true, true, false, {} };


        // Door-adjacent walls should have a polygon collider, all other walls shouls have a line collider.
        if (right_of_door || left_of_door)
        {
            static const vector<vec3> RIGHT_WALL_POINTS
            {
                vec3(-0.25f, 0.25f, 0.0f),
                vec3( 0.25f, 0.25f, 0.0f),
                vec3( 0.25f,-0.25f, 0.0f),
            };

            static const vector<vec3> LEFT_WALL_POINTS
            {
                vec3(-0.25f,-0.25f, 0.0f),
                vec3(-0.25f, 0.25f, 0.0f),
                vec3( 0.25f, 0.25f, 0.0f),
            };

            tile_components["polygon_collider"] = new Polygon_Collider
            {
                right_of_door ? RIGHT_WALL_POINTS : LEFT_WALL_POINTS,
                false,
            };

            tile_systems.push_back("polygon_collider");
        }
        else
        {
            tile_components["line_collider"] = new Line_Collider { vec3(-0.25f, 0.25f, 0.0f), vec3(0.25f, 0.25f, 0.0f) };
            tile_systems.push_back("line_collider");
        }
    }


    generate_entity(tile_components, tile_systems);
    transform->position = position * (vec3(dimensions->width, dimensions->height, 0.0f) / get_pixels_per_unit());
    transform->position.z = ROOM_Z;
}


static void iterate_tile_map(const function<void(int, int, Tile_Types & tile_type)> & callback)
{
    for (int x = 0; x < ROOM_WIDTH; x++)
    {
        for (int y = 0; y < ROOM_HEIGHT; y++)
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
    static const string WALL_TILE_TEXTURE_PATH = "resources/textures/tiles/wall.png";
    static const string WALL_CORNER_TILE_TEXTURE_PATH = "resources/textures/tiles/wall_corner.png";
    static const string DOOR_TILE_TEXTURE_PATH = "resources/textures/tiles/door.png";
    static const string FLOOR_TILE_TEXTURE_PATH = "resources/textures/tiles/floor.png";


    // Generate tile types for all tiles on map.
    iterate_tile_map([](int x, int y, Tile_Types & tile_type) -> void
    {
        // Left wall
        if (x == 0 && y != 0 && y != ROOM_HEIGHT - 1)
        {
            if (y == (ROOM_HEIGHT - 1) / 2)
            {
                tile_type = Tile_Types::DOOR_LEFT;
            }
            else
            {
                tile_type = Tile_Types::WALL_LEFT;
            }
        }
        // Top wall
        else if (y == ROOM_HEIGHT - 1 && x != 0 && x != ROOM_WIDTH - 1)
        {
            if (x == (ROOM_WIDTH - 1) / 2)
            {
                tile_type = Tile_Types::DOOR_TOP;
            }
            else
            {
                tile_type = Tile_Types::WALL_TOP;
            }
        }
        // Right wall
        else if (x == ROOM_WIDTH - 1 && y != 0 && y != ROOM_HEIGHT - 1)
        {
            if (y == (ROOM_HEIGHT - 1) / 2)
            {
                tile_type = Tile_Types::DOOR_RIGHT;
            }
            else
            {
                tile_type = Tile_Types::WALL_RIGHT;
            }
        }
        // Bottom wall
        else if (y == 0 && x != 0 && x != ROOM_WIDTH - 1)
        {
            if (x == (ROOM_WIDTH - 1) / 2)
            {
                tile_type = Tile_Types::DOOR_BOTTOM;
            }
            else
            {
                tile_type = Tile_Types::WALL_BOTTOM;
            }
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


    // Create tile entities based on each tile's type.
    iterate_tile_map([&](int x, int y, const Tile_Types & tile_type) -> void
    {
        const vec3 tile_position(x, y, 0.0f);

        switch (tile_type)
        {
            case Tile_Types::WALL_BOTTOM:
            case Tile_Types::WALL_LEFT:
            case Tile_Types::WALL_TOP:
            case Tile_Types::WALL_RIGHT:
            {
                create_tile(tile_type, tile_position, WALL_TILE_TEXTURE_PATH);
                break;
            }
            case Tile_Types::WALL_BOTTOM_LEFT:
            case Tile_Types::WALL_TOP_LEFT:
            case Tile_Types::WALL_TOP_RIGHT:
            case Tile_Types::WALL_BOTTOM_RIGHT:
            {
                create_tile(tile_type, tile_position, WALL_CORNER_TILE_TEXTURE_PATH);
                break;
            }
            case Tile_Types::DOOR_BOTTOM:
            case Tile_Types::DOOR_LEFT:
            case Tile_Types::DOOR_TOP:
            case Tile_Types::DOOR_RIGHT:
            {
                create_tile(tile_type, tile_position, DOOR_TILE_TEXTURE_PATH);
                break;
            }
            case Tile_Types::FLOOR:
            {
                create_tile(tile_type, tile_position, FLOOR_TILE_TEXTURE_PATH);
                break;
            }
        }
    });
}


void room_generator_unsubscribe(Entity /*entity*/) {}


} // namespace Game
