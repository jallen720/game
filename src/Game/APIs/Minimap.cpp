#include "Game/APIs/Minimap.hpp"

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Resources.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Vector.hpp"

#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Game_Manager.hpp"


using std::string;
using std::vector;
using std::map;

// glm/glm.hpp
using glm::vec3;

// Nito/Components.hpp
using Nito::Transform;
using Nito::UI_Transform;
using Nito::Dimensions;
using Nito::Sprite;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::Component;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Nito/APIs/Graphics.hpp
using Nito::get_pixels_per_unit;

// Nito/APIs/Resources.hpp
using Nito::get_loaded_texture;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::contains;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Minimap_Room
{
    int x;
    int y;
    bool * base_render_flag;
    vector<bool *> occupied_render_flags;
    vector<bool *> vacant_render_flags;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string MINIMAP_ROOM_TEXTURE_PATH("resources/textures/ui/minimap_room.png");
static const string MINIMAP_ROOM_CHANGE_HANDLER_ID("minimap");
static vec3 room_texture_offset;
static map<int, vector<Minimap_Room>> minimap_room_groups;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void generate_minimap_tile(
    const vec3 & position,
    const string & texture_path,
    bool ** render_flag,
    float rotation = 0.0f)
{
    Entity minimap_tile = load_blueprint("minimap_tile");
    const int floor_offset = get_floor_size() - 1;
    auto sprite = (Sprite *)get_component(minimap_tile, "sprite");
    sprite->texture_path = texture_path;
    *render_flag = &sprite->render;

    ((UI_Transform *)get_component(minimap_tile, "ui_transform"))->position =
        (position * room_texture_offset) -
        (room_texture_offset * vec3(floor_offset, floor_offset, 0.0f)) -
        (room_texture_offset * ((Dimensions *)get_component(minimap_tile, "dimensions"))->origin) -
        vec3(0.1f, 0.1f, 0);

    ((Transform *)get_component(minimap_tile, "transform"))->rotation = rotation;
}


static void generate_room_connectors(
    int x,
    int y,
    float rotation,
    vector<bool *> & occupied_render_flags,
    vector<bool *> & vacant_render_flags)
{
    static const string ROOM_CONNECTOR_TEXTURE_PATH = "resources/textures/ui/minimap_room_connector.png";
    static const string ROOM_CONNECTOR_VACANT_TEXTURE_PATH = "resources/textures/ui/minimap_room_connector_vacant.png";

    bool * room_connector_render_flag;
    bool * room_connector_vacant_render_flag;

    generate_minimap_tile(
        vec3(x, y, -1.0f),
        ROOM_CONNECTOR_TEXTURE_PATH,
        &room_connector_render_flag,
        rotation);

    generate_minimap_tile(
        vec3(x, y, -1.0f),
        ROOM_CONNECTOR_VACANT_TEXTURE_PATH,
        &room_connector_vacant_render_flag,
        rotation);

    occupied_render_flags.push_back(room_connector_render_flag);
    vacant_render_flags.push_back(room_connector_vacant_render_flag);
}


static void set_render_flags(const vector<bool *> & render_flags, bool value)
{
    for (bool * render_flag : render_flags)
    {
        *render_flag = value;
    }
}


static void occupy_room(int room)
{
    for (Minimap_Room & minimap_room : minimap_room_groups[room])
    {
        // Ensure base is on.
        *minimap_room.base_render_flag = true;


        // Ensure neighboring bases are on.
        int x = minimap_room.x;
        int y = minimap_room.y;

        vector<int> neighbor_rooms
        {
            get_room(x, y - 1),
            get_room(x - 1, y),
            get_room(x, y + 1),
            get_room(x + 1, y),
        };

        vector<int> flagged_rooms;

        for (int neighbor_room : neighbor_rooms)
        {
            // Don't enable base flag for empty/out-of-bounds rooms, the same room as this, or a room that's already had
            // its base flag set.
            if (neighbor_room <= 0 ||
                neighbor_room == room ||
                contains(flagged_rooms, neighbor_room))
            {
                continue;
            }


            for (Minimap_Room & minimap_neighbor_room : minimap_room_groups[neighbor_room])
            {
                *minimap_neighbor_room.base_render_flag = true;
            }

            flagged_rooms.push_back(neighbor_room);
        }


        set_render_flags(minimap_room.occupied_render_flags, true);
        set_render_flags(minimap_room.vacant_render_flags, false);
    }
}


static void vacate_room(int room)
{
    for (Minimap_Room & minimap_room : minimap_room_groups[room])
    {
        set_render_flags(minimap_room.vacant_render_flags, true);
        set_render_flags(minimap_room.occupied_render_flags, false);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void minimap_api_init()
{
    const Dimensions & dimensions = get_loaded_texture(MINIMAP_ROOM_TEXTURE_PATH).dimensions;
    room_texture_offset = vec3(dimensions.width, dimensions.height, 0.0f) / get_pixels_per_unit();
    room_texture_offset.z = 1.0f;
}


void generate_minimap()
{
    static const string MINIMAP_ROOM_VACANT_TEXTURE_PATH = "resources/textures/ui/minimap_room_vacant.png";
    static const string MINIMAP_ROOM_BASE_TEXTURE_PATH = "resources/textures/ui/minimap_room_base.png";

    game_manager_add_room_change_handler(MINIMAP_ROOM_CHANGE_HANDLER_ID, [](int last_room, int current_room) -> void
    {
        vacate_room(last_room);
        occupy_room(current_room);
    });


    // Generate minimap rooms.
    iterate_rooms([&](int x, int y, const int & room) -> void
    {
        // Don't generate minimap room for empty rooms.
        if (room == 0)
        {
            return;
        }


        // Generate minimap room for current room.
        Minimap_Room minimap_room;
        minimap_room.x = x;
        minimap_room.y = y;
        vector<bool *> & occupied_render_flags = minimap_room.occupied_render_flags;
        vector<bool *> & vacant_render_flags = minimap_room.vacant_render_flags;
        bool * room_render_flag;
        bool * room_vacant_render_flag;
        generate_minimap_tile(vec3(x, y, 0.0f), MINIMAP_ROOM_BASE_TEXTURE_PATH, &minimap_room.base_render_flag);
        generate_minimap_tile(vec3(x, y,-1.0f), MINIMAP_ROOM_TEXTURE_PATH, &room_render_flag);
        generate_minimap_tile(vec3(x, y,-1.0f), MINIMAP_ROOM_VACANT_TEXTURE_PATH, &room_vacant_render_flag);
        occupied_render_flags.push_back(room_render_flag);
        vacant_render_flags.push_back(room_vacant_render_flag);


        // Generate room connector for neighboring rooms that are the same as this room.
        if (get_room(x, y - 1) == room)
        {
            generate_room_connectors(x, y, 0.0f, occupied_render_flags, vacant_render_flags);
        }

        if (get_room(x - 1, y) == room)
        {
            generate_room_connectors(x, y, 270.0f, occupied_render_flags, vacant_render_flags);
        }

        if (get_room(x, y + 1) == room)
        {
            generate_room_connectors(x, y, 180.0f, occupied_render_flags, vacant_render_flags);
        }

        if (get_room(x + 1, y) == room)
        {
            generate_room_connectors(x, y, 90.0f, occupied_render_flags, vacant_render_flags);
        }

        minimap_room_groups[room].push_back(minimap_room);
    });


    // Initialize minimap.
    for_each(minimap_room_groups, [](int /*room*/, const vector<Minimap_Room> & minimap_room_group) -> void
    {
        for (const Minimap_Room & minimap_room : minimap_room_group)
        {
            *minimap_room.base_render_flag = false;
            set_render_flags(minimap_room.occupied_render_flags, false);
            set_render_flags(minimap_room.vacant_render_flags, false);
        }
    });

    occupy_room(1);
}


void destroy_minimap()
{
    minimap_room_groups.clear();
    game_manager_remove_room_change_handler(MINIMAP_ROOM_CHANGE_HANDLER_ID);
}


} // namespace Game
