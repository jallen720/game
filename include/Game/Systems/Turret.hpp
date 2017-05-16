#pragma once


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
void turret_init();
void turret_subscribe(Nito::Entity entity);
void turret_unsubscribe(Nito::Entity entity);
void turret_update();
std::vector<Nito::Entity> turret_generate(int room, int room_origin_x, int room_origin_y);
const std::map<int, std::map<Nito::Entity, glm::ivec2>> & get_turret_room_tiles();


} // namespace Game
