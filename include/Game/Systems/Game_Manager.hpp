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
    const std::function<void(int, int)> & room_change_handler);

void game_manager_remove_room_change_handler(const std::string & id);
void game_manager_complete_floor();
void game_manager_track_render_flag(int room, Nito::Entity entity);
void game_manager_untrack_render_flag(int room, Nito::Entity entity);
void game_manager_track_collider_enabled_flag(int room, Nito::Entity entity);
void game_manager_untrack_collider_enabled_flag(int room, Nito::Entity entity);
void game_manager_track_enemy_enabled_flag(int room, Nito::Entity entity);
void game_manager_untrack_enemy_enabled_flag(int room, Nito::Entity entity);
void game_manager_track_light_source_enabled_flag(int room, Nito::Entity entity);
void game_manager_untrack_light_source_enabled_flag(int room, Nito::Entity entity);


} // namespace Game
