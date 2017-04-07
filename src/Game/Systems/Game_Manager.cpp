#include "Game/Systems/Game_Manager.hpp"

#include <map>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Cpp_Utils/String.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/APIs/Floor_Manager.hpp"
#include "Game/APIs/Enemy_Manager.hpp"
#include "Game/APIs/Minimap.hpp"
#include "Game/Systems/Floor_Entity.hpp"


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
using Nito::Sprite;

// Nito/Collider_Component.hpp
using Nito::Collider;

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
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Room_Flags = map<int, map<Entity, bool *>>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, function<void(int, int)>> room_change_handlers;
static vec3 * player_position;
static const vec2 * spawn_position;
static int last_room;
static int current_room;
static int spawn_room_id;
static Room_Flags render_flags;
static Room_Flags collider_enabled_flags;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void set_room_flags(Room_Flags & room_flags, int room, bool value)
{
    for_each(room_flags[room], [=](Entity /*entity*/, bool * room_flag) -> void
    {
        *room_flag = value;
    });
}


static void start_floor()
{
    last_room = spawn_room_id;
    current_room = spawn_room_id;
    generate_floor(5);
    generate_minimap();
    generate_enemies();
    player_position->x = spawn_position->x;
    player_position->y = spawn_position->y;


    // Initialize render flags.
    for_each(render_flags, [](int room, const map<Entity, bool *> & /*render_flags*/) -> void
    {
        set_room_flags(render_flags, room, false);
    });

    for_each(collider_enabled_flags, [](int room, const map<Entity, bool *> & /*collider_enabled_flags*/) -> void
    {
        set_room_flags(collider_enabled_flags, room, false);
    });

    set_room_flags(render_flags, spawn_room_id, true);
    set_room_flags(collider_enabled_flags, spawn_room_id, true);
}


static void cleanup_floor()
{
    destroy_floor();
    destroy_minimap();
    destroy_enemies();
    render_flags.clear();
    collider_enabled_flags.clear();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void game_manager_subscribe(Entity /*entity*/)
{
    spawn_room_id = get_spawn_room_id();
    player_position = &((Transform *)get_component(get_entity("player"), "transform"))->position;
    spawn_position = &get_spawn_position();
    minimap_api_init();
    start_floor();
}


void game_manager_unsubscribe(Entity /*entity*/)
{
    cleanup_floor();
    room_change_handlers.clear();
    player_position = nullptr;
    spawn_position = nullptr;
}


void game_manager_change_rooms(float door_rotation)
{
    static const float PLAYER_MOVEMENT_VALUE = 1.1f;

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
        const function<void(int, int)> & room_change_handler) -> void
    {
        room_change_handler(last_room, current_room);
    });


    // Update room flags.
    set_room_flags(render_flags, last_room, false);
    set_room_flags(render_flags, current_room, true);
    set_room_flags(collider_enabled_flags, last_room, false);
    set_room_flags(collider_enabled_flags, current_room, true);
}


void game_manager_add_room_change_handler(const string & id, const function<void(int, int)> & room_change_handler)
{
    room_change_handlers[id] = room_change_handler;
}


void game_manager_remove_room_change_handler(const string & id)
{
    remove(room_change_handlers, id);
}


void game_manager_complete_floor()
{
    floor_entity_destroy_all();
    cleanup_floor();
    start_floor();
}


void game_manager_track_render_flag(int room, Entity entity)
{
    render_flags[room][entity] = &((Sprite *)get_component(entity, "sprite"))->render;
}


void game_manager_untrack_render_flag(int room, Entity entity)
{
    remove(render_flags[room], entity);
}


void game_manager_track_collider_enabled_flag(int room, Entity entity)
{
    collider_enabled_flags[room][entity] = &((Collider *)get_component(entity, "collider"))->enabled;
}


void game_manager_untrack_collider_enabled_flag(int room, Entity entity)
{
    remove(collider_enabled_flags[room], entity);
}


} // namespace Game
