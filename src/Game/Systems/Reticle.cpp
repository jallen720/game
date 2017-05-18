#include <GL/glew.h>

#include "Game/Systems/Reticle.hpp"

#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Graphics.hpp"


// glm/glm.hpp
using glm::vec3;
using glm::dvec2;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::UI_Transform;

// Nito/APIs/Input.hpp
using Nito::set_mouse_visible;
using Nito::get_mouse_position;

// Nito/APIs/Graphics.hpp
using Nito::get_pixels_per_unit;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static vec3 * ui_position;
static const dvec2 * mouse_position;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reticle_subscribe(Entity entity)
{
    // Whenever reticle is displayed mouse should be hidden.
    set_mouse_visible(false);


    ui_position = &((UI_Transform *)get_component(entity, "ui_transform"))->position;
    mouse_position = &get_mouse_position();
}


void reticle_unsubscribe(Entity /*entity*/)
{
    // Whenever reticle is destroyed mouse should be visible again.
    set_mouse_visible(true);


    ui_position = nullptr;
    mouse_position = nullptr;
}


void reticle_update()
{
    if (ui_position == nullptr)
    {
        return;
    }

    const int pixels_per_unit = get_pixels_per_unit();
    ui_position->x = mouse_position->x / pixels_per_unit;
    ui_position->y = mouse_position->y / pixels_per_unit;
}


} // namespace Game
