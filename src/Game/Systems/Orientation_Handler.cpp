#include "Game/Systems/Orientation_Handler.hpp"

#include <map>
#include <string>
#include <cmath>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;
using std::string;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Sprite;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Orientation_Handler_State
{
    Sprite * sprite;
    Orientation_Handler * orientation_handler;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Orientation_Handler_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Orientation get_orientation(const vec3 & look_direction)
{
    if (fabsf(look_direction.x) > fabsf(look_direction.y))
    {
        return look_direction.x < 0 ? Orientation::LEFT : Orientation::RIGHT;
    }
    else
    {
        return look_direction.y < 0 ? Orientation::DOWN : Orientation::UP;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void orientation_handler_subscribe(const Entity entity)
{
    entity_states[entity] =
    {
        (Sprite *)get_component(entity, "sprite"),
        (Orientation_Handler *)get_component(entity, "orientation_handler"),
    };
}


void orientation_handler_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void orientation_handler_update()
{
    for_each(entity_states, [](const Entity /*entity*/, Orientation_Handler_State & entity_state) -> void
    {
        Orientation_Handler * orientation_handler = entity_state.orientation_handler;
        Orientation & orientation = orientation_handler->orientation;
        orientation = get_orientation(orientation_handler->look_direction);
        entity_state.sprite->texture_path = orientation_handler->orientation_texture_paths.at(orientation);
    });
}


} // namespace Game
