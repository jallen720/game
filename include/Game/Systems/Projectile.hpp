#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void projectile_subscribe(Nito::Entity entity);
void projectile_unsubscribe(Nito::Entity entity);
void projectile_update();


} // namespace Game
