#include "Game/Systems/Health.hpp"

#include <map>
#include <string>
#include <functional>
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;
using std::string;
using std::function;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


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
        for_each(entity_health->death_handlers, [](const string & /*id*/, const function<void()> & handler) -> void
        {
            handler();
        });
    }
}


void heal_entity(Entity entity, float amount)
{
    Health * entity_health = entity_healths[entity];
    float & current_health = entity_health->current;
    const float max_health = entity_health->max;
    current_health += amount;

    if (current_health > max_health)
    {
        current_health = max_health;
    }
}


} // namespace Game
