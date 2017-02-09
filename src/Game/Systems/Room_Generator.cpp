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
struct Tile
{
    enum class Types
    {
        WALL,
        WALL_CORNER,
        LEFT_DOOR_WALL,
        RIGHT_DOOR_WALL,
        DOOR,
        FLOOR,
    }
    type;

    float rotation;
    const string * texture_path;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const int ROOM_WIDTH = 13;
static const int ROOM_HEIGHT = 9;
static const float ROOM_Z = 100.0f;
static Tile room[ROOM_WIDTH * ROOM_HEIGHT];


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
static void iterate_map(T * map, int width, int height, const function<void(int, int, T &)> & callback)
{
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            callback(x, y, map[(y * width) + x]);
        }
    }
}


static void iterate_room(const function<void(int, int, Tile &)> & callback)
{
    iterate_map(room, ROOM_WIDTH, ROOM_HEIGHT, callback);
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
    iterate_room([](int x, int y, Tile & tile) -> void
    {
        // Floor
        if (x > 0 && x < ROOM_WIDTH - 1 &&
            y > 0 && y < ROOM_HEIGHT - 1)
        {
            tile.type = Tile::Types::FLOOR;
            tile.texture_path = &FLOOR_TILE_TEXTURE_PATH;
            tile.rotation = 0.0f;
        }
        // Bottom wall
        else if (y == 0 && x != ROOM_WIDTH - 1)
        {
            if (x == (ROOM_WIDTH - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (x == (ROOM_WIDTH - 2) / 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (x == ((ROOM_WIDTH - 2) / 2) + 2)
            {
                tile.type = Tile::Types::LEFT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (x == 0)
            {
                tile.type = Tile::Types::WALL_CORNER;
                tile.texture_path = &WALL_CORNER_TILE_TEXTURE_PATH;
            }
            else
            {
                tile.type = Tile::Types::WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }

            tile.rotation = 0.0f;
        }
        // Left wall
        else if (x == 0 && y != 0)
        {
            if (y == (ROOM_HEIGHT - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (y == (ROOM_HEIGHT - 2) / 2)
            {
                tile.type = Tile::Types::LEFT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (y == ((ROOM_HEIGHT - 2) / 2) + 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (y == ROOM_HEIGHT - 1)
            {
                tile.type = Tile::Types::WALL_CORNER;
                tile.texture_path = &WALL_CORNER_TILE_TEXTURE_PATH;
            }
            else
            {
                tile.type = Tile::Types::WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }

            tile.rotation = 270.0f;
        }
        // Top wall
        else if (y == ROOM_HEIGHT - 1 && x != 0)
        {
            if (x == (ROOM_WIDTH - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (x == (ROOM_WIDTH - 2) / 2)
            {
                tile.type = Tile::Types::LEFT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (x == ((ROOM_WIDTH - 2) / 2) + 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (x == ROOM_WIDTH - 1)
            {
                tile.type = Tile::Types::WALL_CORNER;
                tile.texture_path = &WALL_CORNER_TILE_TEXTURE_PATH;
            }
            else
            {
                tile.type = Tile::Types::WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }

            tile.rotation = 180.0f;
        }
        // Right wall
        else if (x == ROOM_WIDTH - 1 && y != ROOM_HEIGHT - 1)
        {
            if (y == (ROOM_HEIGHT - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (y == (ROOM_HEIGHT - 2) / 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (y == ((ROOM_HEIGHT - 2) / 2) + 2)
            {
                tile.type = Tile::Types::LEFT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (y == 0)
            {
                tile.type = Tile::Types::WALL_CORNER;
                tile.texture_path = &WALL_CORNER_TILE_TEXTURE_PATH;
            }
            else
            {
                tile.type = Tile::Types::WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }

            tile.rotation = 90.0f;
        }
    });


    // Create tile entities based on each tile's type.
    iterate_room([](int tile_x, int tile_y, const Tile & tile) -> void
    {
        auto dimensions = new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) };
        auto transform = new Transform { vec3(), vec3(1.0f), tile.rotation };
        const Tile::Types tile_type = tile.type;

        map<string, Component> tile_components
        {
            { "render_layer" , new string("world")                          },
            { "transform"    , transform                                    },
            { "dimensions"   , dimensions                                   },
            { "sprite"       , new Sprite { *tile.texture_path, "texture" } },
        };

        vector<string> tile_systems
        {
            "sprite_dimensions_handler",
            "renderer",
        };


        // Give wall tiles a line collider.
        if (tile_type == Tile::Types::WALL ||
            tile_type == Tile::Types::RIGHT_DOOR_WALL ||
            tile_type == Tile::Types::LEFT_DOOR_WALL)
        {
            tile_components["collider"] = new Collider { true, true, false, {} };


            // Door-adjacent walls should have a polygon collider, all other walls shouls have a line collider.
            if (tile_type == Tile::Types::RIGHT_DOOR_WALL ||
                tile_type == Tile::Types::LEFT_DOOR_WALL)
            {
                static const vector<vec3> RIGHT_DOOR_WALL_POINTS
                {
                    vec3(-0.25f, 0.25f, 0.0f),
                    vec3( 0.25f, 0.25f, 0.0f),
                    vec3( 0.25f,-0.25f, 0.0f),
                };

                static const vector<vec3> LEFT_DOOR_WALL_POINTS
                {
                    vec3(-0.25f,-0.25f, 0.0f),
                    vec3(-0.25f, 0.25f, 0.0f),
                    vec3( 0.25f, 0.25f, 0.0f),
                };

                tile_components["polygon_collider"] = new Polygon_Collider
                {
                    tile_type == Tile::Types::RIGHT_DOOR_WALL ? RIGHT_DOOR_WALL_POINTS : LEFT_DOOR_WALL_POINTS,
                    false,
                };

                tile_systems.push_back("polygon_collider");
            }
            else
            {
                tile_components["line_collider"] = new Line_Collider
                {
                    vec3(-0.25f, 0.25f, 0.0f),
                    vec3(0.25f, 0.25f, 0.0f),
                };

                tile_systems.push_back("line_collider");
            }
        }


        generate_entity(tile_components, tile_systems);

        transform->position =
            vec3(tile_x, tile_y, 0.0f) * (vec3(dimensions->width, dimensions->height, 0.0f) / get_pixels_per_unit());

        transform->position.z = ROOM_Z;
    });
}


void room_generator_unsubscribe(Entity /*entity*/) {}


} // namespace Game
