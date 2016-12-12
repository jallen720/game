#include <GL/glew.h>

#include "Game/Systems/Player_Controller.hpp"

#include <map>
#include <string>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;
using std::string;
using std::runtime_error;

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

// Nito/Graphics.hpp
using Nito::get_pixels_per_unit;

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
enum class Orientation
{
    LEFT,
    UP,
    RIGHT,
    DOWN,
};


struct Entity_State
{
    Transform * transform;
    Dimensions * dimensions;
    Sprite * sprite;
    const Player_Controller * player_controller;
    vec3 look_direction;
    Orientation orientation;
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
    static const float PROJECTILE_DURATION = 1.0f;
    static const float VERTICAL_Y_OFFSET = 0.05f;

    const Entity_State & entity_state = entity_states[entity];
    const vec3 & player_position = entity_state.transform->position;
    const Orientation orientation = entity_state.orientation;


    // Calculate projectile's position.
    const float horizontal_x_offset = (entity_state.dimensions->width / get_pixels_per_unit() / 2) - 0.05f;
    vec3 projectile_position(player_position.x, player_position.y, 0.0f);

    if (orientation == Orientation::RIGHT)
    {
        projectile_position.x += horizontal_x_offset;
    }
    else if (orientation == Orientation::LEFT)
    {
        projectile_position.x -= horizontal_x_offset;
    }
    else if (orientation == Orientation::UP)
    {
        projectile_position.y += VERTICAL_Y_OFFSET;
    }
    else
    {
        projectile_position.y -= VERTICAL_Y_OFFSET;
    }

    projectile_position.z = projectile_position.y;


    // Generate projectile entity.
    const Entity projectile = create_entity();
    add_component(projectile, "transform", new Transform { projectile_position, vec3(1.0f), 0.0f });
    add_component(projectile, "projectile", new Projectile { 3.0f, entity_state.look_direction, PROJECTILE_DURATION });
    add_component(projectile, "sprite", new Sprite { "resources/textures/projectile.png", "texture" });
    add_component(projectile, "dimensions", new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) });
    add_component(projectile, "render_layer", new string("world"));
    subscribe_to_system(projectile, "projectile");
    subscribe_to_system(projectile, "sprite_dimensions_handler");
    subscribe_to_system(projectile, "renderer");
    subscribe_to_system(projectile, "depth_handler");
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
        (Dimensions *)get_component(entity, "dimensions"),
        (Sprite *)get_component(entity, "sprite"),
        (Player_Controller *)get_component(entity, "player_controller"),
        vec3(0.0f, -1.0f, 0.0f),
        Orientation::DOWN,
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
        Orientation & orientation = entity_state.orientation;


        // Get move direction and modifiy entity position.
        const vec3 left_stick_direction(
            get_controller_axis(Controller_Axes::LEFT_STICK_X),
            -get_controller_axis(Controller_Axes::LEFT_STICK_Y),
            0.0f);

        const vec3 move_direction(
            fabsf(left_stick_direction.x) > stick_dead_zone ? left_stick_direction.x : 0.0f,
            fabsf(left_stick_direction.y) > stick_dead_zone ? left_stick_direction.y : 0.0f,
            0.0f);

        if (move_direction.x != 0.0f || move_direction.y != 0.0f)
        {
            entity_state.transform->position += normalize(move_direction) * player_controller->speed * get_delta_time();
        }


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
            // Only update look direction if it's not (0, 0).
            entity_state.look_direction = normalize(look_direction);

            if (fabsf(look_direction.x) > fabsf(look_direction.y))
            {
                orientation = look_direction.x < 0 ? Orientation::LEFT : Orientation::RIGHT;
            }
            else
            {
                orientation = look_direction.y < 0 ? Orientation::DOWN : Orientation::UP;
            }
        }


        entity_state.sprite->texture_path =
            orientation == Orientation::LEFT ? "resources/textures/player_left.png" :
            orientation == Orientation::UP ? "resources/textures/player_up.png" :
            orientation == Orientation::RIGHT ? "resources/textures/player_right.png" :
            orientation == Orientation::DOWN ? "resources/textures/player_down.png" :
            throw runtime_error("ERROR: unknown orientation for player!");
    });
}


} // namespace Game
