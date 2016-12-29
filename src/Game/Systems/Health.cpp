#include "Game/Systems/Health.hpp"

#include <map>
#include <stdexcept>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"

#include "Game/Components.hpp"


using std::map;
using std::runtime_error;

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
void health_subscribe(const Entity entity)
{
    entity_healths[entity] = (Health *)get_component(entity, "health");
}


void health_unsubscribe(const Entity entity)
{
    remove(entity_healths, entity);
}


void damage_entity(const Entity entity, const float amount)
{
    if (!contains_key(entity_healths, entity))
    {
        throw runtime_error("ERROR: entity " + to_string(entity) + " is not subscribed to the health system!");
    }

    float & current_health = entity_healths[entity]->current;
    current_health -= current_health < amount ? current_health : amount;
}


} // namespace Game
