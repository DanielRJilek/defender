#ifndef BULLET_SPAWN_SYSTEM_HPP
#define BULLET_SPAWN_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include "engine/ecs/entity_manager.hpp"
#include <utility>

/**
 * BulletSpawnSystem - Game-level system for alien bullet firing
 *
 * Spawns bullets for entities marked with BulletFireRequest (set from Lua).
 * Aim direction is toward the player ship.
 */
class BulletSpawnSystem {
public:
    /// Unit direction from entity toward the ship, scaled by bullet.speed.
    std::pair<float, float> calculate_bullet_velocity(ComponentStorage& storage,
                                                      Blackboard& blackboard,
                                                      Entity entity);

    void update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager);
};

#endif // BULLET_SPAWN_SYSTEM_HPP
