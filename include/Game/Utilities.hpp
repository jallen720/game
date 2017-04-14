#pragma once


#include <string>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fire_projectile(
    const glm::vec3 & origin,
    const glm::vec3 & direction,
    float duration,
    const std::vector<std::string> & target_layers);

int random(int min, int max);
bool in_layer(Nito::Entity entity, const std::string & layer);

template<typename T>
T * array_2d_at(T * array_2d, int width, int x, int y);

template<typename T>
void iterate_array_2d(
    T * array_2d,
    int width,
    int start_x,
    int start_y,
    int sub_width,
    int sub_height,
    bool relative_coordinates,
    const std::function<void(int, int, T &)> & callback);

template<typename T>
void iterate_array_2d(T * array_2d, int width, int height, const std::function<void(int, int, T &)> & callback);

glm::vec2 move_entity(glm::vec3 & position, glm::vec3 & look_direction, const glm::vec2 & destination);


} // namespace Game


#include "Game/Utilities.ipp"
