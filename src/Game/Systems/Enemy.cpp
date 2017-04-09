#include "Game/Systems/Enemy.hpp"

#include "Game/Components.hpp"


// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::flag_entity_for_deletion;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void enemy_subscribe(Entity entity)
{
    ((Health *)get_component(entity, "health"))->death_handlers["flag_entity_for_deletion"] = [=]() -> void
    {
        flag_entity_for_deletion(entity);
    };
}


void enemy_unsubscribe(Entity /*entity*/)
{

}


} // namespace Game
