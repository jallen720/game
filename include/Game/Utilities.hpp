#pragma once


#include <string>
#include <vector>
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


} // namespace Game
