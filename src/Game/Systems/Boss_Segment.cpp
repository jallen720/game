#include "Game/Systems/Boss_Segment.hpp"

#include <map>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Nito/Components.hpp"
#include "Nito/Engine.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;
using std::isnan;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// glm/gtc/matrix_transform.hpp
using glm::normalize;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Boss_Segment_State
{
    vec3 * position;
    vec3 * look_direction;
    vec2 * destination;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct map<Entity, Boss_Segment_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void boss_segment_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        &((Transform *)get_component(entity, "transform"))->position,
        &((Orientation_Handler *)get_component(entity, "orientation_handler"))->look_direction,
        (vec2 *)get_component(entity, "destination"),
    };
}


void boss_segment_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void boss_segment_update()
{
    const float time_scale = get_time_scale();

    for_each(entity_states, [=](Entity /*entity*/, Boss_Segment_State & entity_state) -> void
    {
        vec3 * position = entity_state.position;
        vec3 * look_direction = entity_state.look_direction;
        const vec2 position_2d = (vec2)*position;


        // Don't update boss position/orientation if game is paused.
        if (time_scale == 0)
        {
            return;
        }


        // Calculate movement_direction.
        vec2 movement_direction = normalize(*entity_state.destination - position_2d);

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
    });
}


} // namespace Game
