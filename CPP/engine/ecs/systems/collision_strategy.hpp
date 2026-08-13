#ifndef COLLISION_STRATEGY_HPP
#define COLLISION_STRATEGY_HPP

#include "engine/ecs/component_storage.hpp"
#include <vector>
#include <utility>

/**
 * Abstract interface for collision detection strategies.
 *
 * Each strategy receives a const reference to ComponentStorage and returns
 * a vector of collision pairs. Pairs are normalized: smaller Entity ID first.
 * No duplicates.
 *
 * Implementations: BruteForceStrategy (Phase 2), UniformGridStrategy (Phase 8),
 * QuadtreeStrategy (Phase 8).
 */
class CollisionStrategy {
public:
    virtual ~CollisionStrategy() = default;

    /**
     * Detect all colliding entity pairs.
     *
     * @param storage Component storage to query for Position and Collider
     * @return Vector of (Entity, Entity) pairs where first < second
     */
    virtual std::vector<std::pair<Entity, Entity>>
    detect(const ComponentStorage& storage) const = 0;
};

#endif // COLLISION_STRATEGY_HPP
