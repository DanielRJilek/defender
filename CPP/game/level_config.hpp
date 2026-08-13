#ifndef LEVEL_CONFIG_HPP
#define LEVEL_CONFIG_HPP

#include <string>
#include "game/wave_config.hpp"

/**
 * LevelConfig — defines parameters for one level.
 * Parsed from GameData.json "game.levels" array.
 */
struct LevelConfig {
    std::vector<WaveConfig> waves;
    int astronaut_count = 0;
    float elapsed_time = 0.0f;
    float wave_delay = 10.0f;
    float wave_time_limit = 10.0f;
    int spawned_baiters = 0;
};

#endif // LEVEL_CONFIG_HPP