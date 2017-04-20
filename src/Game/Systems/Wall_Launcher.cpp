#include "Game/Systems/Wall_Launcher.hpp"

#include <map>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Cpp_Utils/Map.hpp"

#include "Game/APIs/Floor_Manager.hpp"


using std::vector;
using std::map;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Wall_Launcher_Entity_State
{
    vec3 * position;
    vector<vec2> path;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Wall_Launcher_Entity_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void wall_launcher_subscribe(Entity entity)
{
    entity_states[entity] =
    {
        &((Transform *)get_component(entity, "transform"))->position,
        vector<vec2>(),
    };
}


void wall_launcher_unsubscribe(Entity entity)
{
    remove(entity_states, entity);
}


void wall_launcher_update()
{

}


vector<Entity> wall_launcher_generate(int room_origin_x, int room_origin_y)
{
    static const vector<vec2> SPAWN_COORDINATES
    {
        vec2(1, 1),
        vec2(1, 7),
        vec2(11, 7),
        vec2(11, 1),
    };

    const vec3 room_tile_texture_scale = get_room_tile_texture_scale();
    vector<Entity> wall_launchers;

    for (const vec2 & spawn_coordinates : SPAWN_COORDINATES)
    {
        const Entity wall_launcher = load_blueprint("wall_launcher");
        wall_launchers.push_back(wall_launcher);

        *entity_states[wall_launcher].position =
            vec3(room_origin_x + spawn_coordinates.x, room_origin_y + spawn_coordinates.y, 0) * room_tile_texture_scale;
    }

    return wall_launchers;
}


} // namespace Game
