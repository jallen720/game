#include "Game/Systems/Enemy.hpp"

#include "Game/Components.hpp"
#include "Game/Systems/Item.hpp"


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
    ((Health *)get_component(entity, "health"))->death_handlers["enemy"] = [=]() -> void
    {
        check_spawn_item(entity);
        flag_entity_for_deletion(entity);
    };
}


void enemy_unsubscribe(Entity /*entity*/)
{

}


} // namespace Game
