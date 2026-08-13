#include "game/game_state_system.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "game/wave_config.hpp"
#include "game/level_config.hpp"

void GameStateSystem::init_level(int level_number, Blackboard& blackboard) {
    blackboard.set<int>("game.level", level_number);
    blackboard.set<float>("game.spawn_delay", 2.0f);
    blackboard.set<float>("game.spawn_delay_timer", 0.0f);
    std::string prefix = "levels." + std::to_string(level_number);
}

void GameStateSystem::update(ComponentStorage& storage, Blackboard& blackboard) {
    auto level_configs = blackboard.get<std::shared_ptr<std::vector<LevelConfig>>>("level_configs");
    auto current_level = blackboard.get<int>("game.level");
    auto current_wave = blackboard.get<int>("game.wave");
    auto alien_count = storage.entities_with_component<AlienTag>().size();
    auto astronaut_count = storage.entities_with_component<AstronautTag>().size();
    auto game_state = blackboard.get<std::string>("game.state");
    double delta_time = blackboard.get<double>("delta_time");
    float dt_f = static_cast<float>(delta_time);

    if (game_state == "PLAYING" && astronaut_count == 0) {
        blackboard.set<std::string>("game.state", "GAME_OVER");
        return;
    }

    if (game_state == "WAVE_COMPLETE" || game_state == "LEVEL_COMPLETE") {
        blackboard.set<float>("game.spawn_delay_timer", blackboard.get<float>("game.spawn_delay_timer") + dt_f);
        if (game_state == "LEVEL_COMPLETE") {
            blackboard.set<int>("game.saved_score", blackboard.get<int>("game.score"));
        }
        if (blackboard.get<float>("game.spawn_delay_timer") >= blackboard.get<float>("game.spawn_delay")) {
            blackboard.set<std::string>("game.state", "SPAWN_WAVE");
            return;
        }
    }

    if (alien_count > 0) {
        blackboard.set<std::string>("game.state", "PLAYING");
        return;
    }

    else if (alien_count == 0 && game_state == "PLAYING") {
        if (current_wave >= level_configs->at(current_level).waves.size() - 1) {
            if (current_level >= level_configs->size() - 1) {
                blackboard.set<std::string>("game.state", "WIN");
                return;
            }
            blackboard.set<int>("game.level", current_level + 1);
            blackboard.set<int>("game.wave", 0);
            blackboard.set<std::string>("game.state", "LEVEL_COMPLETE");
            blackboard.set<float>("game.spawn_delay_timer", 0.0f);
            return;
        }
        blackboard.set<int>("game.wave", current_wave + 1);
        blackboard.set<std::string>("game.state", "WAVE_COMPLETE");
        blackboard.set<float>("game.spawn_delay_timer", 0.0f);
        return;
    }
}