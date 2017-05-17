#include "Game/Systems/Menu_Buttons_Handler.hpp"

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Scene.hpp"
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
using Nito::Transform;
using Nito::Dimensions;
using Nito::UI_Mouse_Event_Handlers;
using Nito::Text;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::generate_entity;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

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
    vector<string *> button_ids;
    string * selection_sprite_parent_id;
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


static Entity generate_button(
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
    return button;
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
    auto menu_buttons_handler = (Menu_Buttons_Handler *)get_component(entity, "menu_buttons_handler");
    const vector<string> & button_names = menu_buttons_handler->button_names;
    const int button_count = button_names.size();
    Menu_Buttons_Handler_State & entity_state = entity_states[entity];
    entity_state.menu_buttons_handler = menu_buttons_handler;
    entity_state.selection_sprite_parent_id = generate_selection_sprite(entity);
    vector<string *> & button_ids = entity_state.button_ids;
    const map<string, function<void()>> & button_handlers = menu_buttons_handler->button_handlers;
    const auto menu_id = (string *)get_component(entity, "id");


    // Load buttons.
    for (int i = 0; i < button_count; i++)
    {
        const string & button_name = button_names[i];


        // Set button up to call its associated handler in entity's Menu_Buttons_Handler component.
        Entity button_entity = generate_button(entity, button_name, *menu_id, i, button_count);
        auto button = (Button *)get_component(button_entity, "button");
        auto ui_mouse_event_handlers = (UI_Mouse_Event_Handlers *)get_component(button_entity, "ui_mouse_event_handlers");
        auto id = (string *)get_component(button_entity, "id");
        button_ids.push_back(id);

        button->click_handler = [&]() -> void
        {
            if (!contains_key(button_handlers, button_name))
            {
                throw runtime_error("ERROR: no handler set for the \"" + button_name + "\" button!");
            }

            button_handlers.at(button_name)();
        };

        ui_mouse_event_handlers->mouse_enter_handlers["menu_buttons_handler"] = [=]() -> void
        {
            menu_buttons_handler_select_button(entity, i);
        };
    }


    menu_buttons_handler_select_button(entity, 0);
}


void menu_buttons_handler_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void menu_buttons_handler_select_button(Entity entity, int index)
{
    Menu_Buttons_Handler_State & entity_state = entity_states[entity];
    const vector<string *> & button_ids = entity_state.button_ids;
    const int button_count = button_ids.size();

    if (index >= button_count)
    {
        throw runtime_error(
            "ERROR: cannot select button at index " + to_string(index) + "; menu only has " + to_string(button_count) +
            " buttons!");
    }

    *entity_state.selection_sprite_parent_id = *button_ids[index];
}


} // namespace Game
