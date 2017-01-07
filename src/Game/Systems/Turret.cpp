#include "Game/Systems/Turret.hpp"

#include <map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Nito/Engine.hpp"
#include "Nito/Components.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"
#include "Game/Utilities.hpp"


using std::map;
using std::string;
using std::vector;

// glm/glm.hpp
using glm::vec3;

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
    Health * health;
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
    static vec3 * target_position;

    if (target_position == nullptr)
    {
        target_position = &((Transform *)get_component(get_entity("Player"), "transform"))->position;
    }

    entity_states[entity] =
    {
        (Transform *)get_component(entity, "transform"),
        (Orientation_Handler *)get_component(entity, "orientation_handler"),
        (Health *)get_component(entity, "health"),
        target_position,
        0.0f,
    };
}


void turret_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void turret_update()
{
    const float delta_time = get_delta_time() * get_time_scale();

    for_each(entity_states, [&](Entity /*entity*/, Turret_State & entity_state) -> void
    {
        const vec3 & position = entity_state.transform->position;
        float & cooldown = entity_state.cooldown;
        vec3 & look_direction = entity_state.orientation_handler->look_direction;
        look_direction = *entity_state.target_position - position;

        if (cooldown <= 0.0f)
        {
            fire_projectile(position, look_direction, 1.0f, TARGET_LAYERS);
            cooldown = FIRE_COOLDOWN;
        }
        else
        {
            cooldown -= delta_time;
        }
    });
}


} // namespace Game
