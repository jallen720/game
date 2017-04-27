#include "Game/APIs/Enemy_Manager.hpp"

#include <string>
#include <vector>
#include <map>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Game_Manager.hpp"
#include "Game/Systems/Boss.hpp"
#include "Game/Systems/Turret.hpp"
#include "Game/Systems/Wall_Launcher.hpp"


using std::string;
using std::vector;
using std::map;

// Nito/Components.hpp
using Nito::Sprite;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_entity;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Cpp_Utils/Map.hpp
using Cpp_Utils::contains_key;


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
    WALL_LAUNCHER,
    NONE,
};


using Enemy_Generator = vector<Entity>(*)(int, int, int);
using Boss_Generator = Entity(*)(int, int);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string ROOM_CHANGE_HANDLER_ID("enemy_manager");


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void track_enemy(Entity enemy_entity, int room)
{
    ((Health *)get_component(enemy_entity, "health"))->death_handlers["enemy_manager enemy death"] = [=]() -> void
    {
        // Remove enemy from its associated room's enemy count.
        remove_enemy(room);

        game_manager_untrack_render_flag(room, enemy_entity);
        game_manager_untrack_collider_enabled_flag(room, enemy_entity);
        game_manager_untrack_enemy_enabled_flag(room, enemy_entity);
    };

    add_enemy(room);
    game_manager_track_render_flag(room, enemy_entity);
    game_manager_track_collider_enabled_flag(room, enemy_entity);
    game_manager_track_enemy_enabled_flag(room, enemy_entity);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generate_enemies()
{
    const int room_tile_width = get_room_tile_width();
    const int room_tile_height = get_room_tile_height();
    const int boss_room = get_max_room_id();
    int boss_room_origin_x = 0;
    int boss_room_origin_y = 0;
    map<int, vector<Enemies>> room_enemy_groups;

    iterate_rooms([&](int x, int y, int & room) -> void
    {
        static const vector<Enemies> POSSIBLE_ENEMIES
        {
            Enemies::TURRET,
            Enemies::WALL_LAUNCHER,
        };


        // Don't generate enemies for non-rooms or spawn room.
        if (room == 0 || room == 1)
        {
            return;
        }


        // Don't generate enemies for boss room, but store its origin coordinates for boss generation.
        if (room == boss_room)
        {
            boss_room_origin_x = room_tile_width * x;
            boss_room_origin_y = room_tile_height * y;
            return;
        }


        if (!contains_key(room_enemy_groups, room))
        {
            vector<Enemies> & enemy_group = room_enemy_groups[room];

            for (const Enemies enemy : POSSIBLE_ENEMIES)
            {
                if (random(0, 2) == 1)
                {
                    enemy_group.push_back(enemy);
                }
            }


            // Ensure each enemy group has atleast 1 enemy.
            if (enemy_group.size() == 0)
            {
                enemy_group.push_back(POSSIBLE_ENEMIES[random(0, POSSIBLE_ENEMIES.size())]);
            }
        }
    });


    // Use enemy data to generate enemy entities.
    iterate_rooms([&](int x, int y, int & room) -> void
    {
        static const map<Enemies, Enemy_Generator> ENEMY_GENERATORS
        {
            { Enemies::TURRET        , turret_generate        },
            { Enemies::WALL_LAUNCHER , wall_launcher_generate },
        };

        for (const Enemies enemy : room_enemy_groups[room])
        {
            const vector<Entity> generated_enemies = ENEMY_GENERATORS.at(enemy)(
                room,
                x * room_tile_width,
                y * room_tile_height);


            // Track enemies generated for this room.
            for (const Entity enemy_entity : generated_enemies)
            {
                track_enemy(enemy_entity, room);
            }
        }
    });


    // Generate boss.
    static const vector<string> BOSS_IDS
    {
        "boss",
    };

    static const map<string, Boss_Generator> BOSS_GENERATORS
    {
        { "boss", boss_generate },
    };

    const string & boss_id = BOSS_IDS[random(0, BOSS_IDS.size())];
    const Entity boss = BOSS_GENERATORS.at(boss_id)(boss_room_origin_x, boss_room_origin_y);
    track_enemy(boss, boss_room);

    bool * boss_health_bar_backround_render =
        &((Sprite *)get_component(get_entity("boss_health_bar_background"), "sprite"))->render;

    game_manager_add_room_change_handler(ROOM_CHANGE_HANDLER_ID, [=](int /*room_a*/, int room_b) -> void
    {
        if (room_b == boss_room)
        {
            load_blueprint("boss_health_bar");
            *boss_health_bar_backround_render = true;
        }
    });

    ((Health *)get_component(boss, "health"))->death_handlers["enemy_manager boss death"] = [=]() -> void
    {
        *boss_health_bar_backround_render = false;


        // Prevent loading health bar when entering boss room after boss has already died.
        game_manager_remove_room_change_handler(ROOM_CHANGE_HANDLER_ID);
    };
}


} // namespace Game
