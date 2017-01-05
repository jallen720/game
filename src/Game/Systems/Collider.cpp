#include "Game/Systems/Collider.hpp"

#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Nito/Components.hpp"
#include "Nito/Utilities.hpp"
#include "Nito/APIs/Graphics.hpp"
#include "Cpp_Utils/Map.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Components.hpp"


using std::map;
using std::string;

// glm/glm.hpp
using glm::distance;
using glm::vec2;
using glm::vec3;
using glm::vec4;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;

// Nito/Components.hpp
using Nito::Transform;

// Nito/Utilities.hpp
using Nito::calculate_model_matrix;

// Nito/APIs/Graphics.hpp
using Nito::get_pixels_per_unit;
using Nito::load_render_data;
using Nito::Render_Modes;
using Nito::Render_Data;
using Nito::Uniform;

// Cpp_Utils/Map.hpp
using Cpp_Utils::remove;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Collider_State
{
    const Collider * collider;
    const Transform * transform;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static map<Entity, Collider_State> entity_states;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void collider_subscribe(const Entity entity)
{
    entity_states[entity] =
    {
        (Collider *)get_component(entity, "collider"),
        (Transform *)get_component(entity, "transform"),
    };
}


void collider_unsubscribe(const Entity entity)
{
    remove(entity_states, entity);
}


void collider_update()
{
    static float pixels_per_unit = get_pixels_per_unit();


    // TODO: could be optimized with an overload of for_each().
    for_each(entity_states, [=](const Entity entity, Collider_State & entity_state) -> void
    {
        const Transform * entity_transform = entity_state.transform;
        const Collider * entity_collider = entity_state.collider;
        const vec3 & entity_position = entity_transform->position;
        const vec3 & entity_scale = entity_transform->scale;
        const float entity_radius = entity_collider->radius;


        // Check for collisions with other colliders.
        for_each(entity_states, [&](const Entity other_entity, Collider_State & other_entity_state) -> void
        {
            // Don't check for collisions against self.
            if (other_entity == entity)
            {
                return;
            }

            const Transform * other_entity_transform = other_entity_state.transform;

            const float collision_distance =
                (entity_radius * entity_scale.x) +
                (other_entity_state.collider->radius * other_entity_transform->scale.x);

            if (distance((vec2)entity_position, (vec2)other_entity_transform->position) <= collision_distance)
            {
                const Collider::Collision_Handler & entity_collision_handler = entity_collider->collision_handler;

                if (entity_collision_handler)
                {
                    entity_collision_handler(other_entity);
                }
            }
        });


        // Render collider if flagged.
        if (entity_collider->render)
        {
            static vec4 color(0.0f, 0.7f, 0.0f, 1.0f);

            static const Render_Data::Uniforms uniforms
            {
                { "color", Uniform { Uniform::Types::VEC4, &color } },
            };

            static const string layer_name("world");
            static const string shader_pipeline_name("color");
            static const string vertex_container_id("collider");
            static const vec3 origin(0.0f);
            static const float rotation = 0.0f;

            const float dimensional_size = entity_radius * pixels_per_unit * 2;

            load_render_data(
                {
                    Render_Modes::LINE_STRIP,
                    &layer_name,
                    nullptr,
                    &shader_pipeline_name,
                    &vertex_container_id,
                    &uniforms,
                    calculate_model_matrix(
                        dimensional_size,
                        dimensional_size,
                        origin,
                        entity_position,
                        entity_scale,
                        rotation)
                });
        }
    });
}


} // namespace Game
