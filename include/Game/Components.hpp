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
    float speed;
    float stick_dead_zone;
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


struct Collider
{
    using Collision_Handler = std::function<void(Nito::Entity)>;

    bool render;
    float radius;
    Collision_Handler collision_handler;
};


struct Health
{
    float max;
    float current;
};


} // namespace Game
