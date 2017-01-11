#include "Game/Systems/Game_Over_Menu.hpp"

#include <map>
#include <string>
#include <functional>
#include <stdexcept>
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Input.hpp"

#include "Game/Components.hpp"
#include "Game/Systems/Menu_Controller.hpp"


using std::map;
using std::string;
using std::function;
using std::runtime_error;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::set_scene_to_load;

// Nito/APIs/Input.hpp
using Nito::Keys;
using Nito::DS4_Buttons;
using Nito::Button_Actions;
using Nito::set_key_handler;
using Nito::remove_key_handler;
using Nito::set_controller_button_handler;
using Nito::remove_controller_button_handler;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string QUIT_HANDLER_ID = "game_over_menu quit";
static Menu_Buttons_Handler * entity_menu_buttons_handler;
static Entity entity;
static bool entity_on;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void quit_handler()
{
    if (entity_on)
    {
        set_scene_to_load("default");
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void game_over_menu_subscribe(Entity _entity)
{
    entity = _entity;

    if (entity_menu_buttons_handler != nullptr)
    {
        throw runtime_error("ERROR: only one entity is allowed to subscribed to the game_over_menu system per scene!");
    }

    entity_menu_buttons_handler = (Menu_Buttons_Handler *)get_component(entity, "menu_buttons_handler");
    map<string, function<void()>> & button_handlers = entity_menu_buttons_handler->button_handlers;

    button_handlers["Restart"] = []() -> void
    {
        if (entity_on)
        {
            set_scene_to_load("game");
        }
    };

    button_handlers["Quit"] = quit_handler;
    set_key_handler(QUIT_HANDLER_ID, Keys::ESCAPE, Button_Actions::PRESS, quit_handler);
    set_controller_button_handler(QUIT_HANDLER_ID, DS4_Buttons::CIRCLE, Button_Actions::PRESS, quit_handler);
    game_over_menu_set_on(false);
}


void game_over_menu_unsubscribe(Entity /*entity*/)
{
    static const auto DUD = []() -> void {};

    map<string, function<void()>> & button_handlers = entity_menu_buttons_handler->button_handlers;
    button_handlers["Restart"] = DUD;
    button_handlers["Quit"] = DUD;
    entity_menu_buttons_handler = nullptr;
    remove_key_handler(QUIT_HANDLER_ID);
    remove_controller_button_handler(QUIT_HANDLER_ID);
}


void game_over_menu_set_on(bool on)
{
    menu_controller_set_on(entity, entity_on = on);
}


} // namespace Game
