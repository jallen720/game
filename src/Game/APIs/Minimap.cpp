#include "Game/APIs/Minimap.hpp"

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Resources.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/APIs/Floor_Generator.hpp"
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
using Nito::generate_entity;

// Nito/APIs/Graphics.hpp
using Nito::get_pixels_per_unit;

// Nito/APIs/Resources.hpp
using Nito::get_loaded_texture;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Minimap_Room
{
    bool * base_flag;
    vector<bool *> occupied_flags;
    vector<bool *> vacant_flags;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string MINIMAP_ROOM_TEXTURE_PATH = "resources/textures/ui/minimap_room.png";
static const string MINIMAP_ROOM_CHANGE_HANDLER_ID = "minimap";
static vec3 room_texture_offset;
static map<char, vector<Minimap_Room>> minimap_room_groups;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, Component> get_room_components(int x, int y, float z, const string & texture_path)
{
    const vec3 position = (vec3(x, y, z) * room_texture_offset) - vec3(2.0f, 2.0f, 0.0f);

    return
    {
        { "render_layer" , new string("ui")                                      },
        { "transform"    , new Transform { vec3(0.0f), vec3(1.0f), 0.0f }        },
        { "ui_transform" , new UI_Transform { position, vec3(1.0f) }             },
        { "dimensions"   , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) } },
        { "sprite"       , new Sprite { true, texture_path, "texture" }          },
    };
}


static void generate_room(const map<string, Component> & room_components, bool ** render_flag)
{
    static const vector<string> MINIMAP_ROOM_SYSTEMS
    {
        "ui_transform",
        "sprite_dimensions_handler",
        "renderer",
    };

    *render_flag = &((Sprite *)room_components.at("sprite"))->render;
    generate_entity(room_components, MINIMAP_ROOM_SYSTEMS);
}


static void generate_room_connector(int x, int y, float rotation, const string & texture_path, bool ** render_flag)
{
    map<string, Component> room_connector_components = get_room_components(x, y, -1.0f, texture_path);
    ((Transform *)room_connector_components["transform"])->rotation = rotation;
    return generate_room(room_connector_components, render_flag);
}


static void show(const vector<bool *> & render_flags)
{
    for (bool * render_flag : render_flags)
    {
        *render_flag = true;
    }
}


static void hide(const vector<bool *> & render_flags)
{
    for (bool * render_flag : render_flags)
    {
        *render_flag = false;
    }
}


static void occupy_room(char room)
{
    for (Minimap_Room & minimap_room : minimap_room_groups[room])
    {
        show(minimap_room.occupied_flags);
        hide(minimap_room.vacant_flags);
    }
}


static void vacate_room(char room)
{
    for (Minimap_Room & minimap_room : minimap_room_groups[room])
    {
        show(minimap_room.vacant_flags);
        hide(minimap_room.occupied_flags);
    }
}


static void generate_room_connectors(
    int x,
    int y,
    float rotation,
    vector<bool *> & occupied_flags,
    vector<bool *> & vacant_flags)
{
    static const string ROOM_CONNECTOR_TEXTURE_PATH = "resources/textures/ui/minimap_room_connector.png";
    static const string ROOM_CONNECTOR_VACANT_TEXTURE_PATH = "resources/textures/ui/minimap_room_connector_vacant.png";

    bool * room_connector_render_flag;
    bool * room_connector_vacant_render_flag;
    generate_room_connector(x, y, rotation, ROOM_CONNECTOR_TEXTURE_PATH, &room_connector_render_flag);
    generate_room_connector(x, y, rotation, ROOM_CONNECTOR_VACANT_TEXTURE_PATH, &room_connector_vacant_render_flag);
    occupied_flags.push_back(room_connector_render_flag);
    vacant_flags.push_back(room_connector_vacant_render_flag);
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

    game_manager_add_room_change_handler(MINIMAP_ROOM_CHANGE_HANDLER_ID, [](char last_room, char current_room) -> void
    {
        vacate_room(last_room);
        occupy_room(current_room);
    });


    // Generate minimap rooms.
    iterate_current_floor_rooms([&](int x, int y, const char & room) -> void
    {
        // Don't generate minimap room for empty rooms.
        if (room == '0')
        {
            return;
        }


        // Generate minimap room for current room.
        Minimap_Room minimap_room;
        vector<bool *> & occupied_flags = minimap_room.occupied_flags;
        vector<bool *> & vacant_flags = minimap_room.vacant_flags;
        bool * room_render_flag;
        bool * room_vacant_render_flag;
        generate_room(get_room_components(x, y, 0.0f, MINIMAP_ROOM_BASE_TEXTURE_PATH), &minimap_room.base_flag);
        generate_room(get_room_components(x, y, -1.0f, MINIMAP_ROOM_TEXTURE_PATH), &room_render_flag);
        generate_room(get_room_components(x, y, -1.0f, MINIMAP_ROOM_VACANT_TEXTURE_PATH), &room_vacant_render_flag);
        occupied_flags.push_back(room_render_flag);
        vacant_flags.push_back(room_vacant_render_flag);


        // Generate room connector for neighbouring rooms that are the same as this room.
        if (get_room(x, y - 1) == room)
        {
            generate_room_connectors(x, y, 0.0f, occupied_flags, vacant_flags);
        }

        if (get_room(x - 1, y) == room)
        {
            generate_room_connectors(x, y, 270.0f, occupied_flags, vacant_flags);
        }

        if (get_room(x, y + 1) == room)
        {
            generate_room_connectors(x, y, 180.0f, occupied_flags, vacant_flags);
        }

        if (get_room(x + 1, y) == room)
        {
            generate_room_connectors(x, y, 90.0f, occupied_flags, vacant_flags);
        }

         minimap_room_groups[room].push_back(minimap_room);
    });


    // Initialize minimap.
    for_each(minimap_room_groups, [](char /*room*/, const vector<Minimap_Room> & minimap_room_group) -> void
    {
        for (const Minimap_Room & minimap_room : minimap_room_group)
        {
            hide(minimap_room.occupied_flags);
        }
    });

    occupy_room('1');
}


void destroy_minimap()
{
    minimap_room_groups.clear();
    game_manager_remove_room_change_handler(MINIMAP_ROOM_CHANGE_HANDLER_ID);
}


} // namespace Game
