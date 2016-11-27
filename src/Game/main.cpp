#include <string>
#include <vector>
#include <map>
#include "Nito/Engine.hpp"
#include "Nito/APIs/ECS.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/JSON.hpp"

#include "Game/Systems/Play_Button.hpp"
#include "Game/Systems/Room_Generator.hpp"
#include "Game/Systems/Player_Controller.hpp"


using std::string;
using std::vector;
using std::map;

// Nito/Engine.hpp
using Nito::add_update_handler;
using Nito::set_control_handler;
using Nito::run_engine;
using Nito::get_component_allocator;
using Nito::get_component_deallocator;
using Nito::Update_Handler;
using Nito::Component_Handlers;
using Nito::System_Entity_Handlers;

// Nito/APIs/ECS.hpp
using Nito::set_component_handlers;
using Nito::set_system_entity_handlers;

// Nito/APIs/Input.hpp
using Nito::Control_Handler;

// Nito/APIs/Window.hpp
using Nito::close_window;

// Cpp_Utils/Container.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/JSON.hpp
using Cpp_Utils::JSON;


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
};


static map<string, const Control_Handler> game_control_handlers
{
    {
        "exit",
        close_window
    },
};


static map<string, const System_Entity_Handlers> game_system_entity_handlers
{
    {
        "play_button",
        {
            play_button_subscribe,
            play_button_unsubscribe,
        }
    },
    {
        "room_generator",
        {
            room_generator_subscribe,
            room_generator_unsubscribe,
        }
    },
    {
        "player_controller",
        {
            player_controller_subscribe,
            player_controller_unsubscribe,
        }
    },
};


static map<string, const Component_Handlers> game_component_handlers
{
    {
        "speed",
        {
            get_component_allocator<float>(),
            get_component_deallocator<float>(),
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
    for_each(game_control_handlers, set_control_handler);

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

    return run_engine();
}


} // namespace Game


int main()
{
    return Game::run();
}
