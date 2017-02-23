#include "Game/APIs/Minimap.hpp"

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"

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


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const vector<string> MINIMAP_ROOM_SYSTEMS
{
    "ui_transform",
    "sprite_dimensions_handler",
    "renderer",
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<string, Component> get_room_components(int x, int y, const string & texture_path)
{
    static const float ROOM_TEXTURE_OFFSET = 0.25f;

    vec3 position = vec3(x, y, 0.0f) * ROOM_TEXTURE_OFFSET - vec3(2.0f, 2.0f, 0.0f);

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
    auto transform = (Transform *)room_connector_components["transform"];
    transform->rotation = rotation;
    transform->position.z = -1.0f;
    generate_entity(room_connector_components, MINIMAP_ROOM_SYSTEMS);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generate_minimap()
{
    static const string MINIMAP_ROOM_TEXTURE_PATH = "resources/textures/ui/minimap_room.png";

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
