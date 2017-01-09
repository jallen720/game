#include "Game/Systems/Main_Menu.hpp"

#include <map>
#include <string>
#include <functional>
#include <stdexcept>
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Scene.hpp"

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


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

    button_handlers["Play"] = []() -> void
    {
        set_scene_to_load("game");
    };

    button_handlers["Quit"] = close_window;
    entity_menu_buttons_handler->back_handler = close_window;
}


void main_menu_unsubscribe(Entity /*entity*/)
{
    static const auto DUD = []() -> void {};

    map<string, function<void()>> & button_handlers = entity_menu_buttons_handler->button_handlers;
    button_handlers["Play"] = DUD;
    button_handlers["Quit"] = DUD;
    entity_menu_buttons_handler->back_handler = DUD;
    entity_menu_buttons_handler = nullptr;
}


} // namespace Game
