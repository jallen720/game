#include <GL/glew.h>

#include "Game/Systems/Player_Controller.hpp"

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include "Nito/Engine.hpp"
#include "Nito/Components.hpp"
#include "Nito/APIs/Input.hpp"
#include "Nito/APIs/Window.hpp"
#include "Nito/APIs/Graphics.hpp"

#include "Game/Components.hpp"
#include "Game/Utilities.hpp"


using std::map;
using std::vector;
using std::string;
using std::runtime_error;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// glm/gtc/matrix_transform.hpp
using glm::normalize;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::get_entity;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Dimensions;

// Nito/Input.hpp
using Nito::get_controller_axis;
using Nito::get_key_button_action;
using Nito::get_mouse_position;
using Nito::set_controller_button_handler;
using Nito::set_mouse_button_handler;
using Nito::remove_controller_button_handler;
using Nito::remove_mouse_button_handler;
using Nito::DS4_Axes;
using Nito::DS4_Buttons;
using Nito::Mouse_Buttons;
using Nito::Keys;
using Nito::Button_Actions;

// Nito/Window.hpp
using Nito::get_delta_time;
using Nito::get_window_size;

// Nito/Graphics.hpp
using Nito::get_pixels_per_unit;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const string FIRE_HANDLER_ID = "player_controller fire";
static const vector<string> TARGET_LAYERS { "enemy" };
static Transform * transform;
static Dimensions * dimensions;
static Orientation_Handler * orientation_handler;
static const Player_Controller * player_controller;
static Transform * camera_transform;
static vec3 * camera_origin;
static int pixels_per_unit;
static float time_scale;
static vec2 mouse_world_position;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool is_paused()
{
    return time_scale == 0;
}


