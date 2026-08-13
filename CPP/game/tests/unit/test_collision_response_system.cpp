/**
 * Unit tests for CollisionResponseSystem
 *
 * These tests verify the CollisionResponseSystem correctly handles
 * bullet-asteroid and ship-asteroid collisions: scoring, destruction
 * requests, split candidates, ship reset, and game over.
 *
 * Requirements tested: 16.1–16.3, 16.7–16.8, 16.10
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/collision_response_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

// Helper: set up a minimal Blackboard with default asteroid config
static void setup_blackboard(Blackboard& bb) {
    bb.set<int>("asteroid.large.points", 20);
    bb.set<int>("asteroid.medium.points", 50);
    bb.set<int>("asteroid.small.points", 100);
    bb.set<int>("game.score", 0);
    bb.set<int>("game.lives", 3);
    bb.set<std::string>("game.state", std::string("PLAYING"));
}

// Helper: create an asteroid entity with the given tier
static Entity make_asteroid(EntityManager& em, ComponentStorage& storage, AsteroidTier tier) {
    Entity asteroid = em.create_entity();
    storage.add_component(asteroid, AsteroidTag{});
    storage.add_component(asteroid, Splittable{tier});
    storage.add_component(asteroid, Position{100.0f, 200.0f});
    storage.add_component(asteroid, Velocity{10.0f, 5.0f});
    return asteroid;
}

// Helper: create a bullet entity
static Entity make_bullet(EntityManager& em, ComponentStorage& storage) {
    Entity bullet = em.create_entity();
    storage.add_component(bullet, BulletTag{});
    storage.add_component(bullet, Position{50.0f, 50.0f});
    storage.add_component(bullet, Velocity{400.0f, 0.0f});
    return bullet;
}

// Helper: create a ship entity
static Entity make_ship(EntityManager& em, ComponentStorage& storage) {
    Entity ship = em.create_entity();
    storage.add_component(ship, ShipTag{});
    storage.add_component(ship, Position{200.0f, 150.0f});
    storage.add_component(ship, Velocity{30.0f, -20.0f});
    storage.add_component(ship, Rotation{1.5f, 0.8f});
    return ship;
}

TEST_CASE("CollisionResponseSystem", "[collision_response][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard blackboard;
    CollisionResponseSystem system;
    setup_blackboard(blackboard);

    SECTION("Bullet hits large asteroid — score +20, bullet DestroyRequest, asteroid SplitCandidate") {
        Entity bullet = make_bullet(em, storage);
        Entity asteroid = make_asteroid(em, storage, AsteroidTier::Large);

        // Set up collision: bullet collided with asteroid
        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, blackboard);

        REQUIRE(blackboard.get<int>("game.score") == 20);
        REQUIRE(storage.has_component<DestroyRequest>(bullet));
        REQUIRE(storage.has_component<SplitCandidate>(asteroid));
    }

    SECTION("Bullet hits medium asteroid — score +50") {
        Entity bullet = make_bullet(em, storage);
        Entity asteroid = make_asteroid(em, storage, AsteroidTier::Medium);

        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, blackboard);

        REQUIRE(blackboard.get<int>("game.score") == 50);
        REQUIRE(storage.has_component<DestroyRequest>(bullet));
        REQUIRE(storage.has_component<SplitCandidate>(asteroid));
    }

    SECTION("Bullet hits small asteroid — score +100") {
        Entity bullet = make_bullet(em, storage);
        Entity asteroid = make_asteroid(em, storage, AsteroidTier::Small);

        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, blackboard);

        REQUIRE(blackboard.get<int>("game.score") == 100);
        REQUIRE(storage.has_component<DestroyRequest>(bullet));
        REQUIRE(storage.has_component<SplitCandidate>(asteroid));
    }

    SECTION("Ship hits asteroid — lives -1, SplitCandidate, ship reset") {
        Entity ship = make_ship(em, storage);
        Entity asteroid = make_asteroid(em, storage, AsteroidTier::Large);

        storage.add_component(ship, CollidedWith{{asteroid}});

        system.update(storage, blackboard);

        REQUIRE(blackboard.get<int>("game.lives") == 2);
        REQUIRE(storage.has_component<SplitCandidate>(asteroid));

        // Ship position reset to (0, 0)
        auto& pos = storage.get_component<Position>(ship)->get();
        REQUIRE(pos.x == Catch::Approx(0.0f));
        REQUIRE(pos.y == Catch::Approx(0.0f));

        // Ship velocity reset to (0, 0)
        auto& vel = storage.get_component<Velocity>(ship)->get();
        REQUIRE(vel.dx == Catch::Approx(0.0f));
        REQUIRE(vel.dy == Catch::Approx(0.0f));

        // Ship rotation reset to angle=0, angular_velocity=0
        auto& rot = storage.get_component<Rotation>(ship)->get();
        REQUIRE(rot.angle == Catch::Approx(0.0f));
        REQUIRE(rot.angular_velocity == Catch::Approx(0.0f));
    }

    SECTION("Ship hits asteroid with 1 life — game over") {
        blackboard.set<int>("game.lives", 1);
        Entity ship = make_ship(em, storage);
        Entity asteroid = make_asteroid(em, storage, AsteroidTier::Large);

        storage.add_component(ship, CollidedWith{{asteroid}});

        system.update(storage, blackboard);

        REQUIRE(blackboard.get<int>("game.lives") == 0);
        REQUIRE(blackboard.get<std::string>("game.state") == "GAME_OVER");
    }

    SECTION("Bullet-asteroid collision — bullet gets DestroyRequest, asteroid does NOT") {
        Entity bullet = make_bullet(em, storage);
        Entity asteroid = make_asteroid(em, storage, AsteroidTier::Large);

        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, blackboard);

        REQUIRE(storage.has_component<DestroyRequest>(bullet));
        REQUIRE_FALSE(storage.has_component<DestroyRequest>(asteroid));
    }

    SECTION("No collisions — score and lives unchanged") {
        // Create entities but no CollidedWith
        make_bullet(em, storage);
        make_ship(em, storage);
        make_asteroid(em, storage, AsteroidTier::Large);

        system.update(storage, blackboard);

        REQUIRE(blackboard.get<int>("game.score") == 0);
        REQUIRE(blackboard.get<int>("game.lives") == 3);
    }
}
