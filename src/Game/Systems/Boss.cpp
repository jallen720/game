#include "Game/Systems/Boss.hpp"

#include <vector>
#include <string>
#include <deque>
#include <stdexcept>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "Nito/Components.hpp"
#include "Nito/Engine.hpp"
#include "Nito/APIs/Scene.hpp"
#include "Nito/APIs/Window.hpp"
#include "Cpp_Utils/Collection.hpp"

#include "Game/Utilities.hpp"
#include "Game/Components.hpp"
#include "Game/APIs/Floor_Manager.hpp"
#include "Game/Systems/Game_Manager.hpp"


using std::vector;
using std::string;
using std::deque;
using std::runtime_error;
using std::fill;

// glm/glm.hpp
using glm::vec3;
using glm::vec2;
using glm::distance;
using glm::length;
using glm::degrees;

// glm/gtx/vector_angle.hpp
using glm::orientedAngle;

// Nito/APIs/ECS.hpp
using Nito::Entity;
using Nito::get_component;
using Nito::flag_entity_for_deletion;

// Nito/Components.hpp
using Nito::Transform;

// Nito/Engine.hpp
using Nito::get_time_scale;

// Nito/APIs/Scene.hpp
using Nito::load_blueprint;

// Nito/APIs/Window.hpp
using Nito::get_delta_time;

// Cpp_Utils/Collection.hpp
using Cpp_Utils::for_each;


namespace Game
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const int SEGMENT_COUNT = 7;
static const float FIRE_COOLDOWN = 2.0f;
static const float SEGMENT_FIRE_INTERVAL = FIRE_COOLDOWN / (SEGMENT_COUNT + 1);
static vec3 * position;
static vec3 * look_direction;
static vec2 destination(-1);
static float cooldown;
static float segment_cooldown;
static float segment_fire_index;
static float time_scale;
static int direction_index = 0;
static int boss_room;
static vector<Entity> segments;
static vector<Entity> segment_connectors;
static vector<vec2 *> segment_destinations;
static vector<vec3 *> segment_positions;
static vector<vec3 *> segment_look_directions;
static vector<Transform *> segment_connector_transforms;
static deque<vec2> destinations;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int wrap_index(int index, int container_size)
{
    return index >= container_size ? index - container_size :
           index < 0 ? index + container_size :
           index;
}


static void update_segments()
{
    for (int i = 0; i < SEGMENT_COUNT; i++)
    {
        *segment_destinations[i] = destinations[i];
    }
}


static const vec3 get_fire_direction_offset(const vec3 & fire_direction, int fire_direction_index)
{
    static const float HORIZONTAL_OFFSET = 0.3f;
    static const float VERTICAL_OFFSET = 0.15f;

    return fire_direction * ((fire_direction_index % 2 == 0) ? VERTICAL_OFFSET : HORIZONTAL_OFFSET);
}


static void fire_boss_projectile(int fire_direction_index, const vec3 & position)
{
    static const vector<vec3> FIRE_DIRECTIONS
    {
        vec3( 0, 1, 0),
        vec3( 1, 0, 0),
        vec3( 0,-1, 0),
        vec3(-1, 0, 0),
    };

    static const vector<string> TARGET_LAYERS
    {
        "player",
    };

    static const string PROJECTILE_NAME("projectile_purple_orb");
    static const float DURATION = 2.0f;

    const vec3 & fire_direction = FIRE_DIRECTIONS[wrap_index(fire_direction_index, FIRE_DIRECTIONS.size())];

    fire_projectile(
        PROJECTILE_NAME,
        position + get_fire_direction_offset(fire_direction, fire_direction_index),
        fire_direction,
        DURATION,
        TARGET_LAYERS);
}


