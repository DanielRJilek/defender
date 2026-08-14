#include "game/alien_spawn_system.hpp"
#include <cmath>
#include <string>
#include "game/wave_config.hpp"
#include "game/level_config.hpp"

AlienSpawnSystem::AlienSpawnSystem()
    : rng_(std::random_device{}()) {}

Entity AlienSpawnSystem::spawn_alien(ComponentStorage& storage, Blackboard& blackboard, 
                                    EntityManager& entity_manager, AlienType type, float x, float y) {
    std::string type_str = type == AlienType::lander ? "lander" : type == AlienType::swarmer ? "swarmer" : "baiter";
    auto size = blackboard.get_or<Size>("alien." + type_str + ".size", Size{0.0f, 0.0f});
    auto script = blackboard.get_or<Script>("alien." + type_str + ".script", Script{"wander"});
    float speed = blackboard.get_or<float>("alien." + type_str + ".speed", 50.0f);

    Entity alien = entity_manager.create_entity();
    storage.add_component<AlienTag>(alien, AlienTag{});
    storage.add_component<AlienType>(alien, AlienType{type});
    storage.add_component<Position>(alien, Position{x, y});
    storage.add_component<Velocity>(alien, Velocity{0, 0});
    storage.add_component<Size>(alien, size);
    storage.add_component<Collider>(alien, Collider{size.width, size.height, 4, 11});
    storage.add_component<Direction>(alien, Direction{1});
    storage.add_component<Script>(alien, script);
    storage.add_component<WrapAround>(alien, WrapAround{});

    Images images;
    if (blackboard.has("alien." + type_str + ".images")) {
        images = blackboard.get<Images>("alien." + type_str + ".images");
        storage.add_component<Images>(alien, images);
    }

    std::vector<std::string> animations;
    if (blackboard.has("alien." + type_str + ".animations")) {
        animations = blackboard.get<std::vector<std::string>>("alien." + type_str + ".animations");
        for (const auto& animation : animations) {
            storage.add_component<Animation>(alien, Animation{0, 0, 3, 0.1f, 0.0f, true, false, false});
            std::string state = blackboard.get<std::string>("anim_def." + animation + ".initial_state");
            std::string prefix = "anim_def." + animation + "." + state;
            std::string row = blackboard.get<std::string>(prefix + ".row");
            int frame_width = blackboard.get<int>("atlas." + row + ".frame_width");
            int frame_height = blackboard.get<int>("atlas." + row + ".frame_height");
            int atlas_columns = blackboard.get<int>("atlas." + row + ".columns");
            int start_height = blackboard.get<int>("atlas." + row + ".start_height");
            std::string atlas_filename = blackboard.get<std::string>("atlas.filename");
            storage.add_component<SpriteSheet>(alien, SpriteSheet{atlas_filename, frame_width, frame_height, atlas_columns, start_height});
            storage.add_component<AnimationState>(alien, AnimationState{animation + "." + state, "", false});
        }
    } 

    blackboard.set("entity." + std::to_string(alien) + ".speed", speed);
    return alien;
}

void AlienSpawnSystem::spawn_wave(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    auto level_configs = blackboard.get<std::shared_ptr<std::vector<LevelConfig>>>("level_configs");
    auto current_level = blackboard.get<int>("game.level");
    auto current_wave = blackboard.get<int>("game.wave");
    auto waves = &level_configs->at(current_level).waves.at(current_wave);
    if (!waves) return;

    std::uniform_real_distribution<float> x_dist(-1000.0f, 1000.0f);

    for (int i = 0; i < waves->lander_count; i++) {
        float x;
        // Rejection sampling: exclude 100px safe zone around (0, 0)
        do {
            x = x_dist(rng_);
        } while (std::abs(x) <= 100.0f);
        spawn_alien(storage, blackboard, entity_manager, AlienType::lander, x, 300.0f);
    }
    for (int i = 0; i < waves->swarmer_count; i++) {
        float x;
        do {
            x = x_dist(rng_);
        } while (std::abs(x) <= 100.0f);
        spawn_alien(storage, blackboard, entity_manager, AlienType::swarmer, x, 300.0f);
    }
}

void AlienSpawnSystem::update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    double delta_time = blackboard.get<double>("delta_time");
    float dt = static_cast<float>(delta_time);
    auto game_state = blackboard.get<std::string>("game.state");
    auto level_configs = blackboard.get<std::shared_ptr<std::vector<LevelConfig>>>("level_configs");
    auto level = blackboard.get<int>("game.level");
    auto wave = blackboard.get<int>("game.wave");
    auto current_wave = &level_configs->at(level).waves.at(wave);
    auto current_level = &level_configs->at(level);
    auto baiter_count = current_wave->baiter_count;
    auto spawned_baiters = current_level->spawned_baiters;

    if (!current_wave) return;
    current_wave->elapsed_time += dt;
    if (game_state == "GAME_START" || game_state == "SPAWN_WAVE") {
        spawn_wave(storage, blackboard, entity_manager);
    }
    if (spawned_baiters < baiter_count && current_wave->elapsed_time >= current_level->wave_time_limit) {
        current_wave->last_baiter_timer += dt;
        if (current_wave->last_baiter_timer >= 2.0f) {
            current_wave->last_baiter_timer = 0.0f;
            spawn_alien(storage, blackboard, entity_manager, AlienType::baiter, 0.0f, 300.0f);
            current_level->spawned_baiters++;
        }
    }
    return;
}