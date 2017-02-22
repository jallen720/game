#include "Game/APIs/Floor_Generator.hpp"

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Game/Utilities.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::ivec2;

// Nito/APIs/ECS.hpp
using Nito::Component;
using Nito::generate_entity;

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
        WALL_CORNER_INNER,
        LEFT_DOOR_WALL,
        RIGHT_DOOR_WALL,
        DOOR,
        FLOOR,
        NONE,
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
    Tile * room_tiles;
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
static const vec3 ROOM_TILE_TEXTURE_SCALE(0.5f, 0.5f, 1.0f);
static vec2 spawn_position;
static map<char, Room_Data> room_datas;
static Floor current_floor;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
static T * array_2d_at(T * array_2d, int width, int x, int y)
{
    return &array_2d[(y * width) + x];
}


template<typename T>
static void iterate_array_2d(
    T * array_2d,
    int width,
    int start_x,
    int start_y,
    int sub_width,
    int sub_height,
    const function<void(int, int, T &)> & callback)
{
    for (int x = start_x; x < start_x + sub_width; x++)
    {
        for (int y = start_y; y < start_y + sub_height; y++)
        {
            callback(x - start_x, y - start_y, *array_2d_at(array_2d, width, x, y));
        }
    }
}


template<typename T>
static void iterate_array_2d(T * array_2d, int width, int height, const function<void(int, int, T &)> & callback)
{
    iterate_array_2d(array_2d, width, 0, 0, width, height, callback);
}


static void iterate_rooms(Floor & floor, const function<void(int, int, char &)> & callback)
{
    iterate_array_2d(floor.rooms, floor.size, floor.size, callback);
}


static void iterate_room_tiles(Floor & floor, const function<void(int, int, Tile &)> & callback)
{
    iterate_array_2d(floor.room_tiles, floor.size * ROOM_TILE_WIDTH, floor.size * ROOM_TILE_HEIGHT, callback);
}


static void iterate_room_tiles(
    Floor & floor,
    int room_x,
    int room_y,
    const function<void(int, int, Tile &)> & callback)
{
    iterate_array_2d(
        floor.room_tiles,
        floor.size * ROOM_TILE_WIDTH,
        room_x * ROOM_TILE_WIDTH,
        room_y * ROOM_TILE_HEIGHT,
        ROOM_TILE_WIDTH,
        ROOM_TILE_HEIGHT,
        callback);
}


static void check_possible_room(Floor & floor, Possible_Rooms & room_extensions, int x, int y)
{
    const int size = floor.size;

    if (x < 0 || x >= size ||
        y < 0 || y >= size)
    {
        return;
    }

    char * room = array_2d_at(floor.rooms, size, x, y);

    if (*room == '0' && !contains_key(room_extensions, room))
    {
        ivec2 room_coordinates(x, y);
        room_extensions[room] = room_coordinates;
        floor.possible_rooms[room] = room_coordinates;
    }
}


