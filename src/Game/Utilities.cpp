#include "Game/Utilities.hpp"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/Engine.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Window.hpp"

#include "Game/Components.hpp"


using std::isnan;
using std::string;
using std::vector;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;

// glm/gtc/matrix_transform.hpp
using glm::normalize;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Sprite;
using Nito::Dimensions;
using Nito::Circle_Collider;

// Nito/Collider_Component.hpp
using Nito::Collider;

// Nito/Components.hpp
using Nito::get_time_scale;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::has_component;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fire_projectile(const vec3 & origin, const vec3 & direction, float duration, const vector<string> & target_layers)
{
    Entity projectile_entity = load_blueprint("projectile");
    auto projectile = (Projectile *)get_component(projectile_entity, "projectile");
    ((Transform *)get_component(projectile_entity, "transform"))->position = origin;
    projectile->speed = 3.0f;
    projectile->direction = normalize(direction);
    projectile->duration = duration;
    projectile->target_layers = target_layers;
}


int random(int min, int max)
{
    static bool srand_set = false;

    if (!srand_set)
    {
        srand(time(NULL));
        srand_set = true;
    }

    return min == max ? min : (rand() % (max - min)) + min;
}


bool in_layer(Entity entity, const string & layer)
{
    return has_component(entity, "layer") && *(string *)get_component(entity, "layer") == layer;
}


vec2 move_entity(vec3 & position, vec3 & look_direction, const vec2 & destination)
{
    const float time_scale = get_time_scale();


    // Don't update boss entity position/orientation if game is paused.
    if (time_scale == 0)
    {
        return vec2(0);
    }


    // Calculate movement_direction.
    vec2 movement_direction = normalize(destination - (vec2)position);

    if (isnan(movement_direction.x))
    {
        movement_direction.x = 0;
    }

    if (isnan(movement_direction.y))
    {
        movement_direction.y = 0;
    }


    const vec2 movement = movement_direction * get_delta_time() * time_scale;
    position.x += movement.x;
    position.y += movement.y;
    look_direction.x = movement.x;
    look_direction.y = movement.y;
    return movement;
}


} // namespace Game
