#pragma once


#include <glm/glm.hpp>


namespace Game
{


struct Player_Controller
{
    float speed;
    float stick_dead_zone;
};


struct Projectile
{
    float speed;
    glm::vec3 direction;
};


} // namespace Game
