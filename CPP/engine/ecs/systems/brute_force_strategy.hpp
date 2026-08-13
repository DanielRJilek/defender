#ifndef BRUTE_FORCE_STRATEGY_HPP
#define BRUTE_FORCE_STRATEGY_HPP

#include "collision_strategy.hpp"

/**
 * Brute force collision detection — O(N²) pairwise comparison.
 *
 * Queries ComponentStorage for all entities with both Position and Collider.
 * For each unique pair, applies layer/mask filter first (cheap), then AABB
 * overlap test. Pairs are normalized (smaller ID first) and deduplicated.
 */
class BruteForceStrategy : public CollisionStrategy {
public:
    std::vector<std::pair<Entity, Entity>>
    detect(const ComponentStorage& storage) const override;
};

#endif // BRUTE_FORCE_STRATEGY_HPP
