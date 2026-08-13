#include "game/laser_spawn_system.hpp"
#include <cmath>
#include <algorithm>

void LaserSpawnSystem::update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    double delta_time = blackboard.get<double>("delta_time");
    float dt = static_cast<float>(delta_time);

    // Read config from Blackboard with defaults
    float laser_speed = blackboard.get_or<float>("laser.speed", 400.0f);
    float laser_lifetime = blackboard.get_or<float>("laser.lifetime", 2.0f);
    auto laser_size = blackboard.get_or<Size>("laser.size", Size{0.0f, 0.0f});
    int max_live = blackboard.get_or<int>("laser.max_live", 6);
    float fire_cooldown_config = blackboard.get_or<float>("laser.fire_cooldown", 0.25f);
    int laser_layer = blackboard.get_or<int>("laser.layer", 2);
    int laser_mask = blackboard.get_or<int>("laser.mask", 4);
    auto images = blackboard.get_or<Images>("laser.images", Images{{"laser.png"}, 0});

    // Decrement cooldown, clamp at zero
    float cooldown = blackboard.get_or<float>("ship.fire_cooldown_remaining", 0.0f);
    cooldown = std::max(0.0f, cooldown - dt);
    blackboard.set("ship.fire_cooldown_remaining", cooldown);

    // Iterate entities with Input
    auto entities = storage.entities_with_component<Input>();
    for (Entity entity : entities) {
        auto& input = storage.get_component<Input>(entity)->get();
        if (!input.fire) continue;

        // Need Position and Rotation to spawn
        if (!storage.has_component<Position>(entity)) continue;

        // Check cooldown
        if (cooldown > 0.0f) continue;

        // Check max live cap
        auto laser_count = storage.entities_with_component<LaserTag>().size();
        if (static_cast<int>(laser_count) >= max_live) continue;

        // Get ship state
        auto& pos = storage.get_component<Position>(entity)->get();
        // auto& rot = storage.get_component<Rotation>(entity)->get();

        float ship_dx = 0.0f, ship_dy = 0.0f;
        if (storage.has_component<Velocity>(entity)) {
            auto& vel = storage.get_component<Velocity>(entity)->get();
            ship_dx = vel.dx;
            ship_dy = vel.dy;
        }

        // Compute laser spawn at the ship's nose (tip)
        // Ship position is bottom-left corner; center is (pos.x + half, pos.y + half)
        // Facing direction: (-sin(angle), cos(angle)) — texture points up
        float half_ship_height = 0.0f;
        float half_ship_width = 0.0f;
        if (storage.has_component<Size>(entity)) {
            auto& sz = storage.get_component<Size>(entity)->get();
            half_ship_height = sz.height / 2.0f;
            half_ship_width = sz.width / 2.0f;
        }
        float center_x = pos.x + half_ship_width;
        float center_y = pos.y + half_ship_height;
        float dir_x = 0.0f;
        float dir_y = 0.0f;
        float nose_x = center_x + dir_x * half_ship_width - laser_size.width / 2.0f;
        float nose_y = center_y + dir_y * half_ship_height - laser_size.height / 2.0f;

        // Check ship direction
        float laser_velocity = 0.0f;
        if (ship_dx > 0) {
            laser_velocity = ship_dx + laser_speed;
        }
        else if (ship_dx < 0) {
            laser_velocity = ship_dx - laser_speed;
        }
        
        // Create laser entity
        Entity laser = entity_manager.create_entity();
        storage.add_component(laser, Position{nose_x, nose_y});
        storage.add_component(laser, laser_size);
        storage.add_component(laser, Velocity{
            laser_velocity,
            0.0f
        });
        storage.add_component(laser, Lifetime{laser_lifetime});
        storage.add_component(laser, WrapAround{});
        storage.add_component(laser, Collider{
            laser_size.width, laser_size.height,
            static_cast<uint8_t>(laser_layer),
            static_cast<uint8_t>(laser_mask)
        });
        storage.add_component(laser, Color{255, 255, 255, 255});
        storage.add_component(laser, LaserTag{});
        storage.add_component(laser, images);

        // Reset cooldown
        cooldown = fire_cooldown_config;
        blackboard.set("ship.fire_cooldown_remaining", cooldown);
    }
}