static void set_room(Floor & floor, Possible_Rooms & room_extensions, int x, int y, char id)
{
    char * room = array_2d_at(floor.rooms, floor.size, x, y);
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


static float get_room_center_coordinate(int room_coordinate, int tile_dimension_size, float texture_dimension_scale)
{
    return ((room_coordinate * tile_dimension_size) + (tile_dimension_size / 2)) * texture_dimension_scale;
}


static vec2 get_room_center(int room_x, int room_y)
{
    return vec2(
        get_room_center_coordinate(room_x, ROOM_TILE_WIDTH, ROOM_TILE_TEXTURE_SCALE.x),
        get_room_center_coordinate(room_y, ROOM_TILE_HEIGHT, ROOM_TILE_TEXTURE_SCALE.y));
}


static void generate_room(Floor & floor, int x, int y, char id, int max_size)
{
    Possible_Rooms room_extensions;
    Room_Data & room_data = room_datas[id];
    vec2 & room_origin = room_data.origin;
    vec2 & room_bounds = room_data.bounds;
    int room_origin_x = x;
    int room_origin_y = y;
    int room_bounds_x = x;
    int room_bounds_y = y;
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
        float room_coordinates_x = room_coordinates.x;
        float room_coordinates_y = room_coordinates.y;
        set_room(floor, room_extensions, room_coordinates_x, room_coordinates_y, id);
        room_generated++;


        // Update room origin and bounds.
        if (room_coordinates_x < room_origin_x)
        {
            room_origin_x = room_coordinates_x;
        }
        else if (room_coordinates_x > room_bounds_x)
        {
            room_bounds_x = room_coordinates_x;
        }

        if (room_coordinates_y < room_origin_y)
        {
            room_origin_y = room_coordinates_y;
        }
        else if (room_coordinates_y > room_bounds_y)
        {
            room_bounds_y = room_coordinates_y;
        }
    }


    // Adjust room origin and bounds based on tile size.
    room_origin = get_room_center(room_origin_x, room_origin_y);
    room_bounds = get_room_center(room_bounds_x, room_bounds_y);
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

    for (int y = size - 1; y >= 0; y--)
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


static Floor create_floor(int size)
{
    Floor floor;
    floor.size = size;
    floor.rooms = new char[size * size];
    floor.room_tiles = new Tile[(size * ROOM_TILE_WIDTH) * (size * ROOM_TILE_HEIGHT)];
    return floor;
}


static void set_tile_type(Tile & tile, Tile::Types type)
{
    static const string WALL_TEXTURE_PATH = "resources/textures/tiles/wall.png";
    static const string WALL_CORNER_TEXTURE_PATH = "resources/textures/tiles/wall_corner.png";
    static const string WALL_CORNER_INNER_TEXTURE_PATH = "resources/textures/tiles/wall_corner_inner.png";
    static const string DOOR_TEXTURE_PATH = "resources/textures/tiles/door.png";
    static const string FLOOR_TEXTURE_PATH = "resources/textures/tiles/floor.png";

    static const map<Tile::Types, const string *> TILE_TYPE_TEXTURE_PATHS
    {
        { Tile::Types::WALL              , &WALL_TEXTURE_PATH              },
        { Tile::Types::WALL_CORNER       , &WALL_CORNER_TEXTURE_PATH       },
        { Tile::Types::WALL_CORNER_INNER , &WALL_CORNER_INNER_TEXTURE_PATH },
        { Tile::Types::DOOR              , &DOOR_TEXTURE_PATH              },
        { Tile::Types::FLOOR             , &FLOOR_TEXTURE_PATH             },
        { Tile::Types::LEFT_DOOR_WALL    , &WALL_TEXTURE_PATH              },
        { Tile::Types::RIGHT_DOOR_WALL   , &WALL_TEXTURE_PATH              },
    };

    tile.type = type;
    tile.texture_path = TILE_TYPE_TEXTURE_PATHS.at(type);
}


static void generate_wall_tile(
    Tile & tile,
    int coordinate,
    int dimension_size,
    char room,
    char neighbour,
    char clockwise_neighbour,
    float rotation,
    bool inverted = false)
{
    tile.rotation = rotation;

    // Corner
    if ((inverted && coordinate == dimension_size - 1) || coordinate == 0)
    {
        // Inner corner
        if (neighbour == room && clockwise_neighbour == room)
        {
            set_tile_type(tile, Tile::Types::WALL_CORNER_INNER);
        }
        // Wall to neighbour
        else if (neighbour == room)
        {
            set_tile_type(tile, Tile::Types::WALL);
            tile.rotation -= 90.0f;
        }
        // Wall to clockwise_neighbour
        else if (clockwise_neighbour == room)
        {
            set_tile_type(tile, Tile::Types::WALL);
        }
        // Outer corner
        else
        {
            set_tile_type(tile, Tile::Types::WALL_CORNER);
        }
    }
    // Floor between rooms
    else if (neighbour == room)
    {
        set_tile_type(tile, Tile::Types::FLOOR);
        tile.rotation = 0.0f;
    }
    // Wall
    else if (neighbour != '0')
    {
        // Door
        if (coordinate == (dimension_size - 1) / 2)
        {
            set_tile_type(tile, Tile::Types::DOOR);
        }
        // Door-adjacent wall
        else if (coordinate == (dimension_size - 2) / 2)
        {
            set_tile_type(tile, inverted ? Tile::Types::LEFT_DOOR_WALL : Tile::Types::RIGHT_DOOR_WALL);
        }
        // Door-adjacent wall
        else if (coordinate == ((dimension_size - 2) / 2) + 2)
        {
            set_tile_type(tile, inverted ? Tile::Types::RIGHT_DOOR_WALL : Tile::Types::LEFT_DOOR_WALL);
        }
        // Normal wall
        else
        {
            set_tile_type(tile, Tile::Types::WALL);
        }
    }
    // Normal wall
    else
    {
        set_tile_type(tile, Tile::Types::WALL);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generate_floor()
{
    // Create floor.
    const int floor_size = 6;
    current_floor = create_floor(floor_size);
    Possible_Rooms & possible_rooms = current_floor.possible_rooms;
    iterate_rooms(current_floor, [](int /*x*/, int /*y*/, char & room) -> void { room = '0'; });

    iterate_room_tiles(current_floor, [](int /*x*/, int /*y*/, Tile & room_tile) -> void
    {
        room_tile.type = Tile::Types::NONE;
        room_tile.rotation = 0.0f;
    });


    // Generate rooms.
    const int root_room_x = random(0, floor_size);
    const int root_room_y = random(0, floor_size);
    generate_room(current_floor, root_room_x, root_room_y, '1', 1);

    for (char i = '2'; i < '9'; i++)
    {
        // No possible rooms available
        if (possible_rooms.size() == 0)
        {
            break;
        }

        ivec2 room_coordinates = at_index(possible_rooms, random(0, possible_rooms.size())).second;
        generate_room(current_floor, room_coordinates.x, room_coordinates.y, i, 4);
    }

    spawn_position = get_room_center(root_room_x, root_room_y);
    debug_floor(current_floor);


    // Generate room tiles.
    iterate_rooms(current_floor, [&](int room_x, int room_y, char & room) -> void
    {
        // Don't generate tiles for empty rooms.
        if (room == '0')
        {
            return;
        }


        iterate_room_tiles(current_floor, room_x, room_y, [&](int x, int y, Tile & tile) -> void
        {
            // Floor
            if (x > 0 && x < ROOM_TILE_WIDTH - 1 &&
                y > 0 && y < ROOM_TILE_HEIGHT - 1)
            {
                set_tile_type(tile, Tile::Types::FLOOR);
                tile.rotation = 0.0f;
            }
            // Wall
            else
            {
                const char bottom_neighbour =
                    room_y > 0
                    ? *array_2d_at(current_floor.rooms, current_floor.size, room_x, room_y - 1)
                    : '0';

                const char left_neighbour =
                    room_x > 0
                    ? *array_2d_at(current_floor.rooms, current_floor.size, room_x - 1, room_y)
                    : '0';

                const char top_neighbour =
                    room_y < floor_size - 1
                    ? *array_2d_at(current_floor.rooms, current_floor.size, room_x, room_y + 1)
                    : '0';

                const char right_neighbour =
                    room_x < floor_size - 1
                    ? *array_2d_at(current_floor.rooms, current_floor.size, room_x + 1, room_y)
                    : '0';

                // Bottom wall
                if (y == 0 && x != ROOM_TILE_WIDTH - 1)
                {
                    generate_wall_tile(tile, x, ROOM_TILE_WIDTH, room, bottom_neighbour, left_neighbour, 0.0f);
                }
                // Left wall
                else if (x == 0 && y != 0)
                {
                    generate_wall_tile(tile, y, ROOM_TILE_HEIGHT, room, left_neighbour, top_neighbour, 270.0f, true);
                }
                // Top wall
                else if (y == ROOM_TILE_HEIGHT - 1 && x != 0)
                {
                    generate_wall_tile(tile, x, ROOM_TILE_WIDTH, room, top_neighbour, right_neighbour, 180.0f, true);
                }
                // Right wall
                else if (x == ROOM_TILE_WIDTH - 1 && y != ROOM_TILE_HEIGHT - 1)
                {
                    generate_wall_tile(tile, y, ROOM_TILE_HEIGHT, room, right_neighbour, bottom_neighbour, 90.0f);
                }
            }
        });
    });


    // Create tile entities based on each tile's type.
    iterate_room_tiles(current_floor, [](int tile_x, int tile_y, const Tile & tile) -> void
    {
        if (tile.type == Tile::Types::NONE)
        {
            return;
        }

        auto transform = new Transform { vec3(), vec3(1.0f), tile.rotation };
        const Tile::Types tile_type = tile.type;

        map<string, Component> tile_components
        {
            { "render_layer" , new string("world")                                   },
            { "transform"    , transform                                             },
            { "dimensions"   , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) } },
            { "sprite"       , new Sprite { *tile.texture_path, "texture" }          },
        };

        vector<string> tile_systems
        {
            "sprite_dimensions_handler",
            "renderer",
        };


        // Give wall tiles a line collider.
        if (tile_type == Tile::Types::WALL ||
            tile_type == Tile::Types::RIGHT_DOOR_WALL ||
            tile_type == Tile::Types::LEFT_DOOR_WALL ||
            tile_type == Tile::Types::WALL_CORNER_INNER)
        {
            tile_components["collider"] = new Collider { true, true, false, {} };


            // Door-adjacent walls should have a polygon collider, all other walls shouls have a line collider.
            if (tile_type == Tile::Types::RIGHT_DOOR_WALL ||
                tile_type == Tile::Types::LEFT_DOOR_WALL ||
                tile_type == Tile::Types::WALL_CORNER_INNER)
            {
                static const map<Tile::Types, const vector<vec3>> POINTS
                {
                    {
                        Tile::Types::RIGHT_DOOR_WALL,
                        {
                            vec3(-0.25f, 0.25f, 0.0f),
                            vec3( 0.25f, 0.25f, 0.0f),
                            vec3( 0.25f,-0.25f, 0.0f),
                        }
                    },
                    {
                        Tile::Types::LEFT_DOOR_WALL,
                        {
                            vec3(-0.25f,-0.25f, 0.0f),
                            vec3(-0.25f, 0.25f, 0.0f),
                            vec3( 0.25f, 0.25f, 0.0f),
                        }
                    },
                    {
                        Tile::Types::WALL_CORNER_INNER,
                        {
                            vec3(-0.25f , 0.25f , 0.0f),
                            vec3( 0.05f , 0.25f , 0.0f),
                            vec3( 0.25f , 0.05f , 0.0f),
                            vec3( 0.25f ,-0.25f , 0.0f),
                        }
                    },
                };

                tile_components["polygon_collider"] = new Polygon_Collider { POINTS.at(tile_type), false };
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
        transform->position = vec3(tile_x, tile_y, 0.0f) * ROOM_TILE_TEXTURE_SCALE;
        transform->position.z = ROOM_Z;
    });
}


const vec2 & get_spawn_position()
{
    return spawn_position;
}


char get_room(const vec3 & position)
{
    return *array_2d_at(
        current_floor.rooms,
        current_floor.size,
        (position.x + (ROOM_TILE_TEXTURE_SCALE.x / 2.0f)) / (ROOM_TILE_WIDTH * ROOM_TILE_TEXTURE_SCALE.x),
        (position.y + (ROOM_TILE_TEXTURE_SCALE.y / 2.0f)) / (ROOM_TILE_HEIGHT * ROOM_TILE_TEXTURE_SCALE.y));
}


const Room_Data & get_room_data(char room)
{
    return room_datas.at(room);
}


} // namespace Game
