#include "Game/Systems/Item.hpp"

#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Vector.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Game_Manager.hpp"


using std::vector;
using std::string;
using std::map;

// glm/glm.hpp
using glm::vec3;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::has_component;
using Nito::flag_entity_for_deletion;

// Nito/Components.hpp
using Nito::Transform;

// Nito/Collider_Component.hpp
using Nito::Collider;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Cpp_Utils/Vector.hpp
using Cpp_Utils::contains;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, int> item_rooms;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void item_subscribe(Entity entity)
{
    static const string PLAYER_LAYER("player");
    static const string LAYERS_COMPONENT("layers");

    const auto item = (Item *)get_component(entity, "item");

    ((Collider *)get_component(entity, "collider"))->collision_handler = [=](Entity collision_entity) -> void
    {
        if (has_component(collision_entity, LAYERS_COMPONENT) &&
            contains(*(vector<string> *)get_component(collision_entity, LAYERS_COMPONENT), PLAYER_LAYER))
        {
            if (item->pick_up_handler(collision_entity))
            {
                const int room = item_rooms.at(entity);
                flag_entity_for_deletion(entity);
                game_manager_untrack_render_flag(room, entity);
                game_manager_untrack_collider_enabled_flag(room, entity);
            }
        }
    };
}


void item_unsubscribe(Entity entity)
{
    remove(item_rooms, entity);
}


void spawn_item(Entity enemy)
{
    // Spawn item.
    const Entity item = load_blueprint("mini_health");

    ((Transform *)get_component(item, "transform"))->position =
        ((Transform *)get_component(enemy, "transform"))->position;


    // Track item in game manager.
    const int room = get_enemy_room(enemy);
    item_rooms[item] = room;
    game_manager_track_render_flag(room, item);
    game_manager_track_collider_enabled_flag(room, item);
}


} // namespace Game
