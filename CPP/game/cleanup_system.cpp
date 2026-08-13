#include "cleanup_system.hpp"

void CleanupSystem::update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    auto game_state = blackboard.get<std::string>("game.state");
    if (game_state == "LEVEL_COMPLETE") {
        auto aliens = storage.entities_with_component<AlienTag>();
        for (auto alien : aliens) {
            storage.add_component(alien, DestroyRequest{});
        }
        auto astronauts = storage.entities_with_component<AstronautTag>();
        for (auto astronaut : astronauts) {
            storage.add_component(astronaut, DestroyRequest{});
        }
        auto bullets = storage.entities_with_component<BulletTag>();
        for (auto bullet : bullets) {
            storage.add_component(bullet, DestroyRequest{});
        }
        auto lasers = storage.entities_with_component<LaserTag>();
        for (auto laser : lasers) {
            storage.add_component(laser, DestroyRequest{});
        }
    }
}