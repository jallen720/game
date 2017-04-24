#pragma once


#include <vector>
#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void wall_launcher_init();
void wall_launcher_subscribe(Nito::Entity entity);
void wall_launcher_unsubscribe(Nito::Entity entity);
void wall_launcher_update();
std::vector<Nito::Entity> wall_launcher_generate(int room, int room_origin_x, int room_origin_y);


} // namespace Game
