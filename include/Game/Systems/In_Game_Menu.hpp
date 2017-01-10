#pragma once


#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void in_game_menu_subscribe(Nito::Entity entity);
void in_game_menu_unsubscribe(Nito::Entity entity);
void in_game_menu_set_on(bool on);


} // namespace Game
