/**
 * HUDUpdateSystem - Game-level system for updating HUD text from Blackboard
 *
 * Reads game.score, game.lives, game.wave, and game.state from the Blackboard,
 * formats them into display strings, and writes the results to the Text component
 * on the corresponding HUD entities (hud_score, hud_lives, hud_wave, hud_game_over).
 *
 * Entity lookup uses Blackboard keys: entity.id.hud_score, entity.id.hud_lives,
 * entity.id.hud_wave, entity.id.hud_game_over (set by gamedata_loader).
 *
 * Runs every frame regardless of game state so the game-over overlay updates
 * even when simulation is paused.
 *
 * Reads: Blackboard (game.score, game.lives, game.wave, game.state, entity IDs)
 * Writes: Text.content on HUD entities
 */

#ifndef HUD_UPDATE_SYSTEM_HPP
#define HUD_UPDATE_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

class HUDUpdateSystem {
public:
    void update(ComponentStorage& storage, const Blackboard& blackboard);
};

#endif // HUD_UPDATE_SYSTEM_HPP
