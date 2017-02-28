#include "Game/APIs/Floor_Manager.hpp"

#include <string>
#include <vector>
#include <map>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Systems/Game_Manager.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::ivec2;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::Component;
using Nito::get_component;
using Nito::has_component;
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

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


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


using Possible_Rooms = map<int *, ivec2>;


struct Floor
{
    int size;
    int rooms_size;
    int room_tiles_size;
    int * rooms;
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
static const string FLOOR_MANAGER_ROOM_CHANGE_HANDLER_ID("floor_manager");
static vec2 spawn_position;
static map<int, Room_Data> room_datas;
static Floor current_floor;
static int floor_size;
static map<int, vector<bool *>> room_tile_render_flags;


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
    bool relative_coordinates,
    const function<void(int, int, T &)> & callback)
{
    for (int x = start_x; x < start_x + sub_width; x++)
    {
        for (int y = start_y; y < start_y + sub_height; y++)
        {
            int x_coordinate = x;
            int y_coordinate = y;

            if (relative_coordinates)
            {
                x_coordinate -= start_x;
                y_coordinate -= start_y;
            }

            callback(x_coordinate, y_coordinate, *array_2d_at(array_2d, width, x, y));
        }
    }
}


template<typename T>
static void iterate_array_2d(T * array_2d, int width, int height, const function<void(int, int, T &)> & callback)
{
    iterate_array_2d(array_2d, width, 0, 0, width, height, false, callback);
}


