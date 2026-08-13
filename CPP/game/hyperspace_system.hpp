#ifndef HYPERSPACE_SYSTEM_HPP
#define HYPERSPACE_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <random>
#include <cstdint>

class HyperspaceSystem {
    public:
        explicit HyperspaceSystem(uint32_t seed);
        HyperspaceSystem();
        void update(ComponentStorage& storage, Blackboard& blackboard);
    private:
        std::mt19937 rng_;
};

#endif // HYPERSPACE_SYSTEM_HPP