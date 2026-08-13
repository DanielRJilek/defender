#ifndef ASTRONAUT_SPAWN_SYSTEM_HPP
#define ASTRONAUT_SPAWN_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include "engine/ecs/entity_manager.hpp"
#include <random>
#include <cstdint>

/**
 * AstronautSpawnSystem - Game-level system for astronaut lifecycle
 *
 * The single authority for creating and destroying astronaut entities.
 */
class AstronautSpawnSystem {
    public:
        explicit AstronautSpawnSystem(uint32_t seed);
        AstronautSpawnSystem();  // Uses std::random_device for seed
    
        void update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager);
    
    private:
        std::mt19937 rng_;
        int current_wave_ = 0;
    
        Entity spawn_astronaut(ComponentStorage& storage, Blackboard& blackboard,
                              EntityManager& entity_manager,
                              float x, float y);
        void spawn_wave(ComponentStorage& storage, Blackboard& blackboard,
                        EntityManager& entity_manager);
    };
    
    #endif // ASTRONAUT_SPAWN_SYSTEM_HPP