#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void health_bar_subscribe(Nito::Entity entity);
void health_bar_unsubscribe(Nito::Entity entity);
void health_bar_update();


} // namespace Game
