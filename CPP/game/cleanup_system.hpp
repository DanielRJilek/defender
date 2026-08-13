#ifndef CLEANUP_SYSTEM_HPP
#define CLEANUP_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include "engine/ecs/entity_manager.hpp"

class CleanupSystem {
    public:
        void update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager);
};

#endif // CLEANUP_SYSTEM_HPP