#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void projectile_subscribe(const Nito::Entity entity);
void projectile_unsubscribe(const Nito::Entity entity);
void projectile_update();


} // namespace Game