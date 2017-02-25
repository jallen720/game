#include "Game/Systems/Game_Manager.hpp"

#include <map>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/APIs/Floor_Generator.hpp"
#include "Game/APIs/Minimap.hpp"


using std::string;
using std::function;
using std::map;
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

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, function<void(char, char)>> room_change_handlers;
static vec3 * player_position;
static char last_room;
static char current_room;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void game_manager_subscribe(Entity /*entity*/)
{
    static const char SPAWN_ROOM = '1';

    player_position = &((Transform *)get_component(get_entity("player"), "transform"))->position;
    last_room = SPAWN_ROOM;
    current_room = SPAWN_ROOM;
    const vec2 & spawn_position = get_spawn_position();
    generate_floor();
    minimap_api_init();
    generate_minimap();
    player_position->x = spawn_position.x;
    player_position->y = spawn_position.y;
}


void game_manager_unsubscribe(Entity /*entity*/)
{
    destroy_floor();
    destroy_minimap();
    room_change_handlers.clear();
    player_position = nullptr;
}


void game_manager_change_rooms(float door_rotation)
{
    static const float PLAYER_MOVEMENT_VALUE = 0.7f;

    last_room = current_room;

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

    current_room = get_room(*player_position);


    // Trigger room-change handlers.
    for_each(room_change_handlers, [=](
        const string & /*id*/,
        const function<void(char, char)> & room_change_handler) -> void
    {
        room_change_handler(last_room, current_room);
    });
}


void game_manager_add_room_change_handler(const string & id, const function<void(char, char)> & room_change_handler)
{
    room_change_handlers[id] = room_change_handler;
}


void game_manager_remove_room_change_handler(const string & id)
{
    remove(room_change_handlers, id);
}


} // namespace Game
