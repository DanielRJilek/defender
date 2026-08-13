#include "brute_force_strategy.hpp"
#include "collision_math.hpp"
#include <algorithm>

std::vector<std::pair<Entity, Entity>>
BruteForceStrategy::detect(const ComponentStorage& storage) const {
    std::vector<std::pair<Entity, Entity>> results;

    // Get all entities with Position, filter to those also having Collider
    auto pos_entities = storage.entities_with_component<Position>();
    std::vector<Entity> candidates;
    for (Entity e : pos_entities) {
        if (storage.has_component<Collider>(e)) {
            candidates.push_back(e);
        }
    }

    // Nested loop over unique pairs (i < j)
    for (size_t i = 0; i < candidates.size(); ++i) {
        for (size_t j = i + 1; j < candidates.size(); ++j) {
            Entity a = candidates[i];
            Entity b = candidates[j];

            auto col_a = storage.get_component<Collider>(a);
            auto col_b = storage.get_component<Collider>(b);

            // Layer/mask filter first (cheap)
            if (!layers_compatible(col_a->get().layer, col_a->get().mask,
                                   col_b->get().layer, col_b->get().mask)) {
                continue;
            }

            // AABB overlap test
            auto pos_a = storage.get_component<Position>(a);
            auto pos_b = storage.get_component<Position>(b);

            if (!aabb_overlap(pos_a->get().x, pos_a->get().y,
                              col_a->get().width, col_a->get().height,
                              pos_b->get().x, pos_b->get().y,
                              col_b->get().width, col_b->get().height)) {
                continue;
            }

            // Normalize pair (smaller ID first)
            Entity lo = std::min(a, b);
            Entity hi = std::max(a, b);
            results.emplace_back(lo, hi);
        }
    }

    return results;
}
