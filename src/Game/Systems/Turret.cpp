#include "Game/Systems/Turret.hpp"

#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/JSON.hpp"

#include "Game/Components.hpp"
#include "Game/Utilities.hpp"
#include "Game/APIs/Floor_Manager.hpp"


using std::map;
using std::vector;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;
using Cpp_Utils::read_json_file;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Turret_State
{
    vec3 * position;
    vec3 * look_direction;
    bool * enemy_enabled;
    const vec3 * target_position;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Turret_State> entity_states;
static vector<vector<JSON>> enemy_layouts;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void turret_init()
{
    enemy_layouts = read_json_file("resources/data/enemy_layouts.json").get<vector<vector<JSON>>>();
}


void turret_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        &((Transform *)get_component(entity, "transform"))->position,
        &((Orientation_Handler *)get_component(entity, "orientation_handler"))->look_direction,
        (bool *)get_component(entity, "enemy_enabled"),
        &((Transform *)get_component(get_entity("player"), "transform"))->position,
    };
}


void turret_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void turret_update()
{
    for_each(entity_states, [&](Entity /*entity*/, Turret_State & entity_state) -> void
    {
        if (!*entity_state.enemy_enabled)
        {
            return;
        }

        *entity_state.look_direction = *entity_state.target_position - *entity_state.position;
    });
}


vector<Entity> turret_generate(int /*room*/, int room_origin_x, int room_origin_y)
{
    vector<Entity> turrets;
    const vector<JSON> & enemy_layout = enemy_layouts[random(0, enemy_layouts.size())];
    const vec3 & room_tile_texture_scale = get_room_tile_texture_scale();

    for (const JSON & enemy_position : enemy_layout)
    {
        const int enemy_position_x = room_origin_x + enemy_position["x"].get<int>();
        const int enemy_position_y = room_origin_y + enemy_position["y"].get<int>();

        if (get_room_tile(enemy_position_x, enemy_position_y).type == Tile_Types::FLOOR)
        {
            const Entity turret = load_blueprint("turret");
            turrets.push_back(turret);
            *entity_states[turret].position = vec3(enemy_position_x, enemy_position_y, 0) * room_tile_texture_scale;
        }
    }

    return turrets;
}


} // namespace Game
