#include "Game/APIs/Enemy_Manager.hpp"

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/JSON.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Game_Manager.hpp"
#include "Game/Systems/Boss.hpp"


using std::string;
using std::vector;
using std::map;

// glm/glm.hpp
using glm::vec3;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;

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
enum class Enemies
{
    TURRET,
    NONE,
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string ROOM_CHANGE_HANDLER_ID("enemy_manager");
static Enemies * enemies;
static const vec3 * room_tile_texture_scale;
static vector<vector<JSON>> enemy_layouts;
static map<string, const JSON> boss_datas;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void generate_enemy(const string & blueprint, const vec3 & position, const int room)
{
    const Entity enemy_entity = load_blueprint(blueprint);
    ((Transform *)get_component(enemy_entity, "transform"))->position = position;

    ((Health *)get_component(enemy_entity, "health"))->death_handlers["enemy_manager"] = [=]() -> void
    {
        // Remove enemy from its associated room's enemy count.
        remove_enemy(room);

        game_manager_untrack_render_flag(room, enemy_entity);
        game_manager_untrack_collider_enabled_flag(room, enemy_entity);
    };

    add_enemy(room);
    game_manager_track_render_flag(room, enemy_entity);
    game_manager_track_collider_enabled_flag(room, enemy_entity);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void enemy_manager_api_init()
{
    enemy_layouts = read_json_file("resources/data/enemy_layouts.json").get<vector<vector<JSON>>>();
    boss_datas = read_json_file("resources/data/bosses.json").get<map<string, const JSON>>();
}


void generate_enemies()
{
    const int floor_size = get_floor_size();
    const int room_tile_width = get_room_tile_width();
    const int room_tile_height = get_room_tile_height();
    const int enemies_width = floor_size * room_tile_width;
    const int enemies_height = floor_size * room_tile_height;
    const int boss_room = get_max_room_id();
    enemies = new Enemies[enemies_width * enemies_height];
    room_tile_texture_scale = &get_room_tile_texture_scale();
    int boss_room_x = 0;
    int boss_room_y = 0;


    // Generate enemy data.
    iterate_array_2d<Enemies>(enemies, enemies_width, enemies_height, [](int /*x*/, int /*y*/, Enemies & enemy) -> void
    {
        enemy = Enemies::NONE;
    });

    iterate_rooms([&](int x, int y, int & room) -> void
    {
        // Don't generate enemies for non-rooms or spawn room.
        if (room == 0 || room == 1)
        {
            return;
        }


        // Don't generate enemies for boss room, but store its coordinates for boss generation.
        if (room == boss_room)
        {
            boss_room_x = x;
            boss_room_y = y;
            return;
        }


        const int origin_x = x * room_tile_width;
        const int origin_y = y * room_tile_height;
        const vector<JSON> & enemy_layout = enemy_layouts[random(0, enemy_layouts.size())];

        for (const JSON & enemy_position : enemy_layout)
        {
            int enemy_position_x = origin_x + enemy_position["x"].get<int>();
            int enemy_position_y = origin_y + enemy_position["y"].get<int>();

            if (get_room_tile(enemy_position_x, enemy_position_y).type == Tile_Types::FLOOR)
            {
                *array_2d_at(enemies, enemies_width, enemy_position_x, enemy_position_y) = Enemies::TURRET;
            }
        }
    });


    // Use enemy data to generate enemy entities.
    iterate_array_2d<Enemies>(enemies, enemies_width, enemies_height, [&](int x, int y, Enemies & enemy) -> void
    {
        static const map<Enemies, const string> ENEMY_BLUEPRINTS
        {
            { Enemies::TURRET, "turret" },
        };

        if (enemy == Enemies::NONE)
        {
            return;
        }

        const vec3 enemy_position = vec3(x, y, 0) * *room_tile_texture_scale;
        generate_enemy(ENEMY_BLUEPRINTS.at(enemy), enemy_position, get_room(enemy_position));
    });


    // Generate boss.
    static const vector<string> BOSS_IDS
    {
        "boss",
    };

    static const map<string, void(*)()> BOSS_INITIALIZERS
    {
        { "boss", boss_init },
    };

    const string & boss_id = BOSS_IDS[random(0, BOSS_IDS.size())];
    const JSON & boss_spawn_position = boss_datas.at(boss_id)["spawn_position"];
    const int boss_room_origin_x = boss_room_x * room_tile_width;
    const int boss_room_origin_y = boss_room_y * room_tile_height;

    const vec3 boss_position =
        vec3(
            boss_room_origin_x + boss_spawn_position["x"].get<int>(),
            boss_room_origin_y + boss_spawn_position["y"].get<int>(),
            0) *
        *room_tile_texture_scale;

    generate_enemy(boss_id, boss_position, boss_room);

    if (contains_key(BOSS_INITIALIZERS, boss_id))
    {
        BOSS_INITIALIZERS.at(boss_id)();
    }

    load_blueprint("boss_health_bar");
}


void destroy_enemies()
{
    delete[] enemies;
}


} // namespace Game
