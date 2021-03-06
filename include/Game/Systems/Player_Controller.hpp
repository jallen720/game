#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void player_controller_subscribe(Nito::Entity entity);
void player_controller_unsubscribe(Nito::Entity entity);
void player_controller_update();


} // namespace Game
