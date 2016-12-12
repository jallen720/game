#include "Game/Systems/Depth_Handler.hpp"

#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"


using std::map;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, vec3 *> entity_positions;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void depth_handler_subscribe(const Entity entity)
{
    entity_positions[entity] = &((Transform *)get_component(entity, "transform"))->position;
}


void depth_handler_unsubscribe(const Entity entity)
{

    remove(entity_positions, entity);
}


void depth_handler_update()
{
    for_each(entity_positions, [](const Entity /*entity*/, vec3 * position) -> void
    {
        position->z = position->y;
    });
}


} // namespace Nito_Game
