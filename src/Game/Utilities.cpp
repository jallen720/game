#include "Game/Utilities.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <ctime>
#include "Nito/Components.hpp"
#include "Nito/Collider_Component.hpp"
#include "Nito/APIs/Scene.hpp"

#include "Game/Components.hpp"


using std::string;
using std::vector;

// glm/glm.hpp
using glm::vec3;

// glm/gtc/matrix_transform.hpp
using glm::normalize;

// Nito/Components.hpp
using Nito::Transform;
using Nito::Sprite;
using Nito::Dimensions;
using Nito::Circle_Collider;

// Nito/Collider_Component.hpp
using Nito::Collider;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::has_component;
using Nito::get_component;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;


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


} // namespace Game
