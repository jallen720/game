#include "Game/Systems/Game_Manager.hpp"

#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Cpp_Utils/String.hpp"

#include "Game/APIs/Floor_Generator.hpp"
#include "Game/APIs/Minimap.hpp"


using std::runtime_error;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/Components.hpp
using Nito::Transform;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vec3 * player_position;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void game_manager_subscribe(Entity /*entity*/)
{
    player_position = &((Transform *)get_component(get_entity("player"), "transform"))->position;
    const vec2 & spawn_position = get_spawn_position();
    generate_floor();
    minimap_api_init();
    generate_minimap();
    player_position->x = spawn_position.x;
    player_position->y = spawn_position.y;
}


void game_manager_unsubscribe(Entity /*entity*/) {}


void game_manager_change_rooms(float door_rotation)
{
    static const float PLAYER_MOVEMENT_VALUE = 0.7f;

    if (door_rotation == 0.0f)
    {
        (*player_position) += vec3(0.0f, -PLAYER_MOVEMENT_VALUE, 0.0f);
    }
    else if (door_rotation == 270.0f)
    {
        (*player_position) += vec3(-PLAYER_MOVEMENT_VALUE, 0.0f, 0.0f);
    }
    else if (door_rotation == 180.0f)
    {
        (*player_position) += vec3(0.0f, PLAYER_MOVEMENT_VALUE, 0.0f);
    }
    else if (door_rotation == 90.0f)
    {
        (*player_position) += vec3(PLAYER_MOVEMENT_VALUE, 0.0f, 0.0f);
    }
    else
    {
        throw runtime_error("ERROR: invalid door rotation: " + to_string(door_rotation) + "!");
    }
}


} // namespace Game
