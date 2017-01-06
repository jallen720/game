#include "Game/Systems/Main_Menu_Controller.hpp"

#include <string>
#include "Nito/APIs/Input.hpp"


using std::string;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/APIs/Input.hpp
using Nito::Controller_Axes;
using Nito::get_controller_axis;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string PLAY_BUTTON_ID = "Play_Button";
static const string QUIT_BUTTON_ID = "Quit_Button";
static string * selection_sprite_parent_id;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main_menu_controller_subscribe(const Entity /*entity*/)
{
    selection_sprite_parent_id = (string *)get_component(get_entity("selection_sprite"), "parent_id");
    *selection_sprite_parent_id = PLAY_BUTTON_ID;
}


void main_menu_controller_unsubscribe(const Entity /*entity*/)
{

}


void main_menu_controller_update()
{
    const float d_pad_y = get_controller_axis(Controller_Axes::D_PAD_Y);

    if (d_pad_y > 0.0f)
    {
        *selection_sprite_parent_id = QUIT_BUTTON_ID;
    }
    else if (d_pad_y < 0.0f)
    {
        *selection_sprite_parent_id = PLAY_BUTTON_ID;
    }
}


} // namespace Game
