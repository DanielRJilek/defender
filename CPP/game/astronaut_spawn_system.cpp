#include "game/astronaut_spawn_system.hpp"
#include <cmath>
#include <string>
#include "game/wave_config.hpp"
#include "game/level_config.hpp"

AstronautSpawnSystem::AstronautSpawnSystem(uint32_t seed)
    : rng_(seed) {}

AstronautSpawnSystem::AstronautSpawnSystem()
    : rng_(std::random_device{}()) {}

Entity AstronautSpawnSystem::spawn_astronaut(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager, float x, float y) {
    auto size = blackboard.get_or<Size>("astronaut.size", Size{0.0f, 0.0f});
    auto images = blackboard.get_or<Images>("astronaut.images", Images{{}});
    auto script = blackboard.get_or<Script>("astronaut.script", Script{"wander"});
    auto astronaut = entity_manager.create_entity();
    storage.add_component<AstronautTag>(astronaut, AstronautTag{});
    storage.add_component<Position>(astronaut, Position{x, y});
    storage.add_component<Velocity>(astronaut, Velocity{0.0f, 0.0f});
    storage.add_component<Size>(astronaut, size);
    storage.add_component<Images>(astronaut, images);
    storage.add_component<Collider>(astronaut, Collider{size.width, size.height, 8, 7});
    storage.add_component<Direction>(astronaut, Direction{1});
    storage.add_component<Script>(astronaut, script);
    storage.add_component<WrapAround>(astronaut, WrapAround{});
    return astronaut;
}

void AstronautSpawnSystem::spawn_wave(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    auto level_configs = blackboard.get<std::shared_ptr<std::vector<LevelConfig>>>("level_configs");
    auto current_level = blackboard.get<int>("game.level");
    std::uniform_real_distribution<float> x_dist(-1000.0f, 1000.0f);
    for (int i = 0; i < level_configs->at(current_level).astronaut_count; i++) {
        float x;
        // Rejection sampling: exclude 100px safe zone around (0, 0)
        do {
            x = x_dist(rng_);
        } while (std::abs(x) <= 100.0f);
        spawn_astronaut(storage, blackboard, entity_manager, x, -375.0f);
    }
}

void AstronautSpawnSystem::update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    auto game_state = blackboard.get<std::string>("game.state");
    auto current_wave = blackboard.get<int>("game.wave");
    if (game_state == "GAME_START" || (game_state == "SPAWN_WAVE" && current_wave == 0)) {
        spawn_wave(storage, blackboard, entity_manager);
    }
    return;
}
