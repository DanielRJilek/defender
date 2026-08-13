#ifndef ABDUCTION_SYSTEM_HPP
#define ABDUCTION_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "alien_spawn_system.hpp"

/**
 * AbductionSystem - Game-level system for abducting aliens
 * Handles aliens abducting astronauts and players then rescuing them
 * for all entities with abducted tag check if their abuductor still exists
 * if not, remove abducted tag and add falling tag
 * if alien reaches top, destroy astronaut and remove abducting tag from alien
 * if astronaut with falling tag reaches bottom, destroy astronaut
 * if astronaut with rescued tag reaches bottom, remove rescued tag and award points
 */

 class AbductionSystem {
    public:
        explicit AbductionSystem(AlienSpawnSystem& alien_spawn_system);
        void update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager);
    private:
        bool astronaut_reached_bottom(ComponentStorage& storage, Entity abducted_entity);
        void move_hanging_entity_to_holder(ComponentStorage& storage, Entity abducted_entity, Entity abductor);
        AlienSpawnSystem alien_spawn_system_;
 };

 #endif