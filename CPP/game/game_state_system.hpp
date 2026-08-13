#ifndef GAME_STATE_SYSTEM_HPP
#define GAME_STATE_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

/**
 * GameStateSystem
 *
 * Tracks level and waveprogression, and
 * game state transitions (playing, wave_complete, level_complete, game_over).
 *
 * init_level: reads level config from Blackboard, writes initial game state.
 * update: per-frame processing — tracks alien count, transitions state.
 */
class GameStateSystem {
public:
    void init_level(int level_number, Blackboard& blackboard);
    void update(ComponentStorage& storage, Blackboard& blackboard);
};

#endif // GAME_STATE_SYSTEM_HPP