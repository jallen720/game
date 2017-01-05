#include "Game/Systems/Projectile.hpp"

#include <map>
#include <string>
#include "Nito/Components.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"
#include "Game/Systems/Health.hpp"


using std::map;
using std::string;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::has_component;
using Nito::flag_entity_for_deletion;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Window.hpp
using Nito::get_time;
using Nito::get_delta_time;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

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
struct Projectile_State
{
    Transform * transform;
    const Projectile * projectile;
    float creation_time;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Projectile_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void projectile_subscribe(const Entity entity)
{
    auto projectile = (Projectile *)get_component(entity, "projectile");

    entity_states[entity] =
    {
        (Transform *)get_component(entity, "transform"),
        projectile,
        get_time(),
    };


    // Setup collision handler to damage entity if its layer is in projectile's target layers.
    auto collider = (Collider *)get_component(entity, "collider");

    collider->collision_handler = [=](const Entity collision_entity) -> void
    {
        if (has_component(collision_entity, "layer"))
        {
            const auto collision_layer = (string *)get_component(collision_entity, "layer");

            if (contains(projectile->target_layers, *collision_layer))
            {
                damage_entity(collision_entity, 10.0f);
                flag_entity_for_deletion(entity);
            }
        }
    };
}


void projectile_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void projectile_update()
{
    const float time = get_time();
    const float delta_time = get_delta_time();

    for_each(entity_states, [=](const Entity entity, const Projectile_State & entity_state) -> void
    {
        const Projectile * projectile = entity_state.projectile;

        // If projectile's duration has expired, flag it for deletion.
        if (time - entity_state.creation_time > projectile->duration)
        {
            flag_entity_for_deletion(entity);
            return;
        }

        entity_state.transform->position += projectile->speed * projectile->direction * delta_time;
    });
}


} // namespace Game
