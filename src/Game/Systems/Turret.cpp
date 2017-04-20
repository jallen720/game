#include "Game/Systems/Turret.hpp"

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Nito/Engine.hpp"
#include "Nito/Components.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/JSON.hpp"

#include "Game/Components.hpp"
#include "Game/Utilities.hpp"
#include "Game/APIs/Floor_Manager.hpp"


using std::map;
using std::string;
using std::vector;

// glm/glm.hpp
using glm::vec2;
using glm::vec3;
using glm::distance;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;

// Nito/APIs/ECS.hpp
using Nito::flag_entity_for_deletion;

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
    Transform * transform;
    Orientation_Handler * orientation_handler;
    bool * enemy_enabled;
    const vec3 * target_position;
    float cooldown;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const float FIRE_COOLDOWN = 1.0f;
static const vector<string> TARGET_LAYERS { "player" };
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
        (Transform *)get_component(entity, "transform"),
        (Orientation_Handler *)get_component(entity, "orientation_handler"),
        (bool *)get_component(entity, "enemy_enabled"),
        &((Transform *)get_component(get_entity("player"), "transform"))->position,
        0.0f,
    };
}


void turret_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void turret_update()
{
    static const float TURRET_RANGE = 3.0f;
    static const string PROJECTILE_NAME("projectile_red_orb");

    const float delta_time = get_delta_time() * get_time_scale();

    for_each(entity_states, [&](Entity /*entity*/, Turret_State & entity_state) -> void
    {
        if (!*entity_state.enemy_enabled)
        {
            return;
        }

        const vec3 & position = entity_state.transform->position;
        const vec3 & target_position = *entity_state.target_position;
        float & cooldown = entity_state.cooldown;
        vec3 & look_direction = entity_state.orientation_handler->look_direction;
        look_direction = target_position - position;

        if (cooldown > 0.0f)
        {
            cooldown -= delta_time;
        }
        else if (distance((vec2)target_position, (vec2)position) < TURRET_RANGE)
        {
            static const vector<vec3> FIRE_POSITION_OFFSETS
            {
                vec3(-0.3f      , 0.1f  , 0), // Left
                vec3( 0.284375f , 0.1f  , 0), // Right
                vec3(-0.015625f ,-0.08f , 0), // Down
                vec3(-0.015625f , 0.3f  , 0), // Up
            };

            int fire_position_offset_index =
                fabsf(look_direction.x) > fabsf(look_direction.y)
                ? (look_direction.x < 0 ? 0 : 1)
                : (look_direction.y < 0 ? 2 : 3);

            fire_projectile(
                PROJECTILE_NAME,
                position + FIRE_POSITION_OFFSETS[fire_position_offset_index],
                look_direction,
                1.0f,
                TARGET_LAYERS);

            cooldown = FIRE_COOLDOWN;
        }
    });
}


vector<Entity> turret_generate(int room_origin_x, int room_origin_y)
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

            entity_states[turret].transform->position =
                vec3(enemy_position_x, enemy_position_y, 0) * room_tile_texture_scale;
        }
    }

    return turrets;
}


} // namespace Game
