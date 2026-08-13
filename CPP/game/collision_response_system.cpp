#include "game/collision_response_system.hpp"
#include <string>

void CollisionResponseSystem::update(ComponentStorage& storage, Blackboard& blackboard) {
    // Return early if game is not in PLAYING state
    std::string state = blackboard.get_or<std::string>("game.state", std::string("PLAYING"));
    if (state != "PLAYING") {
        return;
    }

    // Read point values from Blackboard with defaults
    int lander_points = blackboard.get_or<int>("alien.lander.points", 150);
    int swarmer_points = blackboard.get_or<int>("alien.swarmer.points", 150);
    int baiter_points = blackboard.get_or<int>("alien.baiter.points", 200);

    // Iterate all entities that have a CollidedWith component
    auto collided_entities = storage.entities_with_component<CollidedWith>();

    for (Entity entity : collided_entities) {
        auto& collided_with = storage.get_component<CollidedWith>(entity)->get();

        // --- Laser-Alien collision ---
        if (storage.has_component<LaserTag>(entity)) {
            for (Entity other : collided_with.entities) {
                if (storage.has_component<AlienTag>(other)) {
                    // Mark laser for destruction
                    storage.add_component(entity, DestroyRequest{});

                    // Mark alien as destroyRequest
                    storage.add_component(other, DestroyRequest{});

                    // Score points based on alien type
                    if (storage.has_component<AlienType>(other)) {
                        auto& alien_type = storage.get_component<AlienType>(other)->get();

                        int points = 0;
                        if (alien_type == AlienType::lander)       points = lander_points;
                        else if (alien_type == AlienType::swarmer) points = swarmer_points;
                        else if (alien_type == AlienType::baiter)  points = baiter_points;

                        int score = blackboard.get_or<int>("game.score", 0);
                        blackboard.set("game.score", score + points);
                    }
                }
            }
        }

        // --- Ship-Alien collision ---
        if (storage.has_component<ShipTag>(entity)) {
            for (Entity other : collided_with.entities) {
                if (storage.has_component<AlienTag>(other)) {

                    // Decrement lives
                    int lives = blackboard.get_or<int>("game.lives", 3);
                    lives -= 1;
                    if (lives < 0) lives = 0;
                    blackboard.set("game.lives", lives);

                    // Reset ship Position to center (0, 0)
                    if (storage.has_component<Position>(entity)) {
                        auto& pos = storage.get_component<Position>(entity)->get();
                        pos.x = 0.0f;
                        pos.y = 0.0f;
                    }

                    // Reset ship Velocity to (0, 0)
                    if (storage.has_component<Velocity>(entity)) {
                        auto& vel = storage.get_component<Velocity>(entity)->get();
                        vel.dx = 300.0f;
                        vel.dy = 0.0f;
                    }

                    // Game over if lives reach 0
                    if (lives == 0) {
                        blackboard.set<std::string>("game.state", "GAME_OVER");
                    }
                    // otherwise, game should restart wave
                    else {
                        blackboard.set<std::string>("game.state", "LEVEL_COMPLETE");
                        blackboard.set<int>("game.wave", 0);
                        blackboard.set<float>("game.spawn_delay_timer", 0.0f);
                        blackboard.set<int>("game.score", blackboard.get<int>("game.saved_score"));
                    }
                }
            }
        }

        // --- Ship-Bullet collision ---
        if (storage.has_component<ShipTag>(entity)) {
            for (Entity other : collided_with.entities) {
                if (storage.has_component<BulletTag>(other)) {

                    // Decrement lives
                    int lives = blackboard.get_or<int>("game.lives", 3);
                    lives -= 1;
                    if (lives < 0) lives = 0;
                    blackboard.set("game.lives", lives);

                    // Reset ship Position to center (0, 0)
                    if (storage.has_component<Position>(entity)) {
                        auto& pos = storage.get_component<Position>(entity)->get();
                        pos.x = 0.0f;
                        pos.y = 0.0f;
                    }

                    // Reset ship Velocity to (0, 0)
                    if (storage.has_component<Velocity>(entity)) {
                        auto& vel = storage.get_component<Velocity>(entity)->get();
                        vel.dx = 300.0f;
                        vel.dy = 0.0f;
                    }

                    // Game over if lives reach 0
                    if (lives == 0) {
                        blackboard.set<std::string>("game.state", "GAME_OVER");
                    }
                    // otherwise, game should restart wave
                    else {
                        blackboard.set<std::string>("game.state", "LEVEL_COMPLETE");
                        blackboard.set<int>("game.wave", 0);
                        blackboard.set<float>("game.spawn_delay_timer", 0.0f);
                        blackboard.set<int>("game.score", blackboard.get<int>("game.saved_score"));
                    }
                }
            }
        }

        // --- laser-astronaut collision ---
        if (storage.has_component<LaserTag>(entity)) {
            for (Entity other : collided_with.entities) {
                if (storage.has_component<AstronautTag>(other)) {
                    // Mark laser for destruction
                    storage.add_component(entity, DestroyRequest{});

                    // Mark astronaut as destroyRequest
                    storage.add_component(other, DestroyRequest{});

                }
            }
        }

        // --- alien-astronaut collision ---
        if (storage.has_component<AlienTag>(entity) && 
                storage.has_component<AlienType>(entity) && 
                storage.get_component<AlienType>(entity)->get() == AlienType::lander) {
            for (Entity other : collided_with.entities) {
                if (storage.has_component<AstronautTag>(other)) {
                    storage.add_component(entity, AbductingTag{(other)});
                    storage.add_component(other, AbductedTag{(entity)});
                }
            }
        }

        // --- ship-astronaut collision ---
        if (storage.has_component<ShipTag>(entity)) {
            for (Entity other : collided_with.entities) {
                if (storage.has_component<AstronautTag>(other)) {
                    if (storage.has_component<FallingTag>(other)) {
                        storage.remove_component<FallingTag>(other);
                        storage.add_component(other, RescuedTag{});
                    }
                }
            }
        }
    }
}
