#include "Game/APIs/Floor_Manager.hpp"

#include <vector>
#include <map>
#include <stdexcept>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/Systems/Game_Manager.hpp"
#include "Game/Systems/Room_Exit_Handler.hpp"


using std::string;
using std::vector;
using std::map;
using std::function;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::ivec2;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::has_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Nito/Components.hpp
using Nito::Transform;

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
using Possible_Rooms = map<int *, ivec2>;


struct Floor
{
    int size;
    int room_tiles_width;
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
static const int ROOM_TILE_TEXTURE_SIZE = 32;
static const float ROOM_TILE_TEXTURE_ORIGINS = 0.5f;
static const string ROOM_CHANGE_HANDLER_ID("floor_manager");
static const int SPAWN_ROOM_ID = 1;
static vec3 room_tile_unit_size;
static Floor current_floor;
static vec2 spawn_position;
static map<int, Room_Data> room_datas;
static map<int, int> room_enemy_counts;
static map<int, vector<Entity>> room_exits;
static int max_room_id;
static map<string, function<void()>> floor_generated_handlers;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


static float get_room_center_coordinate(int room_coordinate, int tile_dimension_size, float tile_unit_size)
{
    return ((room_coordinate * tile_dimension_size) + (tile_dimension_size / 2)) * tile_unit_size;
}


static vec2 get_room_center(int room_x, int room_y)
{
    return vec2(
        get_room_center_coordinate(room_x, ROOM_TILE_WIDTH, room_tile_unit_size.x),
        get_room_center_coordinate(room_y, ROOM_TILE_HEIGHT, room_tile_unit_size.y));
}


static void set_room_locked(int room_id, bool locked)
{
    for (const Entity room_exit : room_exits.at(room_id))
    {
        room_exit_handler_set_locked(room_exit, locked);
    }
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
            int room = rooms[(y * size) + x];
            char room_display = room > 9 ? ('A' + (room - 10)) : ('0' + room);
            printf("%c", room_display);
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
            tile.type = Tile_Types::WALL_CORNER_INNER;
        }
        // Wall to neighbor
        else if (neighbor == room)
        {
            tile.type = Tile_Types::WALL;
            tile.rotation -= 90.0f;
        }
        // Wall to clockwise_neighbor
        else if (clockwise_neighbor == room)
        {
            tile.type = Tile_Types::WALL;
        }
        // Outer corner
        else
        {
            tile.type = Tile_Types::WALL_CORNER;
        }
    }
    // Floor between rooms
    else if (neighbor == room)
    {
        tile.type = Tile_Types::FLOOR;
        tile.rotation = 0.0f;
    }
    // Wall
    else if (neighbor > 0)
    {
        // Door
        if (coordinate == (dimension_size - 1) / 2)
        {
            tile.type = Tile_Types::DOOR;
        }
        // Door-adjacent wall
        else if (coordinate == (dimension_size - 2) / 2)
        {
            tile.type = inverted ? Tile_Types::LEFT_DOOR_WALL : Tile_Types::RIGHT_DOOR_WALL;
        }
        // Door-adjacent wall
        else if (coordinate == ((dimension_size - 2) / 2) + 2)
        {
            tile.type = inverted ? Tile_Types::RIGHT_DOOR_WALL : Tile_Types::LEFT_DOOR_WALL;
        }
        // Normal wall
        else
        {
            tile.type = Tile_Types::WALL;
        }
    }
    // Normal wall
    else
    {
        tile.type = Tile_Types::WALL;
    }
}


static int get_room_position_coordinate(float position, int tile_dimension_size, float tile_unit_size)
{
    return (position + (tile_unit_size * ROOM_TILE_TEXTURE_ORIGINS)) /
           (tile_dimension_size * tile_unit_size);
}


static int get_room_tile_position_coordinate(float position, float tile_unit_size)
{
    return (position + (tile_unit_size * ROOM_TILE_TEXTURE_ORIGINS)) / tile_unit_size;
}


