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
#include "Cpp_Utils/String.hpp"

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
using Nito::generate_entity;

// Nito/APIs/Input.hpp
using Nito::DS4_Axes;
using Nito::DS4_Buttons;
using Nito::Button_Actions;
using Nito::get_controller_axis;
using Nito::set_controller_button_handler;
using Nito::remove_controller_button_handler;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains_key;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Menu_Buttons_Handler_State
{
    const Menu_Buttons_Handler * menu_buttons_handler;
    string * selection_sprite_parent_id;
    string select_handler_id;
    map<string, Button *> buttons;
    const Button * selected_button;
    int selected_button_index;
    int button_count;
    bool dpad_reset;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Menu_Buttons_Handler_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static Button * generate_button(const string & button_id, const string & menu_id, int index, int button_count)
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
            { "parent_id"               , new string(menu_id)                                            },
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


string * generate_selection_sprite()
{
    static const vector<string> SELECTION_SPRITE_COMPONENTS
    {
        "renderer",
        "sprite_dimensions_handler",
        "local_transform"
    };

    auto parent_id = new string("");

    generate_entity(
        {
            { "id"              , new string("selection_sprite")                                  },
            { "parent_id"       , parent_id                                                       },
            { "render_layer"    , new string("ui")                                                },
            { "sprite"          , new Sprite { "resources/textures/ui/selection.png", "texture" } },
            { "dimensions"      , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) }           },
            { "transform"       , new Transform { vec3(), vec3(1.0f), 0.0f }                      },
            { "local_transform" , new Transform { vec3(0.0f, 0.0f, 1.0f), vec3(1.0f), 0.0f }      },
        },
        SELECTION_SPRITE_COMPONENTS);

    return parent_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void menu_buttons_handler_subscribe(Entity entity)
{
    static const string SELECT_HANDLER_ID_PREFIX = "menu_buttons_handler select ";

    auto menu_buttons_handler = (Menu_Buttons_Handler *)get_component(entity, "menu_buttons_handler");
    const string select_handler_id = SELECT_HANDLER_ID_PREFIX + to_string(entity);
    const vector<string> & button_ids = menu_buttons_handler->button_ids;
    const int button_count = button_ids.size();
    Menu_Buttons_Handler_State & entity_state = entity_states[entity];
    entity_state.menu_buttons_handler = menu_buttons_handler;
    entity_state.selection_sprite_parent_id = generate_selection_sprite();
    entity_state.select_handler_id = select_handler_id;
    entity_state.button_count = button_count;
    entity_state.dpad_reset = true;
    map<string, Button *> & buttons = entity_state.buttons;
    const map<string, function<void()>> & button_handlers = menu_buttons_handler->button_handlers;
    const auto menu_id = (string *)get_component(entity, "id");


    // Load buttons.
    for (int i = 0; i < button_count; i++)
    {
        const string & button_id = button_ids[i];


        // Set button up to call its associated handler in entity's Menu_Buttons_Handler component.
        auto button = generate_button(button_id, *menu_id, i, button_count);
        buttons[button_id] = button;

        button->click_handler = [&]() -> void
        {
            if (!contains_key(button_handlers, button_id))
            {
                throw runtime_error("ERROR: no handler set for the \"" + button_id + "\" button!");
            }

            button_handlers.at(button_id)();
        };
    }


    // Set input handlers.
    set_controller_button_handler(select_handler_id, DS4_Buttons::X, Button_Actions::PRESS, [&]() -> void
    {
        entity_state.selected_button->click_handler();
    });


    // By default, select the first button provided.
    menu_buttons_handler_select_button(entity, 0);
}


void menu_buttons_handler_unsubscribe(Entity entity)
{
    // Remove input handlers.
    remove_controller_button_handler(entity_states.at(entity).select_handler_id);


    remove(entity_states, entity);
}


void menu_buttons_handler_update()
{
    for_each(entity_states, [](Entity entity, Menu_Buttons_Handler_State & entity_state) -> void
    {
        const float d_pad_y = get_controller_axis(DS4_Axes::D_PAD_Y);
        bool & dpad_reset = entity_state.dpad_reset;
        const int selected_button_index = entity_state.selected_button_index;


        // When user is no longer holding d-pad down, allow for next input to be read.
        if (!dpad_reset && d_pad_y == 0.0f)
        {
            dpad_reset = true;
        }


        // Ignore checking for input if user hasn't released d-pad from last input.
        if (!dpad_reset)
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
            new_selected_button_index < entity_state.button_count)
        {
            menu_buttons_handler_select_button(entity, new_selected_button_index);


            // Now that user input has been read to change the menu selection, wait until user releases d-pad to read next
            // input, preventing changing selection once per frame for every frame the d-pad is held down.
            dpad_reset = false;
        }
    });
}


void menu_buttons_handler_select_button(Entity entity, int index)
{
    Menu_Buttons_Handler_State & entity_state = entity_states[entity];
    const int button_count = entity_state.button_count;

    if (index >= button_count)
    {
        throw runtime_error(
            "ERROR: cannot select button at index " + to_string(index) + "; menu only has " + to_string(button_count) +
            " buttons!");
    }

    const string & button_id = entity_state.menu_buttons_handler->button_ids[index];
    entity_state.selected_button_index = index;
    entity_state.selected_button = entity_state.buttons.at(button_id);
    *entity_state.selection_sprite_parent_id = button_id;
}


} // namespace Game
