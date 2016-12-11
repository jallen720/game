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
#include "Game/Systems/Room_Generator.hpp"
#include "Game/Systems/Player_Controller.hpp"
#include "Game/Systems/Projectile.hpp"


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
};


static map<string, const System_Entity_Handlers> game_system_entity_handlers
{
    NITO_SYSTEM_ENTITY_HANDLERS(play_button),
    NITO_SYSTEM_ENTITY_HANDLERS(room_generator),
    NITO_SYSTEM_ENTITY_HANDLERS(player_controller),
    NITO_SYSTEM_ENTITY_HANDLERS(projectile),
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
                };
            },
            get_component_deallocator<Projectile>(),
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
