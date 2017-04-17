// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Engine.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/JSON.hpp"

#include "Game/Components.hpp"
#include "Game/APIs/Enemy_Manager.hpp"
#include "Game/APIs/Audio_Manager.hpp"
#include "Game/Systems/Player_Controller.hpp"
#include "Game/Systems/Projectile.hpp"
#include "Game/Systems/Depth_Handler.hpp"
#include "Game/Systems/Turret.hpp"
#include "Game/Systems/Orientation_Handler.hpp"
#include "Game/Systems/Health_Bar.hpp"
#include "Game/Systems/Health.hpp"
#include "Game/Systems/In_Game_Controls.hpp"
#include "Game/Systems/Menu_Buttons_Handler.hpp"
#include "Game/Systems/Main_Menu.hpp"
#include "Game/Systems/Pause_Menu.hpp"
#include "Game/Systems/Game_Over_Menu.hpp"
#include "Game/Systems/Menu_Controller.hpp"
#include "Game/Systems/Camera_Controller.hpp"
#include "Game/Systems/Game_Manager.hpp"
#include "Game/Systems/Room_Exit_Handler.hpp"
#include "Game/Systems/Floor_Entity.hpp"
#include "Game/Systems/Enemy.hpp"
#include "Game/Systems/Boss.hpp"
#include "Game/Systems/Boss_Segment.hpp"


using std::string;
using std::vector;
using std::map;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// Nito/Engine.hpp
using Nito::add_update_handler;
using Nito::run_engine;
using Nito::get_component_allocator;
using Nito::get_component_deallocator;
using Nito::Update_Handler;
using Nito::Component_Handlers;
using Nito::System_Entity_Handlers;

// Nito/APIs/ECS.hpp
using Nito::set_component_handlers;
using Nito::set_system_entity_handlers;
using Nito::Component;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;
using Cpp_Utils::contains_key;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const vector<Update_Handler> GAME_UPDATE_HANDLERS
{
    player_controller_update,
    projectile_update,
    depth_handler_update,
    turret_update,
    orientation_handler_update,
    health_bar_update,
    menu_buttons_handler_update,
    camera_controller_update,
    boss_update,
    boss_segment_update,
};


static const map<string, const System_Entity_Handlers> GAME_SYSTEM_ENTITY_HANDLERS
{
    NITO_SYSTEM_ENTITY_HANDLERS(player_controller),
    NITO_SYSTEM_ENTITY_HANDLERS(projectile),
    NITO_SYSTEM_ENTITY_HANDLERS(depth_handler),
    NITO_SYSTEM_ENTITY_HANDLERS(turret),
    NITO_SYSTEM_ENTITY_HANDLERS(orientation_handler),
    NITO_SYSTEM_ENTITY_HANDLERS(health_bar),
    NITO_SYSTEM_ENTITY_HANDLERS(health),
    NITO_SYSTEM_ENTITY_HANDLERS(in_game_controls),
    NITO_SYSTEM_ENTITY_HANDLERS(menu_buttons_handler),
    NITO_SYSTEM_ENTITY_HANDLERS(main_menu),
    NITO_SYSTEM_ENTITY_HANDLERS(pause_menu),
    NITO_SYSTEM_ENTITY_HANDLERS(game_over_menu),
    NITO_SYSTEM_ENTITY_HANDLERS(menu_controller),
    NITO_SYSTEM_ENTITY_HANDLERS(camera_controller),
    NITO_SYSTEM_ENTITY_HANDLERS(game_manager),
    NITO_SYSTEM_ENTITY_HANDLERS(room_exit_handler),
    NITO_SYSTEM_ENTITY_HANDLERS(floor_entity),
    NITO_SYSTEM_ENTITY_HANDLERS(enemy),
    NITO_SYSTEM_ENTITY_HANDLERS(boss),
    NITO_SYSTEM_ENTITY_HANDLERS(boss_segment),
};


