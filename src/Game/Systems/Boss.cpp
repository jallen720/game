#include "Game/Systems/Boss.hpp"

#include <vector>
#include <stdexcept>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Nito/Components.hpp"
#include "Nito/Engine.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"


using std::vector;
using std::runtime_error;
using std::isnan;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::distance;
using glm::length;

// glm/gtc/matrix_transform.hpp
using glm::normalize;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::flag_entity_for_deletion;

// Nito/Components.hpp
using Nito::Transform;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;

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
static vec3 * position;
static vec3 * look_direction;
static int direction_index = 0;
static vec2 destination(-1);
static vector<Entity> tail_segments;


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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void boss_subscribe(Entity entity)
{
    position = &((Transform *)get_component(entity, "transform"))->position;
    look_direction = &((Orientation_Handler *)get_component(entity, "orientation_handler"))->look_direction;
    direction_index = 0;
    destination = vec2(-1);
}


void boss_unsubscribe(Entity /*entity*/)
{
    position = nullptr;
    look_direction = nullptr;
    for_each(tail_segments, flag_entity_for_deletion);
    tail_segments.clear();
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

    const float time_scale = get_time_scale();


    // Don't update boss position/orientation if game is paused.
    if (time_scale == 0)
    {
        return;
    }


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


    const vec2 position_2d = (vec2)*position;


    // Calculate movement_direction.
    vec2 movement_direction = normalize(destination - position_2d);

    if (isnan(movement_direction.x))
    {
        movement_direction.x = 0;
    }

    if (isnan(movement_direction.y))
    {
        movement_direction.y = 0;
    }


    const vec2 movement = movement_direction * get_delta_time() * time_scale;
    position->x += movement.x;
    position->y += movement.y;
    look_direction->x = movement.x;
    look_direction->y = movement.y;


    // Boss is close enough to destination or has passed it, so unset destination to be reset next frame.
    if (distance(destination, position_2d) < length(movement))
    {
        destination = vec2(-1);
    }
}


void boss_init()
{
    // Create boss "tail".
    for (int i = 0; i < 1; i++)
    {
        Entity tail_segment = load_blueprint("boss_tail_segment");
        ((Transform *)get_component(tail_segment, "transform"))->position = *position;
        tail_segments.push_back(tail_segment);
    }
}


} // namespace Game
