#ifndef WAVE_CONFIG_HPP
#define WAVE_CONFIG_HPP

#include <string>

/**
 * WaveConfig — defines parameters for one wave of enemies.
 * Parsed from GameData.json "game.waves" array.
 */
struct WaveConfig {
    int lander_count = 5;
    int swarmer_count = 0;
    int baiter_count = 0;
    float elapsed_time = 0.0f;
    float last_baiter_timer = 0.0f;
};

#endif // WAVE_CONFIG_HPP
