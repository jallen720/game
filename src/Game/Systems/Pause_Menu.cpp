#include "Game/Systems/Pause_Menu.hpp"

#include <map>
#include <string>
#include <functional>
#include <stdexcept>
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Input.hpp"

#include "Game/Components.hpp"
#include "Game/Systems/In_Game_Controls.hpp"
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
using Nito::DS4_Buttons;
using Nito::Button_Actions;
using Nito::set_controller_button_handler;
using Nito::remove_controller_button_handler;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string UNPAUSE_HANDLER_ID = "pause_menu unpause";
static Menu_Buttons_Handler * entity_menu_buttons_handler;
static bool entity_on;
static Entity entity;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void unpause()
{
    if (entity_on)
    {
        in_game_controls_unpause();
        pause_menu_set_on(false);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pause_menu_subscribe(Entity _entity)
{
    entity = _entity;

    if (entity_menu_buttons_handler != nullptr)
    {
        throw runtime_error("ERROR: only one entity is allowed to subscribed to the pause_menu system per scene!");
    }

    entity_menu_buttons_handler = (Menu_Buttons_Handler *)get_component(entity, "menu_buttons_handler");
    map<string, function<void()>> & button_handlers = entity_menu_buttons_handler->button_handlers;
    button_handlers["Continue"] = unpause;

    button_handlers["Quit"] = []() -> void
    {
        if (entity_on)
        {
            set_scene_to_load("default");
        }
    };

    set_controller_button_handler(UNPAUSE_HANDLER_ID, DS4_Buttons::CIRCLE, Button_Actions::PRESS, unpause);
    pause_menu_set_on(false);
}


void pause_menu_unsubscribe(Entity /*entity*/)
{
    static const auto DUD = []() -> void {};

    map<string, function<void()>> & button_handlers = entity_menu_buttons_handler->button_handlers;
    button_handlers["Continue"] = DUD;
    button_handlers["Quit"] = DUD;
    entity_menu_buttons_handler = nullptr;
    remove_controller_button_handler(UNPAUSE_HANDLER_ID);
}


void pause_menu_set_on(bool on)
{
    menu_controller_set_on(entity, entity_on = on);
}


} // namespace Game
