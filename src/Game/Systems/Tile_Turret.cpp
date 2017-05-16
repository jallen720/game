#include "Game/Systems/Tile_Turret.hpp"

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include "Nito/Engine.hpp"
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Turret.hpp"


using std::map;
using std::vector;

// glm/glm.hpp
using glm::ivec2;
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_entity;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Sprite;

// Nito/Collider_Component.hpp
using Nito::Collider;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;
using Cpp_Utils::remove;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::contains;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Tile_Turret_State
{
    vec3 * position;
    vec3 * look_direction;
    bool * enemy_enabled;
    bool * render;
    bool * collider_enabled;
    bool * enemy_projectile_launcher_enabled;
    const vec3 * target_position;
    int room;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Tile_Turret_State> entity_states;
static map<int, vector<ivec2>> room_floor_tiles;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_random_position(Tile_Turret_State & entity_state)
{
    const int room = entity_state.room;
    vector<ivec2> turret_tiles;
    const map<int, map<Entity, ivec2>> & turret_room_tiles = get_turret_room_tiles();

    if (contains_key(turret_room_tiles, room))
    {
        for_each(turret_room_tiles.at(room), [&](Entity /*turret*/, const ivec2 & turret_tile) -> void
        {
            turret_tiles.emplace_back(turret_tile.x, turret_tile.y);
        });
    }

    const vector<ivec2> & tiles = room_floor_tiles.at(entity_state.room);
    const ivec2 * tile;


    // Ensure tile turret does not overlap a turret.
    do
    {
        tile = &tiles[random(0, tiles.size())];
    }
    while (contains(turret_tiles, *tile));


    *entity_state.position = vec3(tile->x, tile->y, 0) * get_room_tile_unit_size();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void tile_turret_init()
{
    add_floor_generated_handler("tile_turret", [&]() -> void
    {
        room_floor_tiles.clear();

        iterate_room_tiles([&](int x, int y, const Tile & tile) -> void
        {
            if (tile.type == Tile_Types::FLOOR)
            {
                room_floor_tiles[tile.room].emplace_back(x, y);
            }
        });
    });
}


void tile_turret_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        &((Transform *)get_component(entity, "transform"))->position,
        &((Orientation_Handler *)get_component(entity, "orientation_handler"))->look_direction,
        (bool *)get_component(entity, "enemy_enabled"),
        &((Sprite *)get_component(entity, "sprite"))->render,
        &((Collider *)get_component(entity, "collider"))->enabled,
        &((Enemy_Projectile_Launcher *)get_component(entity, "enemy_projectile_launcher"))->enabled,
        &((Transform *)get_component(get_entity("player"), "transform"))->position,
        -1,
    };
}


void tile_turret_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void tile_turret_update()
{
    static const float UP_TIME = 3;
    static const float DOWN_TIME = 2;
    static bool up = false;
    static float time = 0;


    // Don't update when game is paused or no entities are subscribed.
    if (entity_states.size() == 0 || get_time_scale() < 1)
    {
        return;
    }


    time -= get_delta_time();

    if (time <= 0)
    {
        if (up)
        {
            time = DOWN_TIME;
        }
        else
        {
            time = UP_TIME;

            for_each(entity_states, [&](Entity /*entity*/, Tile_Turret_State & entity_state) -> void
            {
                set_random_position(entity_state);
            });
        }

        up = !up;
    }

    for_each(entity_states, [&](Entity /*entity*/, Tile_Turret_State & entity_state) -> void
    {
        if (!*entity_state.enemy_enabled)
        {
            return;
        }

        *entity_state.look_direction = *entity_state.target_position - *entity_state.position;
        *entity_state.render = up;
        *entity_state.collider_enabled = up;
        *entity_state.enemy_projectile_launcher_enabled = up;
    });
}


vector<Entity> tile_turret_generate(int room, int /*room_origin_x*/, int /*room_origin_y*/)
{
    vector<Entity> tile_turrets;

    for (int i = 0; i < 2; i++)
    {
        const Entity tile_turret = load_blueprint("tile_turret");
        tile_turrets.push_back(tile_turret);
        Tile_Turret_State & entity_state = entity_states[tile_turret];
        entity_state.room = room;
    }

    return tile_turrets;
}


} // namespace Game
