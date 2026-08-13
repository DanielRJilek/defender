#include "hyperspace_system.hpp"

HyperspaceSystem::HyperspaceSystem(uint32_t seed)
    : rng_(seed) {}

HyperspaceSystem::HyperspaceSystem()
    : rng_(std::random_device{}()) {}

void HyperspaceSystem::update(ComponentStorage& storage, Blackboard& blackboard) {
    auto hyperspace_state = blackboard.get<std::string>("game.hyperspace_state");
    auto hyperspace_uses = blackboard.get<int>("game.hyperspace_uses");
    if (hyperspace_state == "REQUESTED" && hyperspace_uses > 0) {
        Entity ship = storage.entities_with_component<ShipTag>().front();
        auto pos_opt = storage.get_component<Position>(ship);
        if (!pos_opt.has_value()) {
            return;
        }
        Position& pos = pos_opt->get();
        std::uniform_real_distribution<float> x_dist(-1000.0f, 1000.0f);
        float x = x_dist(rng_);
        float y = x_dist(rng_);
        pos.x = x;
        pos.y = y;
        blackboard.set<std::string>("game.hyperspace_state", "INACTIVE");
        hyperspace_uses--;
        blackboard.set<int>("game.hyperspace_uses", hyperspace_uses);
    }
}