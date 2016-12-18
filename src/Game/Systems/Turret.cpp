#include "Game/Systems/Turret.hpp"

#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"
#include "Game/Utilities.hpp"


using std::map;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Window.hpp
using Nito::get_time;

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
struct Entity_State
{
    Transform * transform;
    Orientation_Handler * orientation_handler;
    const vec3 * target_position;
    float last_fire_time;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const float FIRE_COOLDOWN = 1.0f;
static map<Entity, Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void turret_subscribe(const Entity entity)
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
        target_position,
        -FIRE_COOLDOWN,
    };
}


void turret_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void turret_update()
{
    const float time = get_time();

    for_each(entity_states, [=](const Entity /*entity*/, Entity_State & entity_state) -> void
    {
        const vec3 & position = entity_state.transform->position;
        float & last_fire_time = entity_state.last_fire_time;
        vec3 & look_direction = entity_state.orientation_handler->look_direction;
        look_direction = *entity_state.target_position - position;

        if (time - last_fire_time > FIRE_COOLDOWN)
        {
            fire_projectile(position, look_direction, 1.0f);
            last_fire_time = time;
        }
    });
}


} // namespace Game
