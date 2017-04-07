#include "Game/Systems/Main_Menu.hpp"

#include <map>
#include <string>
#include <functional>
#include <stdexcept>
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Input.hpp"

#include "Game/Components.hpp"


using std::map;
using std::string;
using std::function;
using std::runtime_error;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/APIs/Window.hpp
using Nito::close_window;

// Nito/APIs/Scene.hpp
using Nito::set_scene_to_load;

// Nito/APIs/Input.hpp
using Nito::Keys;
using Nito::DS4_Axes;
using Nito::DS4_Buttons;
using Nito::Button_Actions;
using Nito::get_controller_axis;
using Nito::set_controller_button_handler;
using Nito::remove_controller_button_handler;
using Nito::set_key_handler;
using Nito::remove_key_handler;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string EXIT_HANDLER_ID = "main_menu exit";
static Menu_Buttons_Handler * entity_menu_buttons_handler;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main_menu_subscribe(Entity entity)
{
    if (entity_menu_buttons_handler != nullptr)
    {
        throw runtime_error("ERROR: only one entity is allowed to be subscribed to the main_menu system per scene!");
    }

    entity_menu_buttons_handler = (Menu_Buttons_Handler *)get_component(entity, "menu_buttons_handler");
    map<string, function<void()>> & button_handlers = entity_menu_buttons_handler->button_handlers;

    button_handlers["PLAY"] = []() -> void
    {
        set_scene_to_load("game");
    };

    button_handlers["EXIT"] = close_window;
    set_key_handler(EXIT_HANDLER_ID, Keys::ESCAPE, Button_Actions::PRESS, close_window);
    set_controller_button_handler(EXIT_HANDLER_ID, DS4_Buttons::CIRCLE, Button_Actions::PRESS, close_window);
}


void main_menu_unsubscribe(Entity /*entity*/)
{
    static const auto DUD = []() -> void {};

    map<string, function<void()>> & button_handlers = entity_menu_buttons_handler->button_handlers;
    button_handlers["PLAY"] = DUD;
    button_handlers["EXIT"] = DUD;
    entity_menu_buttons_handler = nullptr;
    remove_key_handler(EXIT_HANDLER_ID);
    remove_controller_button_handler(EXIT_HANDLER_ID);
}


} // namespace Game
