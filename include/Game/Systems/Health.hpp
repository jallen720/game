#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void health_subscribe(Nito::Entity entity);
void health_unsubscribe(Nito::Entity entity);
void damage_entity(Nito::Entity entity, float amount);
void heal_entity(Nito::Entity entity, float amount);


} // namespace Game
