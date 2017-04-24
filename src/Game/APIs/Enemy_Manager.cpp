#include "Game/APIs/Enemy_Manager.hpp"

#include <string>
#include <vector>
#include <map>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"

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


struct Room_Enemy_Group
{
    int room_id;
    int room_origin_x;
    int room_origin_y;
    vector<Enemies> enemies;
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
    vector<Room_Enemy_Group> room_enemy_groups;

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


        // Create and populate room enemy group.
        Room_Enemy_Group room_enemy_group;
        room_enemy_group.room_id = room;
        room_enemy_group.room_origin_x = x * room_tile_width;
        room_enemy_group.room_origin_y = y * room_tile_height;
        vector<Enemies> & enemies = room_enemy_group.enemies;

        for (const Enemies enemy : POSSIBLE_ENEMIES)
        {
            enemies.push_back(enemy);
        }

        room_enemy_groups.push_back(room_enemy_group);
    });


    // Use enemy data to generate enemy entities.
    for (const Room_Enemy_Group & room_enemy_group : room_enemy_groups)
    {
        static const map<Enemies, Enemy_Generator> ENEMY_GENERATORS
        {
            { Enemies::TURRET        , turret_generate        },
            { Enemies::WALL_LAUNCHER , wall_launcher_generate },
        };

        for (const Enemies enemy : room_enemy_group.enemies)
        {
            const int room_id = room_enemy_group.room_id;

            const vector<Entity> generated_enemies = ENEMY_GENERATORS.at(enemy)(
                room_id,
                room_enemy_group.room_origin_x,
                room_enemy_group.room_origin_y);


            // Track enemies generated for this room.
            for (const Entity enemy_entity : generated_enemies)
            {
                track_enemy(enemy_entity, room_id);
            }
        }
    }


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