static float get_room_tile_coordinate_position(int coordinate, float tile_unit_size)
{
    return coordinate * tile_unit_size;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void floor_manager_api_init()
{
    room_tile_unit_size = vec3(1) * (float)(ROOM_TILE_TEXTURE_SIZE / get_pixels_per_unit());
    room_tile_unit_size.z = 1;
}


void generate_floor(int floor_size)
{
    // Create floor.
    const int room_tiles_width = floor_size * ROOM_TILE_WIDTH;
    Possible_Rooms & possible_rooms = current_floor.possible_rooms;
    current_floor.size = floor_size;
    current_floor.room_tiles_width = room_tiles_width;
    current_floor.rooms = new int[floor_size * floor_size];
    current_floor.room_tiles = new Tile[room_tiles_width * (floor_size * ROOM_TILE_HEIGHT)];
    iterate_rooms(current_floor, [](int /*x*/, int /*y*/, int & room) -> void { room = 0; });

    iterate_room_tiles(current_floor, [](int /*x*/, int /*y*/, Tile & room_tile) -> void
    {
        room_tile.type = Tile_Types::NONE;
        room_tile.rotation = 0.0f;
    });


    // Generate rooms.
    static const int MAX_ROOM_SIZE = 2;

    // Calculate the max number of fully-sized rooms that can be generated. The "- 2" & "+ 2" account for the spawn and
    // boss rooms being size 1, therefore not being required to be multiplied by MAX_ROOM_SIZE.
    max_room_id = (((floor_size * floor_size) - 2) / MAX_ROOM_SIZE) + 2;

    const int root_room_x = 0;//random(0, floor_size);
    const int root_room_y = 0;//random(0, floor_size);
    generate_room(current_floor, root_room_x, root_room_y, SPAWN_ROOM_ID, 1);

    for (int room_id = SPAWN_ROOM_ID + 1; room_id <= max_room_id; room_id++)
    {
        // No possible rooms available.
        if (possible_rooms.size() == 0)
        {
            throw runtime_error("ERROR: exhausted possible rooms for generation before reaching max room ID!");
        }

        ivec2 room_coordinates = at_index(possible_rooms, random(0, possible_rooms.size())).second;

        generate_room(
            current_floor,
            room_coordinates.x,
            room_coordinates.y,
            room_id,

            // If boss room size changes, max_room_id calculation needs to be updated.
            room_id == max_room_id ? 1 : MAX_ROOM_SIZE);
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
            tile.room = room;


            // Floor
            if (x > 0 && x < ROOM_TILE_WIDTH - 1 &&
                y > 0 && y < ROOM_TILE_HEIGHT - 1)
            {
                if (room == max_room_id &&
                    x == (ROOM_TILE_WIDTH - 1) / 2 &&
                    y == (ROOM_TILE_HEIGHT - 1) / 2)
                {
                    tile.type = Tile_Types::NEXT_FLOOR;
                }
                else
                {
                    tile.type = Tile_Types::FLOOR;
                }

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
    iterate_rooms(current_floor, [&](int room_x, int room_y, int & room_id) -> void
    {
        // Don't generate tiles for empty rooms.
        if (room_id == 0)
        {
            return;
        }


        iterate_room_tiles(current_floor, room_x, room_y, false, [&](
            int tile_x,
            int tile_y,
            const Tile & tile_data) -> void
        {
            static const map<Tile_Types, const string> TILE_TYPE_BLUEPRINTS
            {
                { Tile_Types::WALL              , "wall_tile"              },
                { Tile_Types::WALL_CORNER       , "wall_corner_tile"       },
                { Tile_Types::WALL_CORNER_INNER , "wall_corner_inner_tile" },
                { Tile_Types::DOOR              , "door_tile"              },
                { Tile_Types::FLOOR             , "floor_tile"             },
                { Tile_Types::LEFT_DOOR_WALL    , "left_door_wall_tile"    },
                { Tile_Types::RIGHT_DOOR_WALL   , "right_door_wall_tile"   },
                { Tile_Types::NEXT_FLOOR        , "next_floor_tile"        },
            };

            const Tile_Types tile_type = tile_data.type;

            if (tile_type == Tile_Types::NONE)
            {
                return;
            }


            // Create tile entity, set its position and rotation, and track it.
            const float tile_rotation = tile_data.rotation;
            const Entity tile = load_blueprint(TILE_TYPE_BLUEPRINTS.at(tile_type));
            auto transform = (Transform *)get_component(tile, "transform");
            vec3 & position = transform->position;
            transform->rotation = tile_rotation;
            position = vec3(tile_x, tile_y, 0.0f) * room_tile_unit_size;
            position.z = ROOM_Z;
            game_manager_track_render_flag(room_id, tile);

            if (has_component(tile, "collider"))
            {
                game_manager_track_collider_enabled_flag(room_id, tile);
            }


            // Set collision handlers for tiles that need them.
            if (tile_type == Tile_Types::DOOR ||
                tile_type == Tile_Types::NEXT_FLOOR)
            {
                auto room_exit = (Room_Exit *)get_component(tile, "room_exit");
                auto collider = ((Collider *)get_component(tile, "collider"));
                room_exits[room_id].push_back(tile);

                if (tile_type == Tile_Types::DOOR)
                {
                    collider->collision_handler = [=](Entity collision_entity) -> void
                    {
                        if (!room_exit->locked && in_layer(collision_entity, "player"))
                        {
                            game_manager_change_rooms(tile_rotation);
                        }
                    };
                }
                else if (tile_type == Tile_Types::NEXT_FLOOR)
                {
                    collider->collision_handler = [=](Entity collision_entity) -> void
                    {
                        if (!room_exit->locked && in_layer(collision_entity, "player"))
                        {
                            game_manager_complete_floor();
                        }
                    };
                }
            }
        });
    });


    // Lock current room if its enemy count is > 0.
    game_manager_add_room_change_handler(ROOM_CHANGE_HANDLER_ID, [](int /*last_room*/, int current_room) -> void
    {
        if (room_enemy_counts[current_room] > 0)
        {
            set_room_locked(current_room, true);
        }
    });


    // Trigger floor generated handlers.
    for_each(floor_generated_handlers, [](const string & /*id*/, const function<void()> & handler) -> void
    {
        handler();
    });
}


void destroy_floor()
{
    // Cleanup current floor data.
    current_floor.size = 0;
    current_floor.room_tiles_width = 0;
    delete[] current_floor.rooms;
    delete[] current_floor.room_tiles;
    current_floor.possible_rooms.clear();


    // Cleanup room data.
    room_datas.clear();
    room_enemy_counts.clear();
    room_exits.clear();
    game_manager_remove_room_change_handler(ROOM_CHANGE_HANDLER_ID);
}


const vec2 & get_spawn_position()
{
    return spawn_position;
}


int get_room(int x, int y)
{
    const int floor_size = current_floor.size;

    return x < 0 || x >= floor_size ||
           y < 0 || y >= floor_size
           ? -1
           : *array_2d_at(current_floor.rooms, floor_size, x, y);
}


int get_room(const vec3 & position)
{
    return get_room(
        get_room_position_coordinate(position.x, ROOM_TILE_WIDTH, room_tile_unit_size.x),
        get_room_position_coordinate(position.y, ROOM_TILE_HEIGHT, room_tile_unit_size.y));
}


const Room_Data & get_room_data(int room)
{
    return room_datas.at(room);
}


const Tile & get_room_tile(int x, int y)
{
    return *array_2d_at(current_floor.room_tiles, current_floor.room_tiles_width, x, y);
}


void iterate_rooms(const function<void(int, int, int &)> & callback)
{
    iterate_rooms(current_floor, callback);
}


int get_floor_size()
{
    return current_floor.size;
}


int get_room_tile_width()
{
    return ROOM_TILE_WIDTH;
}


int get_room_tile_height()
{
    return ROOM_TILE_HEIGHT;
}


const glm::vec3 & get_room_tile_texture_scale()
{
    return room_tile_unit_size;
}


int get_max_room_id()
{
    return max_room_id;
}


int get_spawn_room_id()
{
    return SPAWN_ROOM_ID;
}


void add_enemy(int room_id)
{
    room_enemy_counts[room_id]++;
}


void remove_enemy(int room_id)
{
    if (--room_enemy_counts[room_id] == 0)
    {
        set_room_locked(room_id, false);
    }
}


vec2 get_room_tile_coordinates(const vec2 & position)
{
    return vec2(
        get_room_tile_position_coordinate(position.x, room_tile_unit_size.x),
        get_room_tile_position_coordinate(position.y, room_tile_unit_size.y));
}


vec2 get_room_tile_position(const vec2 & coordinates)
{
    return vec2(
        get_room_tile_coordinate_position(coordinates.x, room_tile_unit_size.x),
        get_room_tile_coordinate_position(coordinates.y, room_tile_unit_size.y));
}


void add_floor_generated_handler(const string & id, const function<void()> & handler)
{
    floor_generated_handlers[id] = handler;
}


void remove_floor_generated_handler(const string & id)
{
    remove(floor_generated_handlers, id);
}


} // namespace Game
