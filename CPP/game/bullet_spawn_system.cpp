#include "game/bullet_spawn_system.hpp"
#include <cmath>

std::pair<float, float> BulletSpawnSystem::calculate_bullet_velocity(
    ComponentStorage& storage, Blackboard& blackboard, Entity entity) {
    float bullet_speed = blackboard.get_or<float>("bullet.speed", 600.0f);

    auto alien_pos_opt = storage.get_component<Position>(entity);
    if (!alien_pos_opt.has_value()) {
        return {0.0f, 0.0f};
    }
    const Position& alien_pos = alien_pos_opt->get();

    float ax = alien_pos.x;
    float ay = alien_pos.y;
    if (auto sz = storage.get_component<Size>(entity); sz.has_value()) {
        ax += sz->get().width * 0.5f;
        ay += sz->get().height * 0.5f;
    }

    float sx = 0.0f;
    float sy = 0.0f;
    if (blackboard.has("entity.id.ship")) {
        Entity ship = blackboard.get<Entity>("entity.id.ship");
        auto ship_pos_opt = storage.get_component<Position>(ship);
        if (ship_pos_opt.has_value()) {
            const Position& ship_pos = ship_pos_opt->get();
            sx = ship_pos.x;
            sy = ship_pos.y;
            if (auto sz = storage.get_component<Size>(ship); sz.has_value()) {
                sx += sz->get().width * 0.5f;
                sy += sz->get().height * 0.5f;
            }
        }
    }

    float dx = sx - ax;
    float dy = sy - ay;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 1e-4f) {
        return {0.0f, 0.0f};
    }

    return {bullet_speed * (dx / dist), bullet_speed * (dy / dist)};
}

void BulletSpawnSystem::update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    // Read config from Blackboard with defaults
    float bullet_speed = blackboard.get_or<float>("bullet.speed", 600.0f);
    float bullet_lifetime = blackboard.get_or<float>("bullet.lifetime", 0.8f);
    float bullet_size = blackboard.get_or<float>("bullet.size", 64.0f);
    int max_live = blackboard.get_or<int>("bullet.max_live", 6);
    float fire_cooldown_config = blackboard.get_or<float>("bullet.fire_cooldown", 2.0f);
    int bullet_layer = blackboard.get_or<int>("bullet.layer", 2);
    int bullet_mask = blackboard.get_or<int>("bullet.mask", 4);
    auto ship_pos = blackboard.get_or<Position>("ship.position", Position{0.0f, 0.0f});
    
    auto entities = storage.entities_with_component<BulletFireRequest>();
    for (Entity entity : entities) {
        if (!storage.has_component<Position>(entity)) continue;
        auto [vx, vy] = calculate_bullet_velocity(storage, blackboard, entity);
        auto pos = storage.get_component<Position>(entity)->get();
        // Create bullet entity
        Entity bullet = entity_manager.create_entity();
        storage.add_component(bullet, Position{pos.x, pos.y});
        storage.add_component(bullet, Size{bullet_size, bullet_size});
        storage.add_component(bullet, Velocity{
            vx,
            vy
        });
        storage.add_component(bullet, Lifetime{bullet_lifetime});
        storage.add_component(bullet, WrapAround{});
        storage.add_component(bullet, Collider{
            bullet_size, bullet_size,
            static_cast<uint8_t>(bullet_layer),
            static_cast<uint8_t>(bullet_mask)
        });
        storage.add_component(bullet, Color{255, 255, 255, 255});
        storage.add_component(bullet, BulletTag{});
        storage.remove_component<BulletFireRequest>(entity);
    }
}