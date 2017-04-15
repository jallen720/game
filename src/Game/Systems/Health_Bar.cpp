#include "Game/Systems/Health_Bar.hpp"

#include <map>
#include <string>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;
using std::string;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;
using Nito::flag_entity_for_deletion;

// Nito/Components.hpp
using Nito::Dimensions;

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
struct Health_Bar_State
{
    float max_health_bar_width;
    float * health_bar_width;
    Health * target_health;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Health_Bar_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void health_bar_subscribe(Entity entity)
{
    float * health_bar_width = &((Dimensions *)get_component(entity, "dimensions"))->width;
    auto target_health = (Health *)get_component(get_entity(*(string *)get_component(entity, "target_id")), "health");

    target_health->death_handlers["health_bar"] = [=]() -> void
    {
        flag_entity_for_deletion(entity);
    };

    entity_states[entity] =
    {
        *health_bar_width,
        health_bar_width,
        target_health,
    };
}


void health_bar_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void health_bar_update()
{
    for_each(entity_states, [](Entity /*entity*/, const Health_Bar_State & entity_state) -> void
    {
        Health * target_health = entity_state.target_health;

        *entity_state.health_bar_width =
            entity_state.max_health_bar_width * (target_health->current / target_health->max);
    });
}


} // namespace Game
