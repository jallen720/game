#pragma once


#include <map>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include "Nito/APIs/ECS.hpp"


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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Components
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Player_Controller
{
    enum class Modes
    {
        CONTROLLER,
        KEYBOARD_MOUSE,
    };

    float speed;
    float stick_dead_zone;
    Modes mode;
};


struct Projectile
{
    float speed;
    glm::vec3 direction;
    float duration;
    std::vector<std::string> target_layers;
};


struct Orientation_Handler
{
    Orientation orientation;
    glm::vec3 look_direction;
    std::map<Orientation, std::string> orientation_texture_paths;
};


struct Health
{
    float max;
    float current;
    std::map<std::string, std::function<void()>> death_handlers;
};


struct Menu_Buttons_Handler
{
    std::vector<std::string> button_names;
    std::map<std::string, std::function<void()>> button_handlers;
};


struct Room_Exit
{
    enum class Types
    {
        DOOR,
        NEXT_FLOOR,
    };

    Types type;
    bool locked;
    std::string locked_texture_path;
};


} // namespace Game
