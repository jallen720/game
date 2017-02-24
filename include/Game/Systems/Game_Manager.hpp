#pragma once


#include <string>
#include <functional>
#include "Nito/APIs/ECS.hpp"


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void game_manager_subscribe(Nito::Entity entity);
void game_manager_unsubscribe(Nito::Entity entity);
void game_manager_change_rooms(float door_rotation);

void game_manager_add_room_change_handler(
    const std::string & id,
    const std::function<void(char, char)> & room_change_handler);

void game_manager_remove_room_change_handler(const std::string & id);


} // namespace Game