static void fire_boss_projectiles(const vec3 & position, const vec3 & look_direction, bool forward = false)
{
    // Don't fire projectile if game is paused.
    if (time_scale == 0)
    {
        return;
    }


    int fire_direction_index =
        look_direction.y > 0 ? 0 :
        look_direction.x > 0 ? 1 :
        look_direction.y < 0 ? 2 :
        look_direction.x < 0 ? 3 :
        0;


    // Forward
    if (forward)
    {
        fire_boss_projectile(fire_direction_index, position);
    }


    // Right
    fire_boss_projectile(fire_direction_index + 1, position);


    // Left
    fire_boss_projectile(fire_direction_index - 1, position);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void boss_subscribe(Entity entity)
{
    position = &((Transform *)get_component(entity, "transform"))->position;
    look_direction = &((Orientation_Handler *)get_component(entity, "orientation_handler"))->look_direction;
    destination = vec2(-1);
    cooldown = 0.0f;
    segment_cooldown = SEGMENT_FIRE_INTERVAL;
    segment_fire_index = 0;
    direction_index = 0;
    boss_room = get_max_room_id();


    // Remove boss entity flags from game manager when boss dies.
    ((Health *)get_component(entity, "health"))->death_handlers["boss"] = [&]() -> void
    {
        for (const Entity segment : segments)
        {
            game_manager_untrack_render_flag(boss_room, segment);
            game_manager_untrack_collider_enabled_flag(boss_room, segment);
        }

        for (const Entity segment : segment_connectors)
        {
            game_manager_untrack_render_flag(boss_room, segment);
        }
    };
}


void boss_unsubscribe(Entity /*entity*/)
{
    position = nullptr;
    look_direction = nullptr;
    for_each(segments, flag_entity_for_deletion);
    for_each(segment_connectors, flag_entity_for_deletion);
    segments.clear();
    segment_connectors.clear();
    segment_destinations.clear();
    segment_positions.clear();
    segment_look_directions.clear();
    segment_connector_transforms.clear();
    destinations.clear();
}


void boss_update()
{
    if (position == nullptr)
    {
        return;
    }

    static const vector<vec2> DIRECTIONS
    {
        vec2( 1, 0),
        vec2( 0, 1),
        vec2(-1, 0),
        vec2( 0,-1),
    };


    time_scale = get_time_scale();
    const float delta_time = get_delta_time() * time_scale;


    // If destination is unset, search neighboring tiles for a new destination.
    if (destination.x == -1)
    {
        const vec2 current_tile_coordinates = get_room_tile_coordinates(*position);


        // Give 1:2 chance to randomly change direction.
        if (random(0, 3) == 0)
        {
            direction_index = wrap_index(direction_index + (random(0, 2) == 0 ? 1 : -1), DIRECTIONS.size());
        }


        for (size_t count = 0; count < DIRECTIONS.size(); count++)
        {
            const vec2 & direction = DIRECTIONS[direction_index];

            const vec2 destination_tile_coordinates(
                current_tile_coordinates.x + direction.x,
                current_tile_coordinates.y + direction.y);

            const Tile_Types destination_tile_type = get_room_tile(
                destination_tile_coordinates.x,
                destination_tile_coordinates.y).type;

            if (destination_tile_type != Tile_Types::FLOOR)
            {
                direction_index = wrap_index(direction_index + 1, DIRECTIONS.size());
                continue;
            }

            destination = get_room_tile_position(destination_tile_coordinates);
        }


        // If destination is still unset, no neighboring tile is navigable meaning the boss is out-of-bounds or
        // surrounded by non-navigable tiles.
        if (destination.x == -1)
        {
            throw runtime_error("ERROR: failed to find a navigable neighboring tile from boss' current tile!");
        }
    }


    const vec2 movement = move_entity(*position, *look_direction, destination);


    // Boss is close enough to destination or has passed it, so unset destination to be reset next frame.
    if (distance(destination, (vec2)*position) < length(movement))
    {
        // Update destinations with completed destination.
        destinations.push_front(destination);
        destinations.pop_back();


        update_segments();
        destination = vec2(-1);
    }


    // Handle firing.
    cooldown -= delta_time;
    segment_cooldown -= delta_time;


    if (cooldown <= 0.0f)
    {
        fire_boss_projectiles(*position, *look_direction, true);
        cooldown = FIRE_COOLDOWN;
        segment_cooldown = SEGMENT_FIRE_INTERVAL;
    }
    else if (segment_cooldown <= 0.0f)
    {
        fire_boss_projectiles(*segment_positions[segment_fire_index], *segment_look_directions[segment_fire_index]);
        segment_fire_index = wrap_index(segment_fire_index + 1, SEGMENT_COUNT);
        segment_cooldown = SEGMENT_FIRE_INTERVAL;
    }


    // Update boss segment connectors.
    static const vec2 BASE_ANGLE_VECTOR(0.0f, 1.0f);

    vec3 to_position = *position;

    for (int i = 0; i < SEGMENT_COUNT; i++)
    {
        const vec3 from_position = *segment_positions[i];
        const vec2 to_position_2d = (vec2)to_position;
        const vec2 from_position_2d = (vec2)from_position;
        Transform * segment_connector_transform = segment_connector_transforms[i];
        vec3 & segment_connector_position = segment_connector_transform->position;
        segment_connector_position.x = from_position.x;
        segment_connector_position.y = from_position.y;
        segment_connector_transform->scale.y = distance(to_position_2d, from_position_2d);

        segment_connector_transform->rotation =
            degrees(orientedAngle(BASE_ANGLE_VECTOR, normalize(to_position_2d - from_position_2d)));

        to_position = from_position;
    }
}


void boss_init()
{
    // Initialize all destinations to boss' position.
    destinations.resize(SEGMENT_COUNT);
    fill(destinations.begin(), destinations.end(), *position);


    // Create boss segments.
    for (int i = 0; i < SEGMENT_COUNT; i++)
    {
        // Load and track segment entity.
        Entity segment = load_blueprint("boss_segment");
        segments.push_back(segment);
        vec3 * segment_position = &((Transform *)get_component(segment, "transform"))->position;
        *segment_position = *position;
        segment_destinations.push_back((vec2 *)get_component(segment, "destination"));
        segment_positions.push_back(segment_position);

        segment_look_directions.push_back(
            &((Orientation_Handler *)get_component(segment, "orientation_handler"))->look_direction);

        game_manager_track_render_flag(boss_room, segment);
        game_manager_track_collider_enabled_flag(boss_room, segment);


        // Load and track segment connector entity.
        Entity segment_connector = load_blueprint("boss_segment_connector");
        segment_connectors.push_back(segment_connector);
        segment_connector_transforms.push_back((Transform *)get_component(segment_connector, "transform"));
        game_manager_track_render_flag(boss_room, segment_connector);
    }


    update_segments();
}


} // namespace Game