static void iterate_rooms(Floor & floor, const function<void(int, int, int &)> & callback)
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
    bool relative_coordinates,
    const function<void(int, int, Tile &)> & callback)
{
    iterate_array_2d(
        floor.room_tiles,
        floor.size * ROOM_TILE_WIDTH,
        room_x * ROOM_TILE_WIDTH,
        room_y * ROOM_TILE_HEIGHT,
        ROOM_TILE_WIDTH,
        ROOM_TILE_HEIGHT,
        relative_coordinates,
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

    int * room = array_2d_at(floor.rooms, size, x, y);

    if (*room == 0 && !contains_key(room_extensions, room))
    {
        ivec2 room_coordinates(x, y);
        room_extensions[room] = room_coordinates;
        floor.possible_rooms[room] = room_coordinates;
    }
}


static void set_room(Floor & floor, Possible_Rooms & room_extensions, int x, int y, int id)
{
    int * room = array_2d_at(floor.rooms, floor.size, x, y);
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


static void generate_room(Floor & floor, int x, int y, int id, int max_size)
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
        // Break if no room extensions could be found (room root is surrounded by other rooms or on the edge of the
        // floor).
        if (room_extensions.size() == 0)
        {
            break;
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
    const int * rooms = floor.rooms;

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
            printf("%d", rooms[(y * size) + x]);
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
    const int rooms_size = size * size;
    const int room_tiles_size = (size * ROOM_TILE_WIDTH) * (size * ROOM_TILE_HEIGHT);
    floor.size = size;
    floor.rooms_size = rooms_size;
    floor.room_tiles_size = room_tiles_size;
    floor.rooms = new int[rooms_size];
    floor.room_tiles = new Tile[room_tiles_size];
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
    int room,
    int neighbor,
    int clockwise_neighbor,
    float rotation,
    bool inverted = false)
{
    tile.rotation = rotation;

    // Corner
    if ((inverted && coordinate == dimension_size - 1) || coordinate == 0)
    {
        // Inner corner
        if (neighbor == room && clockwise_neighbor == room)
        {
            set_tile_type(tile, Tile::Types::WALL_CORNER_INNER);
        }
        // Wall to neighbor
        else if (neighbor == room)
        {
            set_tile_type(tile, Tile::Types::WALL);
            tile.rotation -= 90.0f;
        }
        // Wall to clockwise_neighbor
        else if (clockwise_neighbor == room)
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
    else if (neighbor == room)
    {
        set_tile_type(tile, Tile::Types::FLOOR);
        tile.rotation = 0.0f;
    }
    // Wall
    else if (neighbor > 0)
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


static int get_room_position_coordinate(
    float position_coordinate,
    int tile_dimension_size,
    float texture_dimension_scale)
{
    return (position_coordinate + (texture_dimension_scale / 2.0f)) / (tile_dimension_size * texture_dimension_scale);
}


static void set_render_flags(int room, bool value)
{
    for (bool * render_flag : room_tile_render_flags.at(room))
    {
        *render_flag = value;
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
    floor_size = 6;
    current_floor = create_floor(floor_size);
    Possible_Rooms & possible_rooms = current_floor.possible_rooms;
    iterate_rooms(current_floor, [](int /*x*/, int /*y*/, int & room) -> void { room = 0; });

    iterate_room_tiles(current_floor, [](int /*x*/, int /*y*/, Tile & room_tile) -> void
    {
        room_tile.type = Tile::Types::NONE;
        room_tile.rotation = 0.0f;
    });


    // Generate rooms.
    const int root_room_x = random(0, floor_size);
    const int root_room_y = random(0, floor_size);
    generate_room(current_floor, root_room_x, root_room_y, 1, 1);

    for (int i = 2; i < 9; i++)
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
    iterate_rooms(current_floor, [&](int room_x, int room_y, int & room) -> void
    {
        // Don't generate tiles for empty rooms.
        if (room == 0)
        {
            return;
        }


        iterate_room_tiles(current_floor, room_x, room_y, true, [&](int x, int y, Tile & tile) -> void
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
                const int bottom_neighbor = get_room(room_x, room_y - 1);
                const int left_neighbor = get_room(room_x - 1, room_y);
                const int top_neighbor = get_room(room_x, room_y + 1);
                const int right_neighbor = get_room(room_x + 1, room_y);

                // Bottom wall
                if (y == 0 && x != ROOM_TILE_WIDTH - 1)
                {
                    generate_wall_tile(tile, x, ROOM_TILE_WIDTH, room, bottom_neighbor, left_neighbor, 0.0f);
                }
                // Left wall
                else if (x == 0 && y != 0)
                {
                    generate_wall_tile(tile, y, ROOM_TILE_HEIGHT, room, left_neighbor, top_neighbor, 270.0f, true);
                }
                // Top wall
                else if (y == ROOM_TILE_HEIGHT - 1 && x != 0)
                {
                    generate_wall_tile(tile, x, ROOM_TILE_WIDTH, room, top_neighbor, right_neighbor, 180.0f, true);
                }
                // Right wall
                else if (x == ROOM_TILE_WIDTH - 1 && y != ROOM_TILE_HEIGHT - 1)
                {
                    generate_wall_tile(tile, y, ROOM_TILE_HEIGHT, room, right_neighbor, bottom_neighbor, 90.0f);
                }
            }
        });
    });


    // Create tiles for each room based on each tile's type.
    iterate_rooms(current_floor, [&](int room_x, int room_y, int & room) -> void
    {
        // Don't generate tiles for empty rooms.
        if (room == 0)
        {
            return;
        }


        // Track tile's render flag.
        vector<bool *> & render_flags = room_tile_render_flags[room];


        iterate_room_tiles(current_floor, room_x, room_y, false, [&](int tile_x, int tile_y, const Tile & tile) -> void
        {
            const Tile::Types tile_type = tile.type;

            if (tile_type == Tile::Types::NONE)
            {
                return;
            }

            const float tile_rotation = tile.rotation;
            auto transform = new Transform { vec3(), vec3(1.0f), tile_rotation };
            auto sprite = new Sprite { true, *tile.texture_path, "texture" };
            render_flags.push_back(&sprite->render);

            map<string, Component> tile_components
            {
                { "render_layer" , new string("world")                                   },
                { "transform"    , transform                                             },
                { "dimensions"   , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) } },
                { "sprite"       , sprite                                                },
            };

            vector<string> tile_systems
            {
                "sprite_dimensions_handler",
                "renderer",
            };


            // Give wall tiles a line collider.
            if (tile_type == Tile::Types::RIGHT_DOOR_WALL ||
                tile_type == Tile::Types::LEFT_DOOR_WALL ||
                tile_type == Tile::Types::WALL_CORNER_INNER ||
                tile_type == Tile::Types::WALL ||
                tile_type == Tile::Types::DOOR)
            {
                // Create and configure collider component based on tile type.
                auto collider = new Collider { false, true, false, {} };
                tile_components["collider"] = collider;

                if (tile_type == Tile::Types::DOOR)
                {
                    collider->sends_collision = false;

                    collider->collision_handler = [=](Entity collision_entity) -> void
                    {
                        if (has_component(collision_entity, "layer") &&
                            *(string *)get_component(collision_entity, "layer") == "player")
                        {
                            game_manager_change_rooms(tile_rotation);
                        }
                    };
                }


                if (tile_type == Tile::Types::WALL ||
                    tile_type == Tile::Types::DOOR)
                {
                    // Create and configure line collider component based on tile type.
                    auto line_collider = new Line_Collider
                    {
                        vec3(-0.25f, 0.0f, 0.0f),
                        vec3(0.25f, 0.0f, 0.0f),
                    };

                    if (tile_type == Tile::Types::WALL)
                    {
                        line_collider->begin.y = 0.25f;
                        line_collider->end.y = 0.25f;
                    }
                    else
                    {
                        line_collider->begin.y = -0.1f;
                        line_collider->end.y = -0.1f;
                    }

                    tile_components["line_collider"] = line_collider;
                    tile_systems.push_back("line_collider");
                }
                else if (tile_type == Tile::Types::RIGHT_DOOR_WALL ||
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
            }


            generate_entity(tile_components, tile_systems);
            transform->position = vec3(tile_x, tile_y, 0.0f) * ROOM_TILE_TEXTURE_SCALE;
            transform->position.z = ROOM_Z;
        });
    });


    // Initialize floor.
    for_each(room_tile_render_flags, [](int room, const vector<bool *> & /*render_flags*/) -> void
    {
        set_render_flags(room, false);
    });

    set_render_flags(1, true);

    game_manager_add_room_change_handler(FLOOR_MANAGER_ROOM_CHANGE_HANDLER_ID, [](
        int last_room,
        int current_room) -> void
    {
        set_render_flags(last_room, false);
        set_render_flags(current_room, true);
    });
}


void destroy_floor()
{
    room_datas.clear();
    room_tile_render_flags.clear();
    game_manager_remove_room_change_handler(FLOOR_MANAGER_ROOM_CHANGE_HANDLER_ID);
}


const vec2 & get_spawn_position()
{
    return spawn_position;
}


int get_room(int x, int y)
{
    int floor_size = current_floor.size;

    return x < 0 || x >= floor_size ||
           y < 0 || y >= floor_size
           ? -1
           : *array_2d_at(current_floor.rooms, floor_size, x, y);
}


int get_room(const vec3 & position)
{
    return get_room(
        get_room_position_coordinate(position.x, ROOM_TILE_WIDTH, ROOM_TILE_TEXTURE_SCALE.x),
        get_room_position_coordinate(position.y, ROOM_TILE_HEIGHT, ROOM_TILE_TEXTURE_SCALE.y));
}


const Room_Data & get_room_data(int room)
{
    return room_datas.at(room);
}


void iterate_rooms(const function<void(int, int, int &)> & callback)
{
    iterate_rooms(current_floor, callback);
}


int get_floor_size()
{
    return floor_size;
}


} // namespace Game
