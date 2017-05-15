#include "Game/Systems/Enemy_Projectile_Launcher.hpp"

#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <cmath>
#include "Nito/Components.hpp"
#include "Nito/Engine.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"


using std::vector;
using std::string;
using std::map;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::distance;

// Nito/Components.hpp
using Nito::Transform;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_entity;
using Nito::get_component;

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
struct Enemy_Projectile_Launcher_State
{
    Enemy_Projectile_Launcher * enemy_projectile_launcher;
    vec3 * position;
    Orientation * orientation;
    bool * enemy_enabled;
    const vec3 * target_position;
    float cooldown;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const vector<string> TARGET_LAYERS { "player" };
static map<Entity, Enemy_Projectile_Launcher_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void enemy_projectile_launcher_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        (Enemy_Projectile_Launcher *)get_component(entity, "enemy_projectile_launcher"),
        &((Transform *)get_component(entity, "transform"))->position,
        &((Orientation_Handler *)get_component(entity, "orientation_handler"))->orientation,
        (bool *)get_component(entity, "enemy_enabled"),
        &((Transform *)get_component(get_entity("player"), "transform"))->position,
        0.0f,
    };
}


void enemy_projectile_launcher_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void enemy_projectile_launcher_update()
{
    const float delta_time = get_delta_time() * get_time_scale();

    for_each(entity_states, [&](Entity /*entity*/, Enemy_Projectile_Launcher_State & entity_state) -> void
    {
        const Enemy_Projectile_Launcher * enemy_projectile_launcher = entity_state.enemy_projectile_launcher;


        // Don't fire projectile if disabled.
        if (!enemy_projectile_launcher->enabled || !*entity_state.enemy_enabled)
        {
            return;
        }


        const vec3 & position = *entity_state.position;
        const vec3 & target_position = *entity_state.target_position;
        float & cooldown = entity_state.cooldown;

        if (cooldown > 0.0f)
        {
            cooldown -= delta_time;
        }
        else if (distance((vec2)target_position, (vec2)position) < enemy_projectile_launcher->range)
        {
            const vec3 fire_origin =
                position + enemy_projectile_launcher->orientation_offsets.at(*entity_state.orientation);

            fire_projectile(
                enemy_projectile_launcher->projectile_name,
                fire_origin,
                target_position - fire_origin,
                1.0f,
                TARGET_LAYERS);

            cooldown = enemy_projectile_launcher->cooldown_time;
        }
    });
}


} // namespace Game
