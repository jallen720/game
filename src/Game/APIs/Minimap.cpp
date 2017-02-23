#include "Game/APIs/Minimap.hpp"

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Nito/APIs/Resources.hpp"

#include "Game/APIs/Floor_Generator.hpp"


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


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string MINIMAP_ROOM_TEXTURE_PATH = "resources/textures/ui/minimap_room.png";

static const vector<string> MINIMAP_ROOM_SYSTEMS
{
    "ui_transform",
    "sprite_dimensions_handler",
    "renderer",
};

static vec3 room_texture_offset;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, Component> get_room_components(int x, int y, const string & texture_path)
{
    static const int ROOM_TEXTURE_SIZE = 16;

    vec3 position = (vec3(x, y, 0.0f) * room_texture_offset) - vec3(2.0f, 2.0f, 0.0f);

    return
    {
        { "render_layer" , new string("ui")                                      },
        { "transform"    , new Transform { vec3(0.0f), vec3(1.0f), 0.0f }        },
        { "ui_transform" , new UI_Transform { position, vec3(1.0f) }             },
        { "dimensions"   , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) } },
        { "sprite"       , new Sprite { texture_path, "texture" }                },
    };
}


static void generate_room_connector(int x, int y, float rotation)
{
    static const string MINIMAP_ROOM_CONNECTOR_TEXTURE_PATH = "resources/textures/ui/minimap_room_connector.png";

    map<string, Component> room_connector_components = get_room_components(x, y, MINIMAP_ROOM_CONNECTOR_TEXTURE_PATH);
    ((Transform *)room_connector_components["transform"])->rotation = rotation;
    ((UI_Transform *)room_connector_components["ui_transform"])->position.z = -1.0f;
    generate_entity(room_connector_components, MINIMAP_ROOM_SYSTEMS);
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
}


void generate_minimap()
{
    iterate_current_floor_rooms([](int x, int y, const char & room) -> void
    {
        // Don't generate minimap room for empty rooms.
        if (room == '0')
        {
            return;
        }


        // Generate minimap room for current room.
        generate_entity(get_room_components(x, y, MINIMAP_ROOM_TEXTURE_PATH), MINIMAP_ROOM_SYSTEMS);


        // Generate room connector for neighbouring rooms that are the same as this room.
        if (get_room(x, y - 1) == room)
        {
            generate_room_connector(x, y, 0.0f);
        }

        if (get_room(x - 1, y) == room)
        {
            generate_room_connector(x, y, 270.0f);
        }

        if (get_room(x, y + 1) == room)
        {
            generate_room_connector(x, y, 180.0f);
        }

        if (get_room(x + 1, y) == room)
        {
            generate_room_connector(x, y, 90.0f);
        }
    });
}


} // namespace Game