static void fire()
{
    static const float VERTICAL_Y_OFFSET = 0.05f;
    static const string PROJECTILE_NAME("projectile_blue_orb");


    // Don't fire if the game is paused.
    if (is_paused())
    {
        return;
    }


    const vec3 & player_position = transform->position;
    const Orientation orientation = orientation_handler->orientation;


    // Calculate projectile's position.
    const float horizontal_x_offset = (dimensions->width / pixels_per_unit / 2) - 0.05f;
    vec3 projectile_origin(player_position.x, player_position.y, 0);

    if (orientation == Orientation::RIGHT)
    {
        projectile_origin.x += horizontal_x_offset;
    }
    else if (orientation == Orientation::LEFT)
    {
        projectile_origin.x -= horizontal_x_offset;
    }
    else if (orientation == Orientation::UP)
    {
        projectile_origin.y += VERTICAL_Y_OFFSET;
    }
    else
    {
        projectile_origin.y -= VERTICAL_Y_OFFSET;
    }

    projectile_origin.z = projectile_origin.y;


    // Fire projectile.
    vec3 fire_direction;
    const vec2 position_2d = (vec2)transform->position;

    if (player_controller->mode == Player_Controller::Modes::CONTROLLER ||
        distance(mouse_world_position, position_2d) < distance((vec2)projectile_origin, position_2d))
    {
        fire_direction = orientation_handler->look_direction;
    }
    else
    {
        fire_direction = vec3(mouse_world_position.x, mouse_world_position.y, 0) - projectile_origin;
    }

    fire_projectile(
        PROJECTILE_NAME,
        projectile_origin,
        fire_direction,
        1.0f,
        TARGET_LAYERS);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void player_controller_subscribe(Entity entity)
{
    if (transform != nullptr)
    {
        throw runtime_error("ERROR: only one entity is allowed to be subscribed to the player_controller system!");
    }

    Entity camera = get_entity("camera");
    transform = (Transform *)get_component(entity, "transform");
    dimensions = (Dimensions *)get_component(entity, "dimensions");
    orientation_handler = (Orientation_Handler *)get_component(entity, "orientation_handler");
    player_controller = (Player_Controller *)get_component(entity, "player_controller");
    camera_transform = (Transform *)get_component(camera, "transform");
    camera_origin = &((Dimensions *)get_component(camera, "dimensions"))->origin;
    pixels_per_unit = get_pixels_per_unit();


    // Set player fire handler to controller button 5.
    set_controller_button_handler(FIRE_HANDLER_ID, DS4_Buttons::R1, Button_Actions::PRESS, fire);


    // Set player fire handler to left click.
    set_mouse_button_handler(FIRE_HANDLER_ID, [](Mouse_Buttons mouse_button, Button_Actions button_action) -> void
    {
        if (mouse_button == Mouse_Buttons::LEFT && button_action == Button_Actions::PRESS)
        {
            fire();
        }
    });
}


void player_controller_unsubscribe(Entity /*entity*/)
{
    remove_controller_button_handler(FIRE_HANDLER_ID);
    remove_mouse_button_handler(FIRE_HANDLER_ID);
    transform = nullptr;
    dimensions = nullptr;
    orientation_handler = nullptr;
    player_controller = nullptr;
    camera_transform = nullptr;
    camera_origin = nullptr;
}


void player_controller_update()
{
    if (transform == nullptr)
    {
        return;
    }

    time_scale = get_time_scale();
    const float delta_time = get_delta_time() * time_scale;
    vec3 & player_position = transform->position;
    vec3 move_direction;
    vec3 look_direction;

    if (player_controller->mode == Player_Controller::Modes::CONTROLLER)
    {
        const float stick_dead_zone = player_controller->stick_dead_zone;


        // Calculate move direction based on left stick.
        const vec3 left_stick_direction(
            get_controller_axis(DS4_Axes::LEFT_STICK_X),
            -get_controller_axis(DS4_Axes::LEFT_STICK_Y),
            0.0f);

        move_direction.x = fabsf(left_stick_direction.x) > stick_dead_zone ? left_stick_direction.x : 0.0f;
        move_direction.y = fabsf(left_stick_direction.y) > stick_dead_zone ? left_stick_direction.y : 0.0f;


        // Calculate look direction based on right stick.
        const vec3 right_stick_direction(
            get_controller_axis(DS4_Axes::RIGHT_STICK_X),
            -get_controller_axis(DS4_Axes::RIGHT_STICK_Y),
            0.0f);

        look_direction =
            fabsf(right_stick_direction.x) > stick_dead_zone ||
            fabsf(right_stick_direction.y) > stick_dead_zone
            ? right_stick_direction
            : move_direction;
    }
    else if (player_controller->mode == Player_Controller::Modes::KEYBOARD_MOUSE)
    {
        move_direction.x =
            get_key_button_action(Keys::D) == Button_Actions::PRESS ? 1 :
            get_key_button_action(Keys::A) == Button_Actions::PRESS ? -1 :
            0;

        move_direction.y =
            get_key_button_action(Keys::W) == Button_Actions::PRESS ? 1 :
            get_key_button_action(Keys::S) == Button_Actions::PRESS ? -1 :
            0;

        const vec3 & camera_position = camera_transform->position;
        const vec2 mouse_position = (vec2)get_mouse_position();
        const vec2 mouse_unit_camera_position = mouse_position / (float)pixels_per_unit;
        const vec3 window_camera_origin_offset = (get_window_size() * *camera_origin) / (float)pixels_per_unit;

        const vec2 mouse_camera_offset =
            (mouse_unit_camera_position - (vec2)window_camera_origin_offset) / (vec2)camera_transform->scale;

        mouse_world_position = (vec2)camera_position + mouse_camera_offset;
        const vec2 mouse_direction = normalize(mouse_world_position - (vec2)player_position);
        look_direction.x = mouse_direction.x;
        look_direction.y = mouse_direction.y;
    }


    if (move_direction.x != 0.0f || move_direction.y != 0.0f)
    {
        player_position += normalize(move_direction) * player_controller->speed * delta_time;
    }


    // Don't update look direction if game is paused.
    if (is_paused())
    {
        return;
    }


    // Only update look direction if it's not (0, 0).
    if (look_direction.x != 0.0f || look_direction.y != 0.0f)
    {
        orientation_handler->look_direction = look_direction;
    }
}


} // namespace Game
