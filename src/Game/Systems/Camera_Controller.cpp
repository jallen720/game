#include "Game/Systems/Camera_Controller.hpp"

#include <map>
#include <string>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"


using std::map;
using std::string;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_entity;
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
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Entity_State
{
    Transform * transform;
    const Transform * target_transform;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void camera_controller_subscribe(const Entity entity)
{
    const auto target_id = (string *)get_component(entity, "target_id");

    entity_states[entity] =
    {
        (Transform *)get_component(entity, "transform"),
        (Transform *)get_component(get_entity(*target_id), "transform"),
    };
}


void camera_controller_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void camera_controller_update()
{
    for_each(entity_states, [](const Entity /*entity*/, Entity_State & entity_state) -> void
    {
        entity_state.transform->position = entity_state.target_transform->position;
    });
}


} // namespace Nito_Game
