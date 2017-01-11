#include "Game/Systems/Health.hpp"

#include <map>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"

#include "Game/Components.hpp"


using std::map;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains_key;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Health *> entity_healths;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void health_subscribe(Entity entity)
{
    entity_healths[entity] = (Health *)get_component(entity, "health");
}


void health_unsubscribe(Entity entity)
{
    remove(entity_healths, entity);
}


void damage_entity(Entity entity, float amount)
{
    Health * entity_health = entity_healths[entity];
    float & current_health = entity_health->current;


    // If health is already 0, don't damage any further and don't trigger death handler again.
    if (current_health == 0)
    {
        return;
    }


    current_health -= current_health < amount ? current_health : amount;

    if (current_health == 0)
    {
        entity_health->death_handler();
    }
}


} // namespace Game
