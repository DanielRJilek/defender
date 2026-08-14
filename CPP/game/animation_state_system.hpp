#ifndef ANIMATION_STATE_SYSTEM_HPP
#define ANIMATION_STATE_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

/**
 * AnimationStateSystem
 *
 * Reads AnimationState changes and updates Animation component parameters
 * from data-driven definitions stored in the Blackboard.
 *
 * For each entity with both AnimationState and Animation:
 *   - If current_state != previous_state (state change detected):
 *     - Look up animation definition from Blackboard using BlockType.gem_color
 *     - Update Animation parameters (start_frame, frame_count, frame_duration, looping)
 *     - Reset Animation playback (elapsed=0, playing=true, finished=false, current_frame=start_frame)
 *   - Sync previous_state = current_state
 *   - Set state_changed flag
 *
 * Runs in the always-run section before GridRenderSystem and AnimationSystem.
 * No SDL dependency — fully testable without SDL initialization.
 */
class AnimationStateSystem {
public:
    void update(ComponentStorage& storage, const Blackboard& blackboard);
};

#endif // ANIMATION_STATE_SYSTEM_HPP
