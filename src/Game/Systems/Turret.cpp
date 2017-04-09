#include "Game/Systems/Turret.hpp"

#include <map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Nito/Engine.hpp"
#include "Nito/Components.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"
#include "Game/Utilities.hpp"


using std::map;
using std::string;
using std::vector;

// glm/glm.hpp
using glm::vec2;
using glm::vec3;
using glm::distance;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;

// Nito/APIs/ECS.hpp
using Nito::flag_entity_for_deletion;

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
struct Turret_State
{
    Transform * transform;
    Orientation_Handler * orientation_handler;
    const vec3 * target_position;
    float cooldown;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const float FIRE_COOLDOWN = 1.0f;
static const vector<string> TARGET_LAYERS { "player" };
static map<Entity, Turret_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void turret_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        (Transform *)get_component(entity, "transform"),
        (Orientation_Handler *)get_component(entity, "orientation_handler"),
        &((Transform *)get_component(get_entity("player"), "transform"))->position,
        0.0f,
    };
}


void turret_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void turret_update()
{
    static const float TURRET_RANGE = 3.0f;

    const float delta_time = get_delta_time() * get_time_scale();

    for_each(entity_states, [&](Entity /*entity*/, Turret_State & entity_state) -> void
    {
        const vec3 & position = entity_state.transform->position;
        const vec3 & target_position = *entity_state.target_position;
        float & cooldown = entity_state.cooldown;
        vec3 & look_direction = entity_state.orientation_handler->look_direction;
        look_direction = target_position - position;

        if (cooldown > 0.0f)
        {
            cooldown -= delta_time;
        }
        else if (distance((vec2)target_position, (vec2)position) < TURRET_RANGE)
        {
            fire_projectile(position, look_direction, 1.0f, TARGET_LAYERS);
            cooldown = FIRE_COOLDOWN;
        }
    });
}


} // namespace Game
