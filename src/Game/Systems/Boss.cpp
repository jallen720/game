#include "Game/Systems/Boss.hpp"

#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Nito/Components.hpp"
#include "Nito/Engine.hpp"
#include "Nito/APIs/Window.hpp"

#include "Game/Utilities.hpp"
#include "Game/APIs/Floor_Manager.hpp"


using std::vector;
using std::runtime_error;

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

// Nito/Components.hpp
using Nito::Transform;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/Window.hpp
using Nito::get_delta_time;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vec3 * position;
static int direction_index = 0;
static vec2 destination(-1);


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
    direction_index = 0;
    destination = vec2(-1);
}


void boss_unsubscribe(Entity /*entity*/)
{
    position = nullptr;
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


    const vec2 position_2d = (vec2)*position;
    const vec2 movement_direction = destination - position_2d;
    const vec2 movement = normalize(movement_direction) * get_delta_time() * get_time_scale();
    position->x += movement.x;
    position->y += movement.y;


    // Boss is close enough to destination or has passed it, so unset destination to be reset next frame.
    if (distance(destination, position_2d) < length(movement))
    {
        destination = vec2(-1);
    }
}


} // namespace Game
