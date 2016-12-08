#include "Game/Systems/Player_Controller.hpp"

#include <map>
#include <string>
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
using std::string;

// glm/glm.hpp
using glm::vec3;

// glm/gtc/matrix_transform.hpp
using glm::normalize;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::create_entity;
using Nito::add_component;
using Nito::subscribe_to_system;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Sprite;
using Nito::Dimensions;

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
    vec3 look_direction;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void player_fire(const Entity entity)
{
    const Entity_State & entity_state = entity_states[entity];
    const vec3 & player_position = entity_state.transform->position;
    const vec3 projectile_position(player_position.x, player_position.y, -2.0f);
    const Entity projectile = create_entity();
    add_component(projectile, "transform", new Transform { projectile_position, vec3(1.0f), 0.0f });
    add_component(projectile, "projectile", new Projectile { 3.0f, entity_state.look_direction });
    add_component(projectile, "sprite", new Sprite { "resources/textures/projectile.png", "texture" });
    add_component(projectile, "dimensions", new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) });
    add_component(projectile, "render_layer", new string("world"));
    subscribe_to_system(projectile, "projectile");
    subscribe_to_system(projectile, "sprite_dimensions_handler");
    subscribe_to_system(projectile, "renderer");
}


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
        vec3(),
    };


    // Set player fire handler to controller button 5.
    set_controller_button_handler("player_fire", 5, Button_Actions::PRESS, [=]() -> void
    {
        player_fire(entity);
    });
}


void player_controller_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void player_controller_update()
{
    for_each(entity_states, [](const Entity /*entity*/, Entity_State & entity_state) -> void
    {
        const Player_Controller * player_controller = entity_state.player_controller;
        const float stick_dead_zone = player_controller->stick_dead_zone;
        vec3 & look_direction = entity_state.look_direction;


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

        look_direction =
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
