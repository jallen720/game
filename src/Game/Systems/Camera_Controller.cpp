#include "Game/Systems/Camera_Controller.hpp"

#include <map>
#include <string>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Game/APIs/Floor_Manager.hpp"


using std::map;
using std::string;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

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
        vec3 & position = entity_state.transform->position;
        const vec3 & target_position = entity_state.target_transform->position;
        float & position_x = position.x;
        float & position_y = position.y;
        int target_room = get_room(target_position);

        if (target_room <= 0)
        {
            throw runtime_error("ERROR: camera cannot follow target as target is out-of-bounds!");
        }

        const Room_Data & target_room_data = get_room_data(target_room);
        const vec2 & target_room_origin = target_room_data.origin;
        const vec2 & target_room_bounds = target_room_data.bounds;
        const float target_room_origin_x = target_room_origin.x;
        const float target_room_origin_y = target_room_origin.y;
        const float target_room_bounds_x = target_room_bounds.x;
        const float target_room_bounds_y = target_room_bounds.y;
        position = target_position;

        if (position_x < target_room_origin_x)
        {
            position_x = target_room_origin_x;
        }
        else if (position_x > target_room_bounds_x)
        {
            position_x = target_room_bounds_x;
        }

        if (position_y < target_room_origin_y)
        {
            position_y = target_room_origin_y;
        }
        else if (position_y > target_room_bounds_y)
        {
            position_y = target_room_bounds_y;
        }
    });
}


} // namespace Nito_Game
