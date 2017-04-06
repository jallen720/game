#include "Game/Systems/Room_Exit_Handler.hpp"

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"

#include "Game/Components.hpp"


using std::string;
using std::vector;
using std::map;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/APIs/Components.hpp
using Nito::Sprite;
using Nito::Transform;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

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
    Transform * transform;
    Sprite * sprite;
    string original_texture_path;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Room_Exit_Handler_State> entity_states;
static vector<Transform *> unused_door_lock_transforms;
static map<Entity, Transform *> used_door_lock_transforms;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void set_door_lock(Entity entity, const vec3 & position, float rotation)
{
    Transform * door_lock;

    if (unused_door_lock_transforms.size() > 0)
    {
        door_lock = unused_door_lock_transforms.back();
        unused_door_lock_transforms.pop_back();
    }
    else
    {
        door_lock = (Transform *)get_component(load_blueprint("door_lock_tile"), "transform");
    }

    used_door_lock_transforms[entity] = door_lock;
    door_lock->position = position;
    door_lock->rotation = rotation;
}


static void unset_door_lock(Entity entity)
{
    static const vec3 UNUSED_DOOR_LOCK_POSITION(-100, -100, -100);

    if (!contains_key(used_door_lock_transforms, entity))
    {
        throw runtime_error(
            "ERROR: attempting to unset door lock from entity " + to_string(entity) + " which does not have a door "
            "lock associated with it!");
    }


    Transform * door_lock = used_door_lock_transforms[entity];
    remove(used_door_lock_transforms, entity);
    unused_door_lock_transforms.push_back(door_lock);
    door_lock->position = UNUSED_DOOR_LOCK_POSITION;
}


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
        (Transform *)get_component(entity, "transform"),
        sprite,
        sprite->texture_path,
    };
}


void room_exit_handler_unsubscribe(Entity entity)
{
    if (contains_key(used_door_lock_transforms, entity))
    {
        unset_door_lock(entity);
    }

    remove(entity_states, entity);


    // When no more exit handlers are subscribed then scene is being changed and door locks are no longer valid.
    if (entity_states.size() == 0)
    {
        unused_door_lock_transforms.clear();
        used_door_lock_transforms.clear();
    }
}


void room_exit_handler_set_locked(Entity entity, bool locked)
{
    if (!contains_key(entity_states, entity))
    {
        throw runtime_error("ERROR: entity " + to_string(entity) + " is not tracked by the Room_Exit_Handler system!");
    }


    // Update texture path based on whether the exit is locked or not.
    Room_Exit_Handler_State & entity_state = entity_states[entity];
    Room_Exit * room_exit = entity_state.room_exit;

    entity_state.sprite->texture_path =
        (room_exit->locked = locked)
        ? room_exit->locked_texture_path
        : entity_state.original_texture_path;


    // Handle door-lock entities that prevent the player from passing through locked doors for door-type room-exits.
    if (room_exit->type == Room_Exit::Types::DOOR)
    {
        if (locked)
        {
            Transform * transform = entity_state.transform;
            set_door_lock(entity, transform->position, transform->rotation);
        }
        else
        {
            unset_door_lock(entity);
        }
    }
}


} // namespace Game
