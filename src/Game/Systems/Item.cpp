#include "Game/Systems/Item.hpp"

#include <vector>
#include <string>
#include "Nito/Collider_Component.hpp"
#include "Cpp_Utils/Vector.hpp"

#include "Game/Components.hpp"


using std::vector;
using std::string;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::has_component;
using Nito::flag_entity_for_deletion;

// Nito/Collider_Component.hpp
using Nito::Collider;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::contains;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void item_subscribe(Entity entity)
{
    static const string PLAYER_LAYER("player");
    static const string LAYERS_COMPONENT("layers");

    const auto item = (Item *)get_component(entity, "item");

    ((Collider *)get_component(entity, "collider"))->collision_handler = [=](Entity collision_entity) -> void
    {
        if (has_component(collision_entity, LAYERS_COMPONENT) &&
            contains(*(vector<string> *)get_component(collision_entity, LAYERS_COMPONENT), PLAYER_LAYER))
        {
            if (item->pick_up_handler(collision_entity))
            {
                flag_entity_for_deletion(entity);
            }
        }
    };
}


void item_unsubscribe(Entity /*entity*/)
{

}


} // namespace Game
