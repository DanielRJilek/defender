#ifndef LASER_SPAWN_SYSTEM_HPP
#define LASER_SPAWN_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include "engine/ecs/entity_manager.hpp"

/**
 * LaserSpawnSystem - Game-level system for laser firing
 *
 * Reads Input fire state, checks cooldown and live laser cap,
 * and spawns laser entities. Separate from ShipControlSystem
 * (no god systems).
 *
 * Reads: Input.fire, Position, Velocity (optional)
 * Writes: Creates new laser entities, Blackboard ship.fire_cooldown_remaining
 */
class LaserSpawnSystem {
public:
    void update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager);
};

#endif // LASER_SPAWN_SYSTEM_HPP
