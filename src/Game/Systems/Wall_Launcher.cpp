#include "Game/Systems/Wall_Launcher.hpp"

#include <map>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include <cmath>
#include "Nito/Components.hpp"
#include "Nito/Engine.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"


using std::vector;
using std::map;
using std::string;
using std::function;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::ivec2;
using glm::distance;
using glm::length;

// Nito/Components.hpp
using Nito::Transform;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_entity;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains_key;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::contains;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Wall_Launcher_Entity_State
{
    vec3 * position;
    vec3 * look_direction;
    bool * enemy_enabled;
    const vec3 * target_position;
    int path_index;
    int path_direction;
    float cooldown;
};


struct Wall_Segment
{
    vector<vec2> tile_positions;
    Entity wall_launcher;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const float FIRE_COOLDOWN = 1.0f;
static const vector<string> TARGET_LAYERS { "player" };
static map<Entity, Wall_Launcher_Entity_State> entity_states;
static map<int, vector<Wall_Segment>> room_wall_segments;
static int floor_room_tile_width;
static int floor_room_tile_height;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void traverse_wall(const ivec2 & origin, const function<bool(const ivec2 &)> & callback)
{
    static const vector<ivec2> DIRECTIONS
    {
        ivec2(0, 1),
        ivec2(1, 0),
        ivec2(0, -1),
        ivec2(-1, 0),
    };

    static const vector<Tile_Types> WALL_TILE_TYPES
    {
        Tile_Types::DOOR,
        Tile_Types::WALL,
        Tile_Types::WALL_CORNER,
        Tile_Types::WALL_CORNER_INNER,
        Tile_Types::RIGHT_DOOR_WALL,
        Tile_Types::LEFT_DOOR_WALL,
    };

    ivec2 current_tile = origin;
    ivec2 previous_tile(-1);
    int direction_index = 0;
    int room = get_room_tile(origin.x, origin.y).room;


    // Find clockwise traversal direction from origin tile.
    for (size_t i = 0; i < DIRECTIONS.size(); i++)
    {
        const ivec2 & direction = DIRECTIONS[i];

        if (get_room_tile(origin.x + direction.x, origin.y + direction.y).room != room)
        {
            direction_index = wrap_index(i + 1, DIRECTIONS.size());
            break;
        }
    }


    while (callback(current_tile))
    {
        // Find valid next tile.
        ivec2 next_tile = current_tile + DIRECTIONS[direction_index];
        const Tile * room_tile = &get_room_tile(next_tile.x, next_tile.y);

        while (next_tile.x < 0 || next_tile.x >= floor_room_tile_width ||
               next_tile.y < 0 || next_tile.y >= floor_room_tile_height ||
               room_tile->room != room ||
               next_tile == previous_tile ||
               !contains(WALL_TILE_TYPES, room_tile->type))
        {
            direction_index = wrap_index(direction_index + 1, DIRECTIONS.size());
            next_tile = current_tile + DIRECTIONS[direction_index];
            room_tile = &get_room_tile(next_tile.x, next_tile.y);
        }


        previous_tile = current_tile;
        current_tile = next_tile;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void wall_launcher_init()
{
    add_floor_generated_handler("wall_launcher", [&]() -> void
    {
        const int floor_size = get_floor_size();
        const vec3 & room_tile_texture_scale = get_room_tile_texture_scale();
        const vec2 tile_scale = vec2(room_tile_texture_scale.x, room_tile_texture_scale.y);
        const int room_tile_width = get_room_tile_width();
        const int room_tile_height = get_room_tile_height();
        floor_room_tile_width = room_tile_width * floor_size;
        floor_room_tile_height = room_tile_height * floor_size;
        vector<int> processed_rooms;
        room_wall_segments.clear();

        iterate_rooms([&](int x, int y, int & room) -> void
        {
            if (room < 1 || contains(processed_rooms, room))
            {
                return;
            }

            processed_rooms.push_back(room);
            vector<Wall_Segment> & wall_segments = room_wall_segments[room];


            // Find startring point for generating wall segments for room.
            const ivec2 room_origin(x * room_tile_width, y * room_tile_height);
            ivec2 wall_segments_origin;

            traverse_wall(room_origin, [&](const ivec2 & wall_tile) -> bool
            {
                if (get_room_tile(wall_tile.x, wall_tile.y).type == Tile_Types::DOOR)
                {
                    wall_segments_origin = wall_tile;
                    return false;
                }

                return true;
            });


            // Traverse wall and track contiguous segments of traversable tiles.
            ivec2 wall_segment_start = wall_segments_origin;

            do
            {
                Wall_Segment wall_segment;
                wall_segment.wall_launcher = -1;
                vector<vec2> & tile_positions = wall_segment.tile_positions;

                traverse_wall(wall_segment_start, [&](const ivec2 & wall_tile) -> bool
                {
                    // Ignore wall segment start unless it has been reached again after looping the entire room's wall.
                    if (wall_tile == wall_segment_start && tile_positions.size() == 0)
                    {
                        return true;
                    }


                    // Break wall segment at first traversed door.
                    if (get_room_tile(wall_tile.x, wall_tile.y).type == Tile_Types::DOOR)
                    {
                        wall_segment_start = wall_tile;
                        return false;
                    }


                    tile_positions.push_back(vec2(wall_tile.x, wall_tile.y) * tile_scale);
                    return true;
                });

                wall_segments.push_back(wall_segment);
            }
            while (wall_segment_start != wall_segments_origin);
        });
    });
}


void wall_launcher_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        &((Transform *)get_component(entity, "transform"))->position,
        &((Orientation_Handler *)get_component(entity, "orientation_handler"))->look_direction,
        (bool *)get_component(entity, "enemy_enabled"),
        &((Transform *)get_component(get_entity("player"), "transform"))->position,
        1,
        1,
        0.0f,
    };
}


void wall_launcher_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void wall_launcher_update()
{
    static const float WALL_LAUNCHER_RANGE = 3.0f;
    static const string PROJECTILE_NAME("projectile_red_orb");

    const float time_scale = get_time_scale();
    const float delta_time = get_delta_time() * time_scale;


    // Handle movement.
    for_each(room_wall_segments, [&](int /*room*/, const vector<Wall_Segment> & wall_segments) -> void
    {
        for (const Wall_Segment & wall_segment : wall_segments)
        {
            if (!contains_key(entity_states, wall_segment.wall_launcher))
            {
                continue;
            }

            Wall_Launcher_Entity_State & entity_state = entity_states[wall_segment.wall_launcher];
            int & path_index = entity_state.path_index;
            int & path_direction = entity_state.path_direction;
            vec3 * position = entity_state.position;
            vec3 * look_direction = entity_state.look_direction;
            const vector<vec2> & tile_positions = wall_segment.tile_positions;
            const vec2 & tile_position = tile_positions[path_index];
            const vec2 movement = move_entity(*position, *look_direction, tile_position);


            // If game is not paused, invert look direction when traveling backwards along wall segment.
            if (time_scale > 0)
            {
                *look_direction *= path_direction;
            }


            if (distance(tile_position, (vec2)*position) < length(movement))
            {
                if (path_direction == 1 && path_index == tile_positions.size() - 1)
                {
                    path_direction = -1;
                }
                else if (path_direction == -1 && path_index == 0)
                {
                    path_direction = 1;
                }

                path_index += path_direction;
            }
        }
    });


    // Handle firing.
    for_each(entity_states, [&](Entity /*entity*/, Wall_Launcher_Entity_State & entity_state) -> void
    {
        if (!*entity_state.enemy_enabled)
        {
            return;
        }

        const vec3 & position = *entity_state.position;
        const vec3 & target_position = *entity_state.target_position;
        const vec3 & look_direction = *entity_state.look_direction;
        const int & path_direction = entity_state.path_direction;
        float & cooldown = entity_state.cooldown;

        if (cooldown > 0.0f)
        {
            cooldown -= delta_time;
        }
        else if (distance((vec2)target_position, (vec2)position) < WALL_LAUNCHER_RANGE)
        {
            static const vector<vec3> FIRE_POSITION_OFFSETS
            {
                vec3( 0     , 0.35f , 0), // Up
                vec3( 0     ,-0.35f , 0), // Down
                vec3(-0.35f , 0     , 0), // Left
                vec3( 0.35f , 0     , 0), // Right
            };

            int fire_position_offset_index =
                fabsf(look_direction.x) > fabsf(look_direction.y)
                ? (look_direction.x < 0 ? 0 : 1)
                : (look_direction.y < 0 ? 2 : 3);

            const vec3 fire_origin = position + FIRE_POSITION_OFFSETS[fire_position_offset_index];

            fire_projectile(
                PROJECTILE_NAME,
                fire_origin,
                target_position - fire_origin,
                1.0f,
                TARGET_LAYERS);

            cooldown = FIRE_COOLDOWN;
        }
    });
}


vector<Entity> wall_launcher_generate(int room, int /*room_origin_x*/, int /*room_origin_y*/)
{
    vector<Entity> wall_launchers;
    vector<Wall_Segment> & wall_segments = room_wall_segments[room];

    for (Wall_Segment & wall_segment : wall_segments)
    {
        Entity & wall_launcher = wall_segment.wall_launcher;

        if (wall_launcher == -1)
        {
            wall_launcher = load_blueprint("wall_launcher");
            wall_launchers.push_back(wall_launcher);
            const vec2 & segment_start_tile_position = wall_segment.tile_positions[0];
            vec3 & wall_launcher_position = ((Transform *)get_component(wall_launcher, "transform"))->position;
            wall_launcher_position.x = segment_start_tile_position.x;
            wall_launcher_position.y = segment_start_tile_position.y;
        }
    }

    return wall_launchers;
}


} // namespace Game
