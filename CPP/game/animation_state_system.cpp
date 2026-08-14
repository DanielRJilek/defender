#include "game/animation_state_system.hpp"
#include <string>

void AnimationStateSystem::update(ComponentStorage& storage, const Blackboard& blackboard) {
    auto entities = storage.entities_with_component<AnimationState>();

    for (Entity entity : entities) {
        // Must have Animation component
        auto anim_opt = storage.get_component<Animation>(entity);
        if (!anim_opt.has_value()) continue;

        auto& anim_state = storage.get_component<AnimationState>(entity)->get();
        auto& anim = anim_opt->get();

        bool changed = (anim_state.current_state != anim_state.previous_state);

        if (changed) {
            std::string prefix = "anim_def." + anim_state.current_state;

            // Look up definition — skip if not found
            if (blackboard.has(prefix + ".start_frame")) {
                anim.start_frame = blackboard.get<int>(prefix + ".start_frame");
                anim.frame_count = blackboard.get<int>(prefix + ".frame_count");
                anim.frame_duration = blackboard.get<float>(prefix + ".frame_duration");
                anim.looping = blackboard.get<bool>(prefix + ".looping");
                anim.elapsed = 0.0f;
                anim.playing = true;
                anim.finished = false;
                anim.current_frame = anim.start_frame;
            }
        }

        // Sync state tracking (always, regardless of whether Animation was updated)
        anim_state.state_changed = changed;
        anim_state.previous_state = anim_state.current_state;
    }
}
