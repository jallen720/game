#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void item_subscribe(Nito::Entity entity);
void item_unsubscribe(Nito::Entity entity);


} // namespace Game