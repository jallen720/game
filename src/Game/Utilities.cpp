#include "Game/Utilities.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "Nito/Components.hpp"
#include "Nito/APIs/ECS.hpp"

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
using Nito::Collider;

// Nito/APIs/ECS.hpp
using Nito::generate_entity;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fire_projectile(const vec3 & origin, const vec3 & direction, float duration, const vector<string> & target_layers)
{
    static const vector<string> PROJECTILE_SYSTEMS
    {
        "projectile",
        "sprite_dimensions_handler",
        "renderer",
        "depth_handler",
        "collider",
    };

    generate_entity(
        {
            { "transform"    , new Transform { origin, vec3(1.0f), 0.0f }                             },
            { "projectile"   , new Projectile { 3.0f, normalize(direction), duration, target_layers } },
            { "sprite"       , new Sprite { "resources/textures/projectile.png", "texture" }          },
            { "dimensions"   , new Dimensions { 0.0f, 0.0f, vec3(0.5f, 0.5f, 0.0f) }                  },
            { "collider"     , new Collider { true, 0.065f, {} }                                      },
            { "render_layer" , new string("world")                                                    },
        },
        PROJECTILE_SYSTEMS);
}


} // namespace Game
