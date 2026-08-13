#include "game/hud_update_system.hpp"
#include "engine/ecs/components.hpp"
#include <string>

void HUDUpdateSystem::update(ComponentStorage& storage, const Blackboard& blackboard) {
    // --- Score ---
    if (blackboard.has("entity.id.hud_score")) {
        Entity e = blackboard.get<Entity>("entity.id.hud_score");
        if (storage.has_component<Text>(e)) {
            int score = blackboard.get_or<int>("game.score", 0);
            storage.get_component<Text>(e)->get().content = "Score: " + std::to_string(score);
        }
    }

    // --- Lives ---
    if (blackboard.has("entity.id.hud_lives")) {
        Entity e = blackboard.get<Entity>("entity.id.hud_lives");
        if (storage.has_component<Text>(e)) {
            int lives = blackboard.get_or<int>("game.lives", 0);
            storage.get_component<Text>(e)->get().content = "Lives: " + std::to_string(lives);
        }
    }

    // --- Level ---
    if (blackboard.has("entity.id.hud_level")) {
        Entity e = blackboard.get<Entity>("entity.id.hud_level");
        if (storage.has_component<Text>(e)) {
            int level = blackboard.get_or<int>("game.level", 0) + 1;
            storage.get_component<Text>(e)->get().content = "Level: " + std::to_string(level);
        }
    }

    // --- Wave ---
    if (blackboard.has("entity.id.hud_wave")) {
        Entity e = blackboard.get<Entity>("entity.id.hud_wave");
        if (storage.has_component<Text>(e)) {
            int wave = blackboard.get_or<int>("game.wave", 0) + 1;
            storage.get_component<Text>(e)->get().content = "Wave: " + std::to_string(wave);
        }
    }

    // --- Game Over ---
    if (blackboard.has("entity.id.hud_game_over")) {
        Entity e = blackboard.get<Entity>("entity.id.hud_game_over");
        if (storage.has_component<Text>(e)) {
            std::string state = blackboard.get_or<std::string>("game.state", std::string("PLAYING"));
            storage.get_component<Text>(e)->get().content = (state == "GAME_OVER") ? "GAME OVER" : (state == "WIN") ? "YOU WIN!" : "";
        }
    }

    // --- Hyperspace ---
    if (blackboard.has("entity.id.hud_hyperspace")) {
        Entity e = blackboard.get<Entity>("entity.id.hud_hyperspace");
        if (storage.has_component<Text>(e)) {
            int hyperspace = blackboard.get_or<int>("game.hyperspace_uses", 0);
            storage.get_component<Text>(e)->get().content = "Hyperspace Jumps: " + std::to_string(hyperspace);
        }
    }
}
