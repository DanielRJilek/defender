#include "game/direction_system.hpp"

void DirectionSystem::update(ComponentStorage& storage, Blackboard& blackboard) {
    auto entities = storage.entities_with_component<Direction>();
    for (auto entity : entities) {
        auto direction_opt = storage.get_component<Direction>(entity);
        if (!direction_opt.has_value()) {
            continue;
        }
        auto& direction = direction_opt->get();
        auto velocity_opt = storage.get_component<Velocity>(entity);
        if (!velocity_opt.has_value()) {
            continue;
        }
        auto& velocity = velocity_opt->get();
        // auto& images_opt = storage.get_component<Images>(entity);
        // if (!images_opt.has_value()) {
        //     continue;
        // }
        // auto& images = images_opt->get();
        // if (velocity.dx > 0) {
        //     direction.dx = 1;
        //     images.active_index = 1;
        // } else if (velocity.dx < 0) {
        //     direction.dx = -1;
        //     images.active_index = 0;
        // } 
        auto anim_opt = storage.get_component<Animation>(entity);
        if (!anim_opt.has_value()) {
            continue;
        }
        Animation& anim = anim_opt->get();
    }
}