#include "Game/Systems/Main_Menu_Controller.hpp"

#include <map>
#include <string>
#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"
#include "Cpp_Utils/Collection.hpp"


using std::map;
using std::string;

// Nito/Components.hpp
using Nito::Button;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/APIs/Input.hpp
using Nito::Controller_Axes;
using Nito::Button_Actions;
using Nito::get_controller_axis;
using Nito::set_controller_button_handler;
using Nito::remove_controller_button_handler;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class Selections
{
    DEFAULT_BUTTON,
    QUIT_BUTTON,
};


struct Menu_Button
{
    const string id;
    const Button * button;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Selections, Menu_Button> menu_buttons
{
    { Selections::DEFAULT_BUTTON , { "Play_Button", nullptr } },
    { Selections::QUIT_BUTTON    , { "Quit_Button", nullptr } },
};

static const string CONTROLLER_BUTTON_HANDLER_ID = "main_menu_controller select";
static string * selection_sprite_parent_id;
static const Menu_Button * selected_menu_button;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void select_menu_button(const Selections selection)
{
    selected_menu_button = &menu_buttons.at(selection);
    *selection_sprite_parent_id = selected_menu_button->id;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main_menu_controller_subscribe(const Entity /*entity*/)
{
    selection_sprite_parent_id = (string *)get_component(get_entity("selection_sprite"), "parent_id");
    select_menu_button(Selections::DEFAULT_BUTTON);


    // Get menu buttons' Button components.
    for_each(menu_buttons, [](const Selections /*selection*/, Menu_Button & menu_button) -> void
    {
        menu_button.button = (Button *)get_component(get_entity(menu_button.id), "button");
    });


    // Set selection handler to call the selected button's click_handler().
    set_controller_button_handler(
        CONTROLLER_BUTTON_HANDLER_ID,
        1,
        Button_Actions::PRESS,
        [=]() -> void
        {
            selected_menu_button->button->click_handler();
        });
}


void main_menu_controller_unsubscribe(const Entity /*entity*/)
{
    remove_controller_button_handler(CONTROLLER_BUTTON_HANDLER_ID);
}


void main_menu_controller_update()
{
    const float d_pad_y = get_controller_axis(Controller_Axes::D_PAD_Y);

    if (d_pad_y > 0.0f)
    {
        select_menu_button(Selections::QUIT_BUTTON);
    }
    else if (d_pad_y < 0.0f)
    {
        select_menu_button(Selections::DEFAULT_BUTTON);
    }
}


} // namespace Game
