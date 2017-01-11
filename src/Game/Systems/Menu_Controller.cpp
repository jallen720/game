#include "Game/Systems/Menu_Controller.hpp"

#include <map>
#include <stdexcept>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/String.hpp"

#include "Game/Systems/Menu_Buttons_Handler.hpp"


using std::map;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;
using Cpp_Utils::contains_key;

// Cpp_Utils/String.hpp
using Cpp_Utils::to_string;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Transform *> entity_transforms;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void menu_controller_subscribe(Entity entity)
{
    entity_transforms[entity] = (Transform *)get_component(entity, "transform");
}


void menu_controller_unsubscribe(Entity entity)
{
    remove(entity_transforms, entity);
}


void menu_controller_set_on(Entity entity, bool on)
{
    static const vec3 ON_SCALE(1.0f);
    static const vec3 OFF_SCALE(0.0f);

    if (!contains_key(entity_transforms, entity))
    {
        throw runtime_error("ERROR: entity " + to_string(entity) + " is not subscribed to the menu_controller system!");
    }

    entity_transforms[entity]->scale = on ? ON_SCALE : OFF_SCALE;


    // Default to first button whenever menu is opened.
    if (on)
    {
        menu_buttons_handler_select_button(entity, 0);
    }
}


} // namespace Game