static const map<string, const Component_Handlers> GAME_COMPONENT_HANDLERS
{
    {
        "player_controller",
        {
            [](const JSON & data) -> Component
            {
                static const map<string, const Player_Controller::Modes> MODES
                {
                    { "controller"     , Player_Controller::Modes::CONTROLLER     },
                    { "keyboard_mouse" , Player_Controller::Modes::KEYBOARD_MOUSE },
                };

                return new Player_Controller
                {
                    data["speed"],
                    data["stick_dead_zone"],
                    MODES.at(data["mode"]),
                };
            },
            get_component_deallocator<Player_Controller>(),
        }
    },
    {
        "projectile",
        {
            [](const JSON & data) -> Component
            {
                auto projectile = new Projectile;
                projectile->speed = contains_key(data, "speed") ? data["speed"].get<float>() : 1.0f;
                projectile->duration = contains_key(data, "duration") ? data["duration"].get<float>() : 1.0f;
                projectile->damage = contains_key(data, "damage") ? data["damage"].get<float>() : 10.0f;
                vec3 & direction = projectile->direction;

                if (contains_key(data, "direction"))
                {
                    const JSON & direction_data = data["direction"];
                    direction.x = direction_data["x"];
                    direction.y = direction_data["y"];
                }

                if (contains_key(data, "target_layers"))
                {
                    projectile->target_layers = data["target_layers"].get<vector<string>>();
                }

                if (contains_key(data, "ignore_layers"))
                {
                    projectile->ignore_layers = data["ignore_layers"].get<vector<string>>();
                }

                return projectile;
            },
            get_component_deallocator<Projectile>(),
        }
    },
    {
        "orientation_handler",
        {
            [](const JSON & data) -> Component
            {
                const JSON & orientation_texture_paths = data["texture_paths"];

                return new Orientation_Handler
                {
                    Orientation::DOWN,
                    vec3(0.0f, -1.0f, 0.0f),
                    {
                        { Orientation::UP    , orientation_texture_paths["up"].get<string>()    },
                        { Orientation::DOWN  , orientation_texture_paths["down"].get<string>()  },
                        { Orientation::LEFT  , orientation_texture_paths["left"].get<string>()  },
                        { Orientation::RIGHT , orientation_texture_paths["right"].get<string>() },
                    },
                };
            },
            get_component_deallocator<Orientation_Handler>(),
        }
    },
    {
        "health",
        {
            [](const JSON & data) -> Component
            {
                return new Health
                {
                    data,
                    data,
                    {},
                };
            },
            get_component_deallocator<Health>(),
        }
    },
    {
        "layers",
        {
            get_component_allocator<vector<string>>(),
            get_component_deallocator<vector<string>>(),
        }
    },
    {
        "target_id",
        {
            get_component_allocator<string>(),
            get_component_deallocator<string>(),
        }
    },
    {
        "room_exit",
        {
            [](const JSON & data) -> Component
            {
                static const map<string, Room_Exit::Types> ROOM_EXIT_TYPES
                {
                    { "door"       , Room_Exit::Types::DOOR       },
                    { "next_floor" , Room_Exit::Types::NEXT_FLOOR },
                };

                return new Room_Exit
                {
                    ROOM_EXIT_TYPES.at(data["type"]),
                    false,
                    data["locked_texture_path"],
                };
            },
            get_component_deallocator<Room_Exit>(),
        }
    },
    {
        "menu_buttons_handler",
        {
            [](const JSON & data) -> Component
            {
                auto menu_buttons_handler = new Menu_Buttons_Handler;
                vector<string> & button_names = menu_buttons_handler->button_names;

                for (const string & button_name : data["button_names"])
                {
                    button_names.push_back(button_name);
                }

                return menu_buttons_handler;
            },
            get_component_deallocator<Menu_Buttons_Handler>(),
        }
    },
    {
        "destination",
        {
            [](const JSON & /*data*/) -> Component
            {
                return new vec2;
            },
            get_component_deallocator<vec2>(),
        }
    },
    {
        "enemy_enabled",
        {
            get_component_allocator<bool>(),
            get_component_deallocator<bool>(),
        }
    },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int run(char ** argv)
{
    for_each(GAME_UPDATE_HANDLERS, add_update_handler);

    for_each(
        GAME_SYSTEM_ENTITY_HANDLERS,
        [](const string & name, const System_Entity_Handlers & system_entity_handlers) -> void
        {
            set_system_entity_handlers(name, system_entity_handlers.subscriber, system_entity_handlers.unsubscriber);
        });

    for_each(GAME_COMPONENT_HANDLERS, [](const string & type, const Component_Handlers & component_handlers) -> void
    {
        set_component_handlers(type, component_handlers.allocator, component_handlers.deallocator);
    });

    enemy_manager_api_init();
    audio_manager_api_init();
    return run_engine(argv);
}


} // namespace Game


int main(int /*argc*/, char ** argv)
{
    return Game::run(argv);
}
