#include "Game/Systems/In_Game_Controls.hpp"

#include <string>
#include "Nito/Engine.hpp"
#include "Nito/APIs/Input.hpp"

#include "Game/Components.hpp"
#include "Game/Systems/Pause_Menu.hpp"
#include "Game/Systems/Game_Over_Menu.hpp"


using std::string;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/APIs/Input.hpp
using Nito::set_key_handler;
using Nito::remove_key_handler;
using Nito::set_controller_button_handler;
using Nito::remove_controller_button_handler;
using Nito::Keys;
using Nito::DS4_Buttons;
using Nito::Button_Actions;

// Nito/Engine.hpp
using Nito::set_time_scale;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string PAUSE_HANDLER_ID = "in_game_controls pause";
static bool entity_paused;
static bool entity_game_over;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void set_paused(bool paused)
{
    entity_paused = paused;
    set_time_scale(entity_paused ? 0.0f : 1.0f);
}


static void toggle_paused()
{
    // If player gameover'd, don't toggle pause.
    if (!entity_game_over)
    {
        set_paused(!entity_paused);
        pause_menu_set_on(entity_paused);
    }
}


static void game_over()
{
    entity_game_over = true;
    set_paused(true);
    game_over_menu_set_on(true);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void in_game_controls_subscribe(Entity /*entity*/)
{
    entity_paused = false;
    entity_game_over = false;
    set_key_handler(PAUSE_HANDLER_ID, Keys::ESCAPE, Button_Actions::PRESS, toggle_paused);
    set_controller_button_handler(PAUSE_HANDLER_ID, DS4_Buttons::START, Button_Actions::PRESS, toggle_paused);
    ((Health *)get_component(get_entity("player"), "health"))->death_handler = game_over;
}


void in_game_controls_unsubscribe(Entity /*entity*/)
{
    in_game_controls_unpause();
    remove_key_handler(PAUSE_HANDLER_ID);
    remove_controller_button_handler(PAUSE_HANDLER_ID);
}


void in_game_controls_unpause()
{
    set_paused(false);
}


} // namespace Game
