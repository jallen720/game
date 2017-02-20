#include "Game/Systems/Game_Manager.hpp"

#include <glm/glm.hpp>
#include "Nito/Components.hpp"

#include "Game/Systems/Room_Generator.hpp"


// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/Components.hpp
using Nito::Transform;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void game_manager_subscribe(Entity /*entity*/)
{
    vec3 & player_position = ((Transform *)get_component(get_entity("player"), "transform"))->position;
    const vec2 & spawn_position = room_generator_get_spawn_position();
    room_generator_run();
    player_position.x = spawn_position.x;
    player_position.y = spawn_position.y;
}


void game_manager_unsubscribe(Entity /*entity*/) {}


} // namespace Game
