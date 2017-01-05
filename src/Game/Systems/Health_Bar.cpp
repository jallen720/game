#include "Game/Systems/Health_Bar.hpp"

#include <map>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

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
    Health * health;
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
void health_bar_subscribe(const Entity entity)
{
    float * health_bar_width = &((Dimensions *)get_component(get_entity("health_bar_foreground"), "dimensions"))->width;

    entity_states[entity] =
    {
        *health_bar_width,
        health_bar_width,
        (Health *)get_component(entity, "health"),
    };
}


void health_bar_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void health_bar_update()
{
    for_each(entity_states, [](const Entity /*entity*/, const Health_Bar_State & entity_state) -> void
    {
        Health * health = entity_state.health;
        *entity_state.health_bar_width = entity_state.max_health_bar_width * (health->current / health->max);
    });
}


} // namespace Game
