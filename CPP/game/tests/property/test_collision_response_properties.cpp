/**
 * Property-based tests for CollisionResponseSystem
 *
 * These tests verify universal invariants of the CollisionResponseSystem across
 * random inputs: scoring correctness, bullet destruction, asteroid protection,
 * split candidate marking, ship reset, and lives tracking.
 *
 * Feature: 050-08-lets-rock-the-game
 * Requirements tested: 5.3–5.6, 6.1–6.5, 6.7, 10.1, 17.5, 17.11, 17.12
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include "game/collision_response_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <cmath>

// Configurable test iteration counts
constexpr int NUM_OUTER_TESTS = 10;
constexpr int NUM_INNER_TESTS = 5;

// Helper: set up a minimal Blackboard with default asteroid config
static void setup_blackboard(Blackboard& bb) {
    bb.set<int>("asteroid.large.points", 20);
    bb.set<int>("asteroid.medium.points", 50);
    bb.set<int>("asteroid.small.points", 100);
    bb.set<int>("game.score", 0);
    bb.set<int>("game.lives", 3);
    bb.set<std::string>("game.state", std::string("PLAYING"));
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 1: Scoring correctness per tier
//
// For any asteroid tier and any initial score, when a bullet collides with an
// asteroid of that tier, the score increment equals asteroid.<tier>.points.
//
// **Validates: Requirements 5.5, 10.1, 17.5**
// ============================================================================
TEST_CASE("Scoring correctness per tier", "[collision_response][property]") {
    SECTION("score increment equals configured points for random initial scores") {
        auto initial_score = GENERATE(take(NUM_OUTER_TESTS, random(0, 10000)));
        auto tier_index = GENERATE(take(NUM_INNER_TESTS, random(0, 2)));

        AsteroidTier tier;
        int expected_points;
        switch (tier_index) {
            case 0: tier = AsteroidTier::Large;  expected_points = 20;  break;
            case 1: tier = AsteroidTier::Medium; expected_points = 50;  break;
            default: tier = AsteroidTier::Small; expected_points = 100; break;
        }

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);
        bb.set<int>("game.score", initial_score);

        Entity bullet = em.create_entity();
        storage.add_component(bullet, BulletTag{});
        storage.add_component(bullet, Position{0.0f, 0.0f});
        storage.add_component(bullet, Velocity{400.0f, 0.0f});

        Entity asteroid = em.create_entity();
        storage.add_component(asteroid, AsteroidTag{});
        storage.add_component(asteroid, Splittable{tier});
        storage.add_component(asteroid, Position{100.0f, 100.0f});

        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, bb);

        int new_score = bb.get<int>("game.score");
        REQUIRE(new_score == initial_score + expected_points);
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 2: Bullet receives DestroyRequest on asteroid collision
//
// For any bullet with CollidedWith containing an AsteroidTag entity, after
// update the bullet has DestroyRequest.
//
// **Validates: Requirements 5.3, 16.10**
// ============================================================================
TEST_CASE("Bullet receives DestroyRequest on asteroid collision", "[collision_response][property]") {
    SECTION("bullet always gets DestroyRequest when colliding with asteroid") {
        auto bullet_x = GENERATE(take(NUM_OUTER_TESTS, random(-400.0f, 400.0f)));
        auto bullet_y = GENERATE(take(NUM_INNER_TESTS, random(-300.0f, 300.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);

        Entity bullet = em.create_entity();
        storage.add_component(bullet, BulletTag{});
        storage.add_component(bullet, Position{bullet_x, bullet_y});
        storage.add_component(bullet, Velocity{400.0f, 0.0f});

        Entity asteroid = em.create_entity();
        storage.add_component(asteroid, AsteroidTag{});
        storage.add_component(asteroid, Splittable{AsteroidTier::Large});
        storage.add_component(asteroid, Position{100.0f, 100.0f});

        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, bb);

        REQUIRE(storage.has_component<DestroyRequest>(bullet));
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 3: CollisionResponseSystem never attaches DestroyRequest to asteroids
//
// For any collision scenario (bullet-asteroid or ship-asteroid), no AsteroidTag
// entity has DestroyRequest after update.
//
// **Validates: Requirements 5.6, 6.7**
// ============================================================================
TEST_CASE("CollisionResponseSystem never attaches DestroyRequest to asteroids", "[collision_response][property]") {
    SECTION("bullet-asteroid collision: asteroid never gets DestroyRequest") {
        auto tier_index = GENERATE(take(NUM_OUTER_TESTS, random(0, 2)));

        AsteroidTier tier;
        switch (tier_index) {
            case 0: tier = AsteroidTier::Large;  break;
            case 1: tier = AsteroidTier::Medium; break;
            default: tier = AsteroidTier::Small; break;
        }

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);

        Entity bullet = em.create_entity();
        storage.add_component(bullet, BulletTag{});
        storage.add_component(bullet, Position{0.0f, 0.0f});
        storage.add_component(bullet, Velocity{400.0f, 0.0f});

        Entity asteroid = em.create_entity();
        storage.add_component(asteroid, AsteroidTag{});
        storage.add_component(asteroid, Splittable{tier});
        storage.add_component(asteroid, Position{100.0f, 100.0f});

        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, bb);

        REQUIRE_FALSE(storage.has_component<DestroyRequest>(asteroid));
    }

    SECTION("ship-asteroid collision: asteroid never gets DestroyRequest") {
        auto tier_index = GENERATE(take(NUM_OUTER_TESTS, random(0, 2)));

        AsteroidTier tier;
        switch (tier_index) {
            case 0: tier = AsteroidTier::Large;  break;
            case 1: tier = AsteroidTier::Medium; break;
            default: tier = AsteroidTier::Small; break;
        }

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);

        Entity ship = em.create_entity();
        storage.add_component(ship, ShipTag{});
        storage.add_component(ship, Position{200.0f, 150.0f});
        storage.add_component(ship, Velocity{30.0f, -20.0f});
        storage.add_component(ship, Rotation{1.5f, 0.8f});

        Entity asteroid = em.create_entity();
        storage.add_component(asteroid, AsteroidTag{});
        storage.add_component(asteroid, Splittable{tier});
        storage.add_component(asteroid, Position{100.0f, 100.0f});

        storage.add_component(ship, CollidedWith{{asteroid}});

        system.update(storage, bb);

        REQUIRE_FALSE(storage.has_component<DestroyRequest>(asteroid));
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 4: All asteroid collisions produce SplitCandidate
//
// For any collision involving an asteroid (bullet or ship), the asteroid has
// SplitCandidate after update.
//
// **Validates: Requirements 5.4, 6.1, 17.12**
// ============================================================================
TEST_CASE("All asteroid collisions produce SplitCandidate", "[collision_response][property]") {
    SECTION("bullet-asteroid: asteroid always gets SplitCandidate") {
        auto tier_index = GENERATE(take(NUM_OUTER_TESTS, random(0, 2)));

        AsteroidTier tier;
        switch (tier_index) {
            case 0: tier = AsteroidTier::Large;  break;
            case 1: tier = AsteroidTier::Medium; break;
            default: tier = AsteroidTier::Small; break;
        }

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);

        Entity bullet = em.create_entity();
        storage.add_component(bullet, BulletTag{});
        storage.add_component(bullet, Position{0.0f, 0.0f});
        storage.add_component(bullet, Velocity{400.0f, 0.0f});

        Entity asteroid = em.create_entity();
        storage.add_component(asteroid, AsteroidTag{});
        storage.add_component(asteroid, Splittable{tier});
        storage.add_component(asteroid, Position{100.0f, 100.0f});

        storage.add_component(bullet, CollidedWith{{asteroid}});

        system.update(storage, bb);

        REQUIRE(storage.has_component<SplitCandidate>(asteroid));
    }

    SECTION("ship-asteroid: asteroid always gets SplitCandidate") {
        auto tier_index = GENERATE(take(NUM_OUTER_TESTS, random(0, 2)));

        AsteroidTier tier;
        switch (tier_index) {
            case 0: tier = AsteroidTier::Large;  break;
            case 1: tier = AsteroidTier::Medium; break;
            default: tier = AsteroidTier::Small; break;
        }

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);

        Entity ship = em.create_entity();
        storage.add_component(ship, ShipTag{});
        storage.add_component(ship, Position{200.0f, 150.0f});
        storage.add_component(ship, Velocity{30.0f, -20.0f});
        storage.add_component(ship, Rotation{1.5f, 0.8f});

        Entity asteroid = em.create_entity();
        storage.add_component(asteroid, AsteroidTag{});
        storage.add_component(asteroid, Splittable{tier});
        storage.add_component(asteroid, Position{100.0f, 100.0f});

        storage.add_component(ship, CollidedWith{{asteroid}});

        system.update(storage, bb);

        REQUIRE(storage.has_component<SplitCandidate>(asteroid));
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 5: Ship state fully reset after collision
//
// For any ship position/velocity/rotation before collision, after update:
// Position=(0,0), Velocity=(0,0), Rotation angle=0 angular_velocity=0.
//
// **Validates: Requirements 6.3, 6.4, 6.5**
// ============================================================================
TEST_CASE("Ship state fully reset after collision", "[collision_response][property]") {
    SECTION("ship position, velocity, rotation all reset to zero") {
        auto ship_x = GENERATE(take(NUM_OUTER_TESTS, random(-400.0f, 400.0f)));
        auto ship_y = GENERATE(take(NUM_INNER_TESTS, random(-300.0f, 300.0f)));
        auto ship_dx = GENERATE(take(NUM_INNER_TESTS, random(-500.0f, 500.0f)));
        auto ship_dy = GENERATE(take(NUM_INNER_TESTS, random(-500.0f, 500.0f)));
        auto ship_angle = GENERATE(take(NUM_INNER_TESTS, random(0.0f, 6.2832f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);

        Entity ship = em.create_entity();
        storage.add_component(ship, ShipTag{});
        storage.add_component(ship, Position{ship_x, ship_y});
        storage.add_component(ship, Velocity{ship_dx, ship_dy});
        storage.add_component(ship, Rotation{ship_angle, 0.8f});

        Entity asteroid = em.create_entity();
        storage.add_component(asteroid, AsteroidTag{});
        storage.add_component(asteroid, Splittable{AsteroidTier::Large});
        storage.add_component(asteroid, Position{100.0f, 100.0f});

        storage.add_component(ship, CollidedWith{{asteroid}});

        system.update(storage, bb);

        auto& pos = storage.get_component<Position>(ship)->get();
        REQUIRE(pos.x == Catch::Approx(0.0f));
        REQUIRE(pos.y == Catch::Approx(0.0f));

        auto& vel = storage.get_component<Velocity>(ship)->get();
        REQUIRE(vel.dx == Catch::Approx(0.0f));
        REQUIRE(vel.dy == Catch::Approx(0.0f));

        auto& rot = storage.get_component<Rotation>(ship)->get();
        REQUIRE(rot.angle == Catch::Approx(0.0f));
        REQUIRE(rot.angular_velocity == Catch::Approx(0.0f));
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 6: Lives monotonically decrease and never go below zero
//
// For any sequence of ship-asteroid collisions from any positive initial lives,
// lives decreases by 1 per collision and never goes negative.
//
// **Validates: Requirements 6.2, 17.11**
// ============================================================================
TEST_CASE("Lives monotonically decrease and never go below zero", "[collision_response][property]") {
    SECTION("lives decrease by 1 per collision, never negative") {
        auto initial_lives = GENERATE(take(NUM_OUTER_TESTS, random(1, 10)));
        auto num_collisions = GENERATE(take(NUM_INNER_TESTS, random(1, 15)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        CollisionResponseSystem system;
        setup_blackboard(bb);
        bb.set<int>("game.lives", initial_lives);

        Entity ship = em.create_entity();
        storage.add_component(ship, ShipTag{});
        storage.add_component(ship, Position{200.0f, 150.0f});
        storage.add_component(ship, Velocity{30.0f, -20.0f});
        storage.add_component(ship, Rotation{1.5f, 0.8f});

        int prev_lives = initial_lives;

        for (int i = 0; i < num_collisions; ++i) {
            // Skip if game is already over
            std::string state = bb.get_or<std::string>("game.state", std::string("PLAYING"));
            if (state != "PLAYING") break;

            Entity asteroid = em.create_entity();
            storage.add_component(asteroid, AsteroidTag{});
            storage.add_component(asteroid, Splittable{AsteroidTier::Large});
            storage.add_component(asteroid, Position{100.0f, 100.0f});

            // Remove old CollidedWith if present, add new one
            storage.remove_component<CollidedWith>(ship);
            storage.add_component(ship, CollidedWith{{asteroid}});

            system.update(storage, bb);

            int curr_lives = bb.get<int>("game.lives");
            REQUIRE(curr_lives >= 0);
            REQUIRE(curr_lives == prev_lives - 1);
            prev_lives = curr_lives;
        }
    }
}
