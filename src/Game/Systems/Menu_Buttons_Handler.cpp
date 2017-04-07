#include "Game/Systems/Menu_Buttons_Handler.hpp"

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Scene.hpp"
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

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

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
static string get_button_id(Entity entity, const string & button_name)
{
    return button_name + " " + to_string(entity);
}


static Button * generate_button(
    Entity entity,
    const string & button_name,
    const string & menu_id,
    int index,
    int button_count)
{
    static const vector<string> BUTTON_TEXT_SYSTEMS
    {
        "text_renderer",
        "local_transform"
    };

    string button_id = get_button_id(entity, button_name);

    generate_entity(
        {
            { "parent_id"       , new string(button_id)                                                    },
            { "render_layer"    , new string("ui")                                                         },
            { "text"            , new Text { "resources/fonts/UbuntuMono-R.ttf", vec3(0.0f), button_name } },
            { "transform"       , new Transform { vec3(), vec3(), 0.0f }                                   },
            { "dimensions"      , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.45f, 0.0f) }                   },
            { "local_transform" , new Transform { vec3(0.0f, 0.0f, -1.0f), vec3(1.0f), 0.0f }              },
        },
        BUTTON_TEXT_SYSTEMS);

    Entity button = load_blueprint("button");
    *(string *)get_component(button, "id") = button_id;
    *(string *)get_component(button, "parent_id") = menu_id;
    ((Transform *)get_component(button, "local_transform"))->position.y = ((button_count - 1) / 2.0f) - (index * 1.2f);
    return (Button *)get_component(button, "button");
}


string * generate_selection_sprite(Entity entity)
{
    Entity selection_sprite = load_blueprint("selection_sprite");
    *((string *)get_component(selection_sprite, "id")) = "selection_sprite " + to_string(entity);
    return (string *)get_component(selection_sprite, "parent_id");
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
    const vector<string> & button_names = menu_buttons_handler->button_names;
    const int button_count = button_names.size();
    Menu_Buttons_Handler_State & entity_state = entity_states[entity];
    entity_state.menu_buttons_handler = menu_buttons_handler;
    entity_state.selection_sprite_parent_id = generate_selection_sprite(entity);
    entity_state.select_handler_id = select_handler_id;
    entity_state.button_count = button_count;
    entity_state.dpad_reset = true;
    map<string, Button *> & buttons = entity_state.buttons;
    const map<string, function<void()>> & button_handlers = menu_buttons_handler->button_handlers;
    const auto menu_id = (string *)get_component(entity, "id");


    // Load buttons.
    for (int i = 0; i < button_count; i++)
    {
        const string & button_name = button_names[i];


        // Set button up to call its associated handler in entity's Menu_Buttons_Handler component.
        auto button = generate_button(entity, button_name, *menu_id, i, button_count);
        buttons[button_name] = button;

        button->click_handler = [&]() -> void
        {
            if (!contains_key(button_handlers, button_name))
            {
                throw runtime_error("ERROR: no handler set for the \"" + button_name + "\" button!");
            }

            button_handlers.at(button_name)();
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
        const int button_count = entity_state.button_count;


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


        // Ensure the new selected index is different than the current selected index.
        if (new_selected_button_index != selected_button_index)
        {
            // Wrap new index within the range of buttons.
            if (new_selected_button_index >= button_count)
            {
                new_selected_button_index = 0;
            }
            else if (new_selected_button_index < 0)
            {
                new_selected_button_index = button_count - 1;
            }


            menu_buttons_handler_select_button(entity, new_selected_button_index);


            // Now that user input has been read to change the menu selection, wait until user releases d-pad to read
            // next input, preventing changing selection once per frame for every frame the d-pad is held down.
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

    const string & button_name = entity_state.menu_buttons_handler->button_names[index];
    entity_state.selected_button_index = index;
    entity_state.selected_button = entity_state.buttons.at(button_name);
    *entity_state.selection_sprite_parent_id = get_button_id(entity, button_name);
}


} // namespace Game
