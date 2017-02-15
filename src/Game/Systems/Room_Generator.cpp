#include "Game/Systems/Room_Generator.hpp"

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Game/Utilities.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;

// glm/glm.hpp
using glm::vec3;
using glm::ivec2;

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

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::remove;
using Cpp_Utils::at_index;


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


using Possible_Rooms = map<char *, ivec2>;


struct Floor
{
    int size;
    char * rooms;
    Possible_Rooms possible_rooms;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const int ROOM_TILE_WIDTH = 13;
static const int ROOM_TILE_HEIGHT = 9;
static const float ROOM_Z = 100.0f;
static Tile room[ROOM_TILE_WIDTH * ROOM_TILE_HEIGHT];


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
static T * map_at(T * map, int width, int x, int y)
{
    return &map[(y * width) + x];
}


template<typename T>
static void iterate_map(T * map, int width, int height, const function<void(int, int, T &)> & callback)
{
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            callback(x, y, *map_at(map, width, x, y));
        }
    }
}


static void iterate_room(const function<void(int, int, Tile &)> & callback)
{
    iterate_map(room, ROOM_TILE_WIDTH, ROOM_TILE_HEIGHT, callback);
}


static void iterate_floor(Floor & floor, const function<void(int, int, char &)> & callback)
{
    iterate_map(floor.rooms, floor.size, floor.size, callback);
}


static void check_possible_room(Floor & floor, Possible_Rooms & room_extensions, int x, int y)
{
    const int size = floor.size;

    if (x < 0 || x >= size ||
        y < 0 || y >= size)
    {
        return;
    }

    char * room = map_at(floor.rooms, size, x, y);

    if (*room == '0' && !contains_key(room_extensions, room))
    {
        ivec2 room_coordinates(x, y);
        room_extensions[room] = room_coordinates;
        floor.possible_rooms[room] = room_coordinates;
    }
}


static void set_room(Floor & floor, Possible_Rooms & room_extensions, int x, int y, char id)
{
    char * room = map_at(floor.rooms, floor.size, x, y);
    *room = id;

    if (contains_key(room_extensions, room))
    {
        remove(room_extensions, room);
    }

    if (contains_key(floor.possible_rooms, room))
    {
        remove(floor.possible_rooms, room);
    }

    check_possible_room(floor, room_extensions, x + 1, y);
    check_possible_room(floor, room_extensions, x - 1, y);
    check_possible_room(floor, room_extensions, x, y + 1);
    check_possible_room(floor, room_extensions, x, y - 1);
}


static void generate_room(Floor & floor, int x, int y, char id, int max_size)
{
    Possible_Rooms room_extensions;
    const int room_size = random(1, max_size + 1);
    int room_generated = 1;
    set_room(floor, room_extensions, x, y, id);

    while (room_generated < room_size)
    {
        // Return if no room extensions could be found (room root is surrounded by other rooms or on the edge of the
        // floor).
        if (room_extensions.size() == 0)
        {
            return;
        }

        ivec2 room_coordinates = at_index(room_extensions, random(0, room_extensions.size())).second;
        set_room(floor, room_extensions, room_coordinates.x, room_coordinates.y, id);
        room_generated++;
    }
}


static void debug_floor(Floor & floor)
{
    const int size = floor.size;
    const char * rooms = floor.rooms;

    for (int i = 0; i < size + 2; i++)
    {
        printf("=");
    }

    printf("\n");

    for (int y = 0; y < size; y++)
    {
        printf("=");

        for (int x = 0; x < size; x++)
        {
            printf("%c", rooms[(y * size) + x]);
        }

        printf("=");
        printf("\n");
    }

    for (int i = 0; i < size + 2; i++)
    {
        printf("=");
    }

    printf("\n\n");
}


Floor create_floor(int size)
{
    Floor floor;
    floor.size = size;
    floor.rooms = new char[size * size];
    return floor;
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


    // Generate floor
    const int floor_size = 6;
    Floor floor = create_floor(floor_size);
    const int root_room_x = random(0, floor_size);
    const int root_room_y = random(0, floor_size);
    Possible_Rooms & possible_rooms = floor.possible_rooms;
    iterate_floor(floor, [](int /*x*/, int /*y*/, char & floor_tile) -> void { floor_tile = '0'; });
    generate_room(floor, root_room_x, root_room_y, '1', 1);

    for (char i = '2'; i < '9'; i++)
    {
        // No possible rooms available
        if (possible_rooms.size() == 0)
        {
            break;
        }

        ivec2 room_coordinates = at_index(possible_rooms, random(0, possible_rooms.size())).second;
        generate_room(floor, room_coordinates.x, room_coordinates.y, i, 4);
    }

    debug_floor(floor);


    // Generate tile types for all tiles on map.
    iterate_room([](int x, int y, Tile & tile) -> void
    {
        // Floor
        if (x > 0 && x < ROOM_TILE_WIDTH - 1 &&
            y > 0 && y < ROOM_TILE_HEIGHT - 1)
        {
            tile.type = Tile::Types::FLOOR;
            tile.texture_path = &FLOOR_TILE_TEXTURE_PATH;
            tile.rotation = 0.0f;
        }
        // Bottom wall
        else if (y == 0 && x != ROOM_TILE_WIDTH - 1)
        {
            if (x == (ROOM_TILE_WIDTH - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (x == (ROOM_TILE_WIDTH - 2) / 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (x == ((ROOM_TILE_WIDTH - 2) / 2) + 2)
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
            if (y == (ROOM_TILE_HEIGHT - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (y == (ROOM_TILE_HEIGHT - 2) / 2)
            {
                tile.type = Tile::Types::LEFT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (y == ((ROOM_TILE_HEIGHT - 2) / 2) + 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (y == ROOM_TILE_HEIGHT - 1)
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
        else if (y == ROOM_TILE_HEIGHT - 1 && x != 0)
        {
            if (x == (ROOM_TILE_WIDTH - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (x == (ROOM_TILE_WIDTH - 2) / 2)
            {
                tile.type = Tile::Types::LEFT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (x == ((ROOM_TILE_WIDTH - 2) / 2) + 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (x == ROOM_TILE_WIDTH - 1)
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
        else if (x == ROOM_TILE_WIDTH - 1 && y != ROOM_TILE_HEIGHT - 1)
        {
            if (y == (ROOM_TILE_HEIGHT - 1) / 2)
            {
                tile.type = Tile::Types::DOOR;
                tile.texture_path = &DOOR_TILE_TEXTURE_PATH;
            }
            else if (y == (ROOM_TILE_HEIGHT - 2) / 2)
            {
                tile.type = Tile::Types::RIGHT_DOOR_WALL;
                tile.texture_path = &WALL_TILE_TEXTURE_PATH;
            }
            else if (y == ((ROOM_TILE_HEIGHT - 2) / 2) + 2)
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
