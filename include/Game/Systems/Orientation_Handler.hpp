#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void orientation_handler_subscribe(Nito::Entity entity);
void orientation_handler_unsubscribe(Nito::Entity entity);
void orientation_handler_update();


} // namespace Game
