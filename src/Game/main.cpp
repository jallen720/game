// Required before any other OpenGL includes
#include <GL/glew.h>

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Engine.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/JSON.hpp"

#include "Game/Components.hpp"
#include "Game/Systems/Play_Button.hpp"
#include "Game/Systems/Quit_Button.hpp"
#include "Game/Systems/Room_Generator.hpp"
#include "Game/Systems/Player_Controller.hpp"
#include "Game/Systems/Projectile.hpp"
#include "Game/Systems/Depth_Handler.hpp"
#include "Game/Systems/Turret.hpp"
#include "Game/Systems/Orientation_Handler.hpp"
#include "Game/Systems/Health_Bar.hpp"
#include "Game/Systems/Collider.hpp"
#include "Game/Systems/Health.hpp"
#include "Game/Systems/Main_Menu_Controls.hpp"


using std::string;
using std::vector;
using std::map;

// glm/glm.hpp
using glm::vec3;

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

// Nito/APIs/Input.hpp
using Nito::Keys;
using Nito::Button_Actions;
using Nito::set_key_handler;

// Nito/APIs/Window.hpp
using Nito::close_window;

// Cpp_Utils/Container.hpp
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
static vector<Update_Handler> game_update_handlers
{
    player_controller_update,
    projectile_update,
    depth_handler_update,
    turret_update,
    orientation_handler_update,
    health_bar_update,
    collider_update,
    main_menu_controls_update,
};


static map<string, const System_Entity_Handlers> game_system_entity_handlers
{
    NITO_SYSTEM_ENTITY_HANDLERS(play_button),
    NITO_SYSTEM_ENTITY_HANDLERS(quit_button),
    NITO_SYSTEM_ENTITY_HANDLERS(room_generator),
    NITO_SYSTEM_ENTITY_HANDLERS(player_controller),
    NITO_SYSTEM_ENTITY_HANDLERS(projectile),
    NITO_SYSTEM_ENTITY_HANDLERS(depth_handler),
    NITO_SYSTEM_ENTITY_HANDLERS(turret),
    NITO_SYSTEM_ENTITY_HANDLERS(orientation_handler),
    NITO_SYSTEM_ENTITY_HANDLERS(health_bar),
    NITO_SYSTEM_ENTITY_HANDLERS(collider),
    NITO_SYSTEM_ENTITY_HANDLERS(health),
    NITO_SYSTEM_ENTITY_HANDLERS(main_menu_controls),
};


static map<string, const Component_Handlers> game_component_handlers
{
    {
        "player_controller",
        {
            [](const JSON & data) -> Component
            {
                return new Player_Controller
                {
                    data["speed"],
                    data["stick_dead_zone"],
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
                vec3 direction;

                if (contains_key(data, "direction"))
                {
                    const JSON & direction_data = data["direction"];
                    direction.x = direction_data["x"];
                    direction.y = direction_data["y"];
                }

                return new Projectile
                {
                    data["speed"],
                    direction,
                    data["duration"],
                    data["target_layers"],
                };
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
                };
            },
            get_component_deallocator<Health>(),
        }
    },
    {
        "collider",
        {
            [](const JSON & data) -> Component
            {
                return new Collider
                {
                    data["render"],
                    data["radius"],
                    {},
                };
            },
            get_component_deallocator<Collider>(),
        }
    },
    {
        "layer",
        {
            get_component_allocator<string>(),
            get_component_deallocator<string>(),
        }
    },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int run()
{
    for_each(game_update_handlers, add_update_handler);

    for_each(
        game_system_entity_handlers,
        [](const string & name, const System_Entity_Handlers & system_entity_handlers) -> void
        {
            set_system_entity_handlers(name, system_entity_handlers.subscriber, system_entity_handlers.unsubscriber);
        });

    for_each(game_component_handlers, [](const string & type, const Component_Handlers & component_handlers) -> void
    {
        set_component_handlers(type, component_handlers.allocator, component_handlers.deallocator);
    });

    set_key_handler("exit", Keys::ESCAPE, Button_Actions::PRESS, close_window);
    return run_engine();
}


} // namespace Game


int main()
{
    return Game::run();
}
