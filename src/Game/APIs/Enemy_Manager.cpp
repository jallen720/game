#include "Game/APIs/Enemy_Manager.hpp"

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/JSON.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Game_Manager.hpp"


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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void enemy_manager_api_init()
{
    enemy_layouts = read_json_file("resources/data/enemy_layouts.json").get<vector<vector<JSON>>>();
}


void generate_enemies()
{
    const int floor_size = get_floor_size();
    const int room_tile_width = get_room_tile_width();
    const int room_tile_height = get_room_tile_height();
    const int enemies_width = floor_size * room_tile_width;
    const int enemies_height = floor_size * room_tile_height;
    const int max_room_id = get_max_room_id();
    enemies = new Enemies[enemies_width * enemies_height];
    room_tile_texture_scale = &get_room_tile_texture_scale();

    iterate_array_2d<Enemies>(enemies, enemies_width, enemies_height, [](int /*x*/, int /*y*/, Enemies & enemy) -> void
    {
        enemy = Enemies::NONE;
    });

    iterate_rooms([&](int x, int y, int & room) -> void
    {
        // Don't generate enemies for non-rooms, the spawn room, or the boss room.
        if (room == 0 || room == 1 || room == max_room_id)
        {
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

        Entity enemy_entity = load_blueprint(ENEMY_BLUEPRINTS.at(enemy));
        vec3 enemy_position = vec3(x, y, 0) * *room_tile_texture_scale;
        int enemy_room = get_room(enemy_position);
        ((Transform *)get_component(enemy_entity, "transform"))->position = enemy_position;

        ((Health *)get_component(enemy_entity, "health"))->death_handlers["enemy_manager"] = [=]() -> void
        {
            // Remove enemy from its associated room's enemy count.
            remove_enemy(enemy_room);

            game_manager_untrack_render_flag(enemy_room, enemy_entity);
        };

        add_enemy(enemy_room);
        game_manager_track_render_flag(enemy_room, enemy_entity);
    });
}


void destroy_enemies()
{
    delete[] enemies;
}


} // namespace Game
