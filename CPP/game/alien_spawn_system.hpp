#ifndef ALIEN_SPAWN_SYSTEM_HPP
#define ALIEN_SPAWN_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include "engine/ecs/entity_manager.hpp"
#include <random>
#include <cstdint>

/**
 * AlienSpawnSystem - Game-level system for alien lifecycle
 *
 * The single authority for creating and destroying alien entities.
 * Handles three triggers:
 *   1. Spawning new waves when wave_complete or level_complete
 *   2. Spawning the initial wave when level 0 starts
 *   3. Spawning baiters if the player is taking too long to complete the level
 */
class AlienSpawnSystem {
    public:
        explicit AlienSpawnSystem(uint32_t seed);
        AlienSpawnSystem();  // Uses std::random_device for seed
    
        void update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager);
        Entity spawn_alien(ComponentStorage& storage, Blackboard& blackboard,
            EntityManager& entity_manager, AlienType type,
            float x, float y);
    
    private:
        std::mt19937 rng_;
        int current_wave_ = 0;
        float elapsed_time_ = 0.0f;
        float spawn_timer_ = 0.0f;
        bool all_waves_complete_ = false;
    
        
        void spawn_wave(ComponentStorage& storage, Blackboard& blackboard,
                        EntityManager& entity_manager);
    };
    
    #endif // ALIEN_SPAWN_SYSTEM_HPP