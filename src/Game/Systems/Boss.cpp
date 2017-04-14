#include "Game/Systems/Boss.hpp"

#include <vector>
#include <deque>
#include <stdexcept>
#include <algorithm>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Game_Manager.hpp"


using std::vector;
using std::deque;
using std::runtime_error;
using std::fill;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::distance;
using glm::length;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::flag_entity_for_deletion;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const int SEGMENT_COUNT = 7;
static vec3 * position;
static vec3 * look_direction;
static vec2 destination(-1);
static int direction_index = 0;
static int boss_room;
static vector<Entity> segments;
static vector<vec2 *> segment_destinations;
static deque<vec2> destinations;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int wrap_index(int index, int container_size)
{
    return index >= container_size ? index - container_size :
           index < 0 ? index + container_size :
           index;
}


static void update_segments()
{
    for (int i = 0; i < SEGMENT_COUNT; i++)
    {
        *segment_destinations[i] = destinations[i];
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void boss_subscribe(Entity entity)
{
    position = &((Transform *)get_component(entity, "transform"))->position;
    look_direction = &((Orientation_Handler *)get_component(entity, "orientation_handler"))->look_direction;
    destination = vec2(-1);
    direction_index = 0;
    boss_room = get_max_room_id();


    // Remove boss entity flags from game manager when boss dies.
    ((Health *)get_component(entity, "health"))->death_handlers["boss"] = [&]() -> void
    {
        for (const Entity segment : segments)
        {
            game_manager_untrack_render_flag(boss_room, segment);
            game_manager_untrack_collider_enabled_flag(boss_room, segment);
        }
    };
}


void boss_unsubscribe(Entity /*entity*/)
{
    position = nullptr;
    look_direction = nullptr;
    for_each(segments, flag_entity_for_deletion);
    segments.clear();
    segment_destinations.clear();
    destinations.clear();
}


void boss_update()
{
    if (position == nullptr)
    {
        return;
    }

    static const vector<vec2> DIRECTIONS
    {
        vec2( 1, 0),
        vec2( 0, 1),
        vec2(-1, 0),
        vec2( 0,-1),
    };


    // If destination is unset, search neighboring tiles for a new destination.
    if (destination.x == -1)
    {
        const vec2 current_tile_coordinates = get_room_tile_coordinates(*position);


        // Give 1:2 chance to randomly change direction.
        if (random(0, 3) == 0)
        {
            direction_index = wrap_index(direction_index + (random(0, 2) == 0 ? 1 : -1), DIRECTIONS.size());
        }


        for (int count = 0; count < DIRECTIONS.size(); count++)
        {
            const vec2 & direction = DIRECTIONS[direction_index];

            const vec2 destination_tile_coordinates(
                current_tile_coordinates.x + direction.x,
                current_tile_coordinates.y + direction.y);

            const Tile_Types destination_tile_type = get_room_tile(
                destination_tile_coordinates.x,
                destination_tile_coordinates.y).type;

            if (destination_tile_type != Tile_Types::FLOOR)
            {
                direction_index = wrap_index(direction_index + 1, DIRECTIONS.size());
                continue;
            }

            destination = get_room_tile_position(destination_tile_coordinates);
        }


        // If destination is still unset, no neighboring tile is navigable meaning the boss is out-of-bounds or
        // surrounded by non-navigable tiles.
        if (destination.x == -1)
        {
            throw runtime_error("ERROR: failed to find a navigable neighboring tile from boss' current tile!");
        }
    }


    const vec2 movement = move_entity(*position, *look_direction, destination);


    // Boss is close enough to destination or has passed it, so unset destination to be reset next frame.
    if (distance(destination, (vec2)*position) < length(movement))
    {
        // Update destinations with completed destination.
        destinations.push_front(destination);
        destinations.pop_back();


        update_segments();
        destination = vec2(-1);
    }
}


void boss_init()
{
    // Initialize all destinations to boss' position.
    destinations.resize(SEGMENT_COUNT);
    fill(destinations.begin(), destinations.end(), *position);


    // Create boss segments.
    for (int i = 0; i < SEGMENT_COUNT; i++)
    {
        Entity segment = load_blueprint("boss_segment");
        ((Transform *)get_component(segment, "transform"))->position = *position;
        segment_destinations.push_back((vec2 *)get_component(segment, "destination"));
        segments.push_back(segment);
        game_manager_track_render_flag(boss_room, segment);
        game_manager_track_collider_enabled_flag(boss_room, segment);
    }


    update_segments();
}


} // namespace Game
