#ifndef DIRECTION_SYSTEM_HPP
#define DIRECTION_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

class DirectionSystem {
    public:
        void update(ComponentStorage& storage, Blackboard& blackboard);
};

#endif // DIRECTION_SYSTEM_HPP