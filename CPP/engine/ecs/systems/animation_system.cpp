#include "engine/ecs/systems/animation_system.hpp"

void AnimationSystem::update(ComponentStorage& storage, const Blackboard& blackboard) {
    double delta_time = blackboard.get<double>("delta_time");
    float dt = static_cast<float>(delta_time);

    auto entities = storage.entities_with_component<Animation>();
    for (Entity entity : entities) {
        auto anim_opt = storage.get_component<Animation>(entity);
        if (!anim_opt.has_value()) {
            continue;
        }
        Animation& anim = anim_opt->get();

        // This runs every game frame (fast); the animation's frames are slower.
        // Accumulate the trusted delta_time and step one frame each time a full
        // frame_duration has elapsed.
        if (anim.playing) {
            anim.elapsed += dt;

            if (anim.frame_duration > 0.0f && anim.elapsed >= anim.frame_duration) {
                anim.elapsed -= anim.frame_duration;
                anim.current_frame++;

                // Check sequence boundary
                if (anim.current_frame >= anim.start_frame + anim.frame_count) {
                    if (anim.looping) {
                        anim.current_frame = anim.start_frame;
                    } else {
                        // One-shot: stop on last valid frame
                        anim.current_frame = anim.start_frame + anim.frame_count - 1;
                        anim.finished = true;
                        anim.playing = false;
                    }
                }
            }
        }

        // Synchronize to SpriteSheet (always, regardless of playing state)
        auto ss_opt = storage.get_component<SpriteSheet>(entity);
        if (ss_opt.has_value()) {
            ss_opt->get().current_frame = anim.current_frame;
        }
    }
}
