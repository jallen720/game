#include "Game/Systems/In_Game_Controls.hpp"

#include <string>
#include "Nito/Engine.hpp"
#include "Nito/APIs/Input.hpp"


using std::string;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

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
static bool paused;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void toggle_paused()
{
    paused = !paused;
    set_time_scale(paused ? 0.0f : 1.0f);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void in_game_controls_subscribe(const Entity /*entity*/)
{
    paused = false;
    set_key_handler(PAUSE_HANDLER_ID, Keys::ESCAPE, Button_Actions::PRESS, toggle_paused);
    set_controller_button_handler(PAUSE_HANDLER_ID, DS4_Buttons::START, Button_Actions::PRESS, toggle_paused);
}


void in_game_controls_unsubscribe(const Entity /*entity*/)
{
    remove_key_handler(PAUSE_HANDLER_ID);
    remove_controller_button_handler(PAUSE_HANDLER_ID);
}


} // namespace Game
