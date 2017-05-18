#include "Game/Systems/Health_Item.hpp"

#include "Game/Components.hpp"
#include "Game/Systems/Health.hpp"


// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void health_item_subscribe(Entity entity)
{
    const auto health_item = (Health_Item *)get_component(entity, "health_item");

    ((Item *)get_component(entity, "item"))->pick_up_handler = [=](Entity player) -> bool
    {
        auto player_health = (Health *)get_component(player, "health");

        if (player_health->current < player_health->max)
        {
            heal_entity(player, health_item->health_restored);
            return true;
        }
        else
        {
            return false;
        }
    };
}


void health_item_unsubscribe(Entity /*entity*/)
{

}


} // namespace Game
