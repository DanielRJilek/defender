/**
 * ShipControlSystem - Game-level system for Asteroids ship input handling
 *
 * Translates arrow key input into rotation and thrust for the player ship.
 * Left/Right arrows set angular_velocity on the Rotation component (positive = CCW,
 * negative = CW). Up arrow applies acceleration in the ship's facing direction
 * to the Velocity component using cos(angle) and sin(angle).
 *
 * This system does NOT modify Rotation.angle directly — the RotationSystem
 * (engine-level) handles angle += angular_velocity * dt.
 *
 * Reads: Input, Rotation.angle, Blackboard (delta_time, ship.rotation_speed, ship.thrust)
 * Writes: Rotation.angular_velocity, Velocity.dx, Velocity.dy
 */

#ifndef SHIP_CONTROL_SYSTEM_HPP
#define SHIP_CONTROL_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

class ShipControlSystem {
public:
    void update(ComponentStorage& storage, Blackboard& blackboard);
};

#endif // SHIP_CONTROL_SYSTEM_HPP
