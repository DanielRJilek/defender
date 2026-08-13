#include "game/ship_control_system.hpp"
#include <cmath>

void ShipControlSystem::update(ComponentStorage& storage, Blackboard& blackboard) {
    double delta_time = blackboard.get<double>("delta_time");
    float dt = static_cast<float>(delta_time);
    float thrust = blackboard.get_or<float>("ship.thrust", 200.0f);

    auto entities = storage.entities_with_component<Input>();
    for (Entity entity : entities) {
        if (!storage.has_component<Velocity>(entity)) continue;

        auto& input = storage.get_component<Input>(entity)->get();
        auto& velocity = storage.get_component<Velocity>(entity)->get();
        if (!input.right && !input.left) {
            if (velocity.dx > 0) {
                velocity.dx = thrust;
            }
            else if (velocity.dx < 0) {
                velocity.dx = -thrust;
            }
        }
        if (input.left) {
            if (velocity.dx >= 0) {
                velocity.dx = -thrust;
            }
            else if (velocity.dx == -thrust) {
                velocity.dx = -thrust * 1.5f;
            }
        }
        if (input.right) {
            if (velocity.dx <= 0) {
                velocity.dx = thrust;
            }
            else if (velocity.dx == thrust) {
                velocity.dx = thrust * 1.5f;
            }
        }
        if (input.up) {
            velocity.dy = thrust;
        }
        if (input.down) {
            velocity.dy = -thrust;
        }
        if (!input.up && !input.down) {
            velocity.dy = 0;
        }
        if (input.hyperspace) {
            blackboard.set<std::string>("game.hyperspace_state", "REQUESTED");
        }
    }
}
