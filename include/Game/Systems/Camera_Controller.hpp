#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void camera_controller_subscribe(const Nito::Entity entity);
void camera_controller_unsubscribe(const Nito::Entity entity);
void camera_controller_update();


} // namespace Game