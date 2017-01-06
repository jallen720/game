#include "Game/Systems/Quit_Button.hpp"

#include <map>
#include "Nito/Components.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"


using std::map;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Button;

// Nito/APIs/Scene.hpp
using Nito::set_scene_to_load;

// Nito/APIs/Window.hpp
using Nito::close_window;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Button *> entity_buttons;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void quit_button_subscribe(const Entity entity)
{
    auto button = (Button *)get_component(entity, "button");
    entity_buttons[entity] = button;

    button->click_handler = [=]() -> void
    {
        close_window();
    };
}


void quit_button_unsubscribe(const Entity entity)
{
    static const auto dud = []() -> void {};

    entity_buttons[entity]->click_handler = dud;
    remove(entity_buttons, entity);
}


} // namespace Game
