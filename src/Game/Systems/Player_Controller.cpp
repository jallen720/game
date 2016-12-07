#include "Game/Systems/Player_Controller.hpp"

#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;

// glm/glm.hpp
using glm::vec3;

// glm/gtc/matrix_transform.hpp
using glm::normalize;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Sprite;

// Nito/Input.hpp
using Nito::debug_controllers;
using Nito::get_controller_axis;
using Nito::set_controller_button_handler;
using Nito::Controller_Axes;
using Nito::Button_Actions;

// Nito/Window.hpp
using Nito::get_delta_time;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Entity_State
{
    Transform * transform;
    Sprite * sprite;
    const Player_Controller * player_controller;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void player_controller_subscribe(const Entity entity)
{
    entity_states[entity] =
    {
        (Transform *)get_component(entity, "transform"),
        (Sprite *)get_component(entity, "sprite"),
        (Player_Controller *)get_component(entity, "player_controller"),
    };

    set_controller_button_handler(
        "player",
        {
            0,
            5,
            Button_Actions::PRESS,
            []() -> void { puts("fire!"); },
        });
}


void player_controller_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void player_controller_update()
{
    for_each(entity_states, [](const Entity /*entity*/, const Entity_State & entity_state) -> void
    {
        const Player_Controller * player_controller = entity_state.player_controller;
        const float stick_dead_zone = player_controller->stick_dead_zone;


        // Get move direction and modifiy entity position.
        const vec3 left_stick_direction(
            get_controller_axis(Controller_Axes::LEFT_STICK_X),
            -get_controller_axis(Controller_Axes::LEFT_STICK_Y),
            0.0f);

        const vec3 move_direction(
            fabsf(left_stick_direction.x) > stick_dead_zone ? left_stick_direction.x : 0.0f,
            fabsf(left_stick_direction.y) > stick_dead_zone ? left_stick_direction.y : 0.0f,
            0.0f);

        entity_state.transform->position += move_direction * player_controller->speed * get_delta_time();


        // Set texture path for entity's look direction based on right stick input if any or move direction otherwise.
        const vec3 right_stick_direction(
            get_controller_axis(Controller_Axes::RIGHT_STICK_X),
            -get_controller_axis(Controller_Axes::RIGHT_STICK_Y),
            0.0f);

        const vec3 & look_direction =
            fabsf(right_stick_direction.x) > stick_dead_zone || fabsf(right_stick_direction.y) > stick_dead_zone
            ? right_stick_direction
            : move_direction;


        // Don't change texture path if look direction is (0, 0).
        if (look_direction.x != 0.0f || look_direction.y != 0.0f)
        {
            if (fabsf(look_direction.x) > fabsf(look_direction.y))
            {
                entity_state.sprite->texture_path =
                    look_direction.x < 0
                    ? "resources/textures/player_left.png"
                    : "resources/textures/player_right.png";
            }
            else
            {
                entity_state.sprite->texture_path =
                    look_direction.y < 0
                    ? "resources/textures/player_down.png"
                    : "resources/textures/player_up.png";
            }
        }
    });
}


} // namespace Game
