#ifndef COLLISION_RESPONSE_SYSTEM_HPP
#define COLLISION_RESPONSE_SYSTEM_HPP

#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

/**
 * CollisionResponseSystem - Game-level system for collision consequences
 *
 * Reads per-entity CollidedWith components (written by the engine-level
 * CollisionSystem) and interprets what collided with what using tag
 * components (BulletTag, AlienTag, ShipTag).
 *
 * Bullet hits asteroid: DestroyRequest on bullet, SplitCandidate on asteroid, score points.
 * Ship hits asteroid: SplitCandidate on asteroid, decrement lives, reset ship, game over check.
 *
 * NEVER attaches DestroyRequest to asteroids — only SplitCandidate.
 * AsteroidSpawnSystem is the sole authority for destroying asteroids.
 *
 * Reads: CollidedWith, BulletTag, AsteroidTag, ShipTag, Splittable,
 *        Blackboard (game.state, asteroid.<tier>.points)
 * Writes: DestroyRequest (bullets only), SplitCandidate (asteroids),
 *         Blackboard (game.score, game.lives, game.state),
 *         Position, Velocity, Rotation (ship reset)
 */
class CollisionResponseSystem {
public:
    void update(ComponentStorage& storage, Blackboard& blackboard);
};

#endif // COLLISION_RESPONSE_SYSTEM_HPP
