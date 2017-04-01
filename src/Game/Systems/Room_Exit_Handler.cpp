#include "Game/Systems/Room_Exit_Handler.hpp"

#include <string>
#include <map>
#include <stdexcept>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"

#include "Game/Components.hpp"


using std::string;
using std::map;
using std::runtime_error;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/APIs/Components.hpp
using Nito::Sprite;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains_key;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Room_Exit_Handler_State
{
    Room_Exit * room_exit;
    Sprite * sprite;
    string original_texture_path;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Room_Exit_Handler_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void room_exit_handler_subscribe(Entity entity)
{
    auto sprite = (Sprite *)get_component(entity, "sprite");

    entity_states[entity] =
    {
        (Room_Exit *)get_component(entity, "room_exit"),
        sprite,
        sprite->texture_path,
    };
}


void room_exit_handler_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void room_exit_handler_set_locked(Entity entity, bool locked)
{
    if (!contains_key(entity_states, entity))
    {
        throw runtime_error("ERROR: entity " + to_string(entity) + " is not tracked by the Room_Exit_Handler system!");
    }

    Room_Exit_Handler_State & entity_state = entity_states[entity];
    Room_Exit * room_exit = entity_state.room_exit;

    entity_state.sprite->texture_path =
        (room_exit->locked = locked)
        ? room_exit->locked_texture_path
        : entity_state.original_texture_path;
}


} // namespace Game
