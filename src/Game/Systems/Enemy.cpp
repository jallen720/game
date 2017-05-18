#include "Game/Systems/Enemy.hpp"

#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Scene.hpp"

#include "Game/Components.hpp"


// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::flag_entity_for_deletion;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void enemy_subscribe(Entity entity)
{
    const vec3 * position = &((Transform *)get_component(entity, "transform"))->position;

    ((Health *)get_component(entity, "health"))->death_handlers["flag_entity_for_deletion"] = [=]() -> void
    {
        // Spawn item.
        Entity item = load_blueprint("mini_health");
        ((Transform *)get_component(item, "transform"))->position = *position;


        flag_entity_for_deletion(entity);
    };
}


void enemy_unsubscribe(Entity /*entity*/)
{

}


} // namespace Game
