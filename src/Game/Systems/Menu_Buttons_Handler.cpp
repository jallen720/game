#include "Game/Systems/Menu_Buttons_Handler.hpp"

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"
#include "Cpp_Utils/Collection.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Game/Components.hpp"


using std::map;
using std::string;
using std::vector;
using std::function;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;

// Nito/Components.hpp
using Nito::Button;
using Nito::Sprite;
using Nito::Transform;
using Nito::Dimensions;
using Nito::UI_Mouse_Event_Handlers;
using Nito::Text;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;
using Nito::generate_entity;

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

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains_key;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string SELECT_HANDLER_ID = "menu_buttons_handler select";
static const string BACK_HANDLER_ID = "menu_buttons_handler back";
static const Menu_Buttons_Handler * entity_menu_buttons_handler;
static string * selection_sprite_parent_id;
static map<string, Button *> buttons;
static const Button * selected_button;
static int selected_button_index;
static int button_count;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool entity_subscribed()
{
    return entity_menu_buttons_handler != nullptr;
}


static void select_menu_button(int index)
{
    const string & button_id = entity_menu_buttons_handler->button_ids[index];
    selected_button_index = index;
    selected_button = buttons.at(button_id);
    *selection_sprite_parent_id = button_id;
}


static Button * generate_button(const string & button_id, int index, int button_count)
{
    static const vector<string> BUTTON_SYSTEMS
    {
        "button",
        "renderer",
        "sprite_dimensions_handler",
        "ui_mouse_event_dispatcher",
        "local_transform"
    };

    static const vector<string> BUTTON_TEXT_COMPONENTS
    {
        "text_renderer",
        "local_transform"
    };

    Button * button = new Button
    {
        "resources/textures/ui/button_hover.png",
        "resources/textures/ui/button_pressed.png",
        {},
    };

    float button_y = ((button_count - 1) / 2.0f) - (index * 1.2f);

    generate_entity(
        {
            { "id"                      , new string(button_id)                                          },
            { "parent_id"               , new string("menu")                                             },
            { "render_layer"            , new string("ui")                                               },
            { "button"                  , button                                                         },
            { "sprite"                  , new Sprite { "resources/textures/ui/button.png", "texture" }   },
            { "transform"               , new Transform { vec3(), vec3(), 0.0f }                         },
            { "dimensions"              , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) }          },
            { "local_transform"         , new Transform { vec3(0.0f, button_y, 0.0f), vec3(1.0f), 0.0f } },
            { "ui_mouse_event_handlers" , new UI_Mouse_Event_Handlers                                    },
        },
        BUTTON_SYSTEMS);

    generate_entity(
        {
            { "parent_id"       , new string(button_id)                                              },
            { "render_layer"    , new string("ui")                                                   },
            { "text"            , new Text { "resources/fonts/Ubuntu-L.ttf", vec3(0.0f), button_id } },
            { "transform"       , new Transform { vec3(), vec3(), 0.0f }                             },
            { "dimensions"      , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.45f, 0.0f) }             },
            { "local_transform" , new Transform { vec3(0.0f, 0.0f, -1.0f), vec3(1.0f), 0.0f }        },
        },
        BUTTON_TEXT_COMPONENTS);

    return button;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void menu_buttons_handler_subscribe(Entity entity)
{
    if (entity_subscribed())
    {
        throw runtime_error("ERROR: only one entity can be subscribed to the menu_buttons_handler system per scene!");
    }

    entity_menu_buttons_handler = (Menu_Buttons_Handler *)get_component(entity, "menu_buttons_handler");
    selection_sprite_parent_id = (string *)get_component(get_entity("selection_sprite"), "parent_id");
    const function<void()> & back_handler = entity_menu_buttons_handler->back_handler;
    const vector<string> & button_ids = entity_menu_buttons_handler->button_ids;
    button_count = button_ids.size();


    // Load buttons.
    for (int i = 0; i < button_count; i++)
    {
        const string & button_id = button_ids[i];


        // Set button up to call its associated handler in entity's Menu_Buttons_Handler component.
        auto button = generate_button(button_id, i, button_count);
        buttons[button_id] = button;

        button->click_handler = [&]() -> void
        {
            entity_menu_buttons_handler->button_handlers.at(button_id)();
        };
    }


    // Set input handlers.
    set_controller_button_handler(SELECT_HANDLER_ID, DS4_Buttons::X, Button_Actions::PRESS, [&]() -> void
    {
        selected_button->click_handler();
    });

    set_controller_button_handler(BACK_HANDLER_ID, DS4_Buttons::CIRCLE, Button_Actions::PRESS, back_handler);
    set_key_handler(BACK_HANDLER_ID, Keys::ESCAPE, Button_Actions::PRESS, back_handler);


    // By default, select the first button provided.
    select_menu_button(0);
}


void menu_buttons_handler_unsubscribe(Entity /*entity*/)
{
    static const auto DUD = []() -> void {};

    entity_menu_buttons_handler = nullptr;
    selection_sprite_parent_id = nullptr;
    selected_button = nullptr;


    // Cleanup buttons.
    for_each(buttons, [&](const string & /*button_id*/, Button * button) -> void
    {
        button->click_handler = DUD;
    });

    buttons.clear();


    // Remove input handlers.
    remove_controller_button_handler(SELECT_HANDLER_ID);
    remove_controller_button_handler(BACK_HANDLER_ID);
    remove_key_handler(BACK_HANDLER_ID);
}


void menu_buttons_handler_update()
{
    static bool should_read_input = true;

    if (!entity_subscribed())
    {
        return;
    }

    const float d_pad_y = get_controller_axis(DS4_Axes::D_PAD_Y);


    // When user is no longer holding d-pad down, allow for next input to be read.
    if (!should_read_input && d_pad_y == 0.0f)
    {
        should_read_input = true;
    }


    // Ignore checking for user input if user still hasn't released d-pad from last input.
    if (!should_read_input)
    {
        return;
    }


    // Get new selection if user is using the d-pad.
    int new_selected_button_index = selected_button_index;

    if (d_pad_y > 0.0f)
    {
        new_selected_button_index++;
    }
    else if (d_pad_y < 0.0f)
    {
        new_selected_button_index--;
    }


    // Ensure the new selected index is different than the current selected index, and is within range of the number of
    // buttons in the menu before switching the selected button.
    if (new_selected_button_index != selected_button_index &&
        new_selected_button_index >= 0 &&
        new_selected_button_index < button_count)
    {
        select_menu_button(new_selected_button_index);


        // Now that user input has been read to change the menu selection, wait until user releases d-pad to read next
        // input, preventing changing selection once per frame for every frame the d-pad is held down.
        should_read_input = false;
    }
}


} // namespace Game
