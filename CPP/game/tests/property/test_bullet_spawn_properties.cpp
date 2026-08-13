/**
 * Property-based tests for BulletSpawnSystem
 *
 * These tests verify universal invariants of the BulletSpawnSystem across
 * random inputs: velocity direction, speed magnitude, cooldown monotonicity,
 * max live cap enforcement, and no-fire safety.
 *
 * Feature: 050-07-weaponize-the-game
 * Requirements tested: 3.7, 4.2, 4.5, 5.2, 5.3, 14.1–14.9
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include "game/bullet_spawn_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <cmath>

// Configurable test iteration counts
constexpr int NUM_OUTER_TESTS = 10;
constexpr int NUM_INNER_TESTS = 5;

// ============================================================================
// Feature: weaponize-the-game, Property 1: Bullet velocity direction parallel to facing
//
// For any ship angle and ship velocity, when a bullet is spawned, the bullet
// speed component (bullet_vel - ship_vel) shall be parallel to the facing
// direction (cos(angle), sin(angle)) — verified by cross product ≈ 0.
//
// **Validates: Requirements 3.7, 14.5**
// ============================================================================
TEST_CASE("Bullet velocity direction parallel to facing", "[bullet][property]") {
    SECTION("cross product of speed component and facing direction is near zero") {
        auto angle = GENERATE(take(NUM_OUTER_TESTS, random(-6.2832f, 6.2832f)));
        auto ship_dx = GENERATE(take(NUM_INNER_TESTS, random(-500.0f, 500.0f)));
        auto ship_dy = GENERATE(take(NUM_INNER_TESTS, random(-500.0f, 500.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        BulletSpawnSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("bullet.speed", 400.0f);
        bb.set<float>("bullet.lifetime", 2.0f);
        bb.set<float>("bullet.size", 4.0f);
        bb.set<int>("bullet.max_live", 6);
        bb.set<float>("bullet.fire_cooldown", 0.25f);
        bb.set<int>("bullet.layer", 2);
        bb.set<int>("bullet.mask", 4);
        bb.set<float>("ship.fire_cooldown_remaining", 0.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Position{0.0f, 0.0f});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{ship_dx, ship_dy});

        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;

        system.update(storage, bb, em);

        auto bullets = storage.entities_with_component<BulletTag>();
        REQUIRE(bullets.size() == 1);

        auto bvel = storage.get_component<Velocity>(bullets[0]);
        REQUIRE(bvel.has_value());

        float speed_x = bvel->get().dx - ship_dx;
        float speed_y = bvel->get().dy - ship_dy;
        // Ship texture points up, so facing direction is (-sin(angle), cos(angle))
        float face_x = -std::sin(angle);
        float face_y = std::cos(angle);

        // Cross product: speed_x * face_y - speed_y * face_x should be ~0
        float cross = speed_x * face_y - speed_y * face_x;
        REQUIRE(cross == Catch::Approx(0.0f).margin(1e-3f));
    }
}

// ============================================================================
// Feature: weaponize-the-game, Property 2: Bullet speed magnitude equals configured speed
//
// For any ship angle and ship velocity, when a bullet is spawned, the magnitude
// of the bullet speed component (bullet_vel - ship_vel) shall equal bullet.speed
// within tolerance 1e-3.
//
// **Validates: Requirements 3.7, 14.6**
// ============================================================================
TEST_CASE("Bullet speed magnitude equals configured speed", "[bullet][property]") {
    SECTION("|bullet_vel - ship_vel| approximately equals bullet.speed") {
        auto angle = GENERATE(take(NUM_OUTER_TESTS, random(-6.2832f, 6.2832f)));
        auto ship_dx = GENERATE(take(NUM_INNER_TESTS, random(-500.0f, 500.0f)));
        auto ship_dy = GENERATE(take(NUM_INNER_TESTS, random(-500.0f, 500.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        BulletSpawnSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("bullet.speed", 400.0f);
        bb.set<float>("bullet.lifetime", 2.0f);
        bb.set<float>("bullet.size", 4.0f);
        bb.set<int>("bullet.max_live", 6);
        bb.set<float>("bullet.fire_cooldown", 0.25f);
        bb.set<int>("bullet.layer", 2);
        bb.set<int>("bullet.mask", 4);
        bb.set<float>("ship.fire_cooldown_remaining", 0.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Position{0.0f, 0.0f});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{ship_dx, ship_dy});

        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;

        system.update(storage, bb, em);

        auto bullets = storage.entities_with_component<BulletTag>();
        REQUIRE(bullets.size() == 1);

        auto bvel = storage.get_component<Velocity>(bullets[0]);
        REQUIRE(bvel.has_value());

        float speed_x = bvel->get().dx - ship_dx;
        float speed_y = bvel->get().dy - ship_dy;
        float magnitude = std::sqrt(speed_x * speed_x + speed_y * speed_y);

        REQUIRE(magnitude == Catch::Approx(400.0f).margin(1e-3f));
    }
}

// ============================================================================
// Feature: weaponize-the-game, Property 3: Cooldown monotonically decreases
//
// For any sequence of frames with varying delta_time values, the fire cooldown
// remaining shall monotonically decrease (or stay at zero) between fire events,
// and shall never go below zero.
//
// **Validates: Requirements 4.2, 4.5, 14.7**
// ============================================================================
TEST_CASE("Fire cooldown monotonically decreases", "[bullet][property]") {
    SECTION("cooldown decreases or stays at zero, never negative") {
        auto initial_cooldown = GENERATE(take(NUM_OUTER_TESTS, random(0.01f, 2.0f)));
        auto dt = GENERATE(take(NUM_INNER_TESTS, random(0.001, 0.1)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        BulletSpawnSystem system;

        bb.set<double>("delta_time", dt);
        bb.set<float>("bullet.speed", 400.0f);
        bb.set<float>("bullet.lifetime", 2.0f);
        bb.set<float>("bullet.size", 4.0f);
        bb.set<int>("bullet.max_live", 6);
        bb.set<float>("bullet.fire_cooldown", 0.25f);
        bb.set<int>("bullet.layer", 2);
        bb.set<int>("bullet.mask", 4);
        bb.set<float>("ship.fire_cooldown_remaining", initial_cooldown);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Position{0.0f, 0.0f});
        storage.add_component(ship, Rotation{0.0f, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        // fire=false so no bullets spawn, just cooldown ticking
        float prev_cooldown = initial_cooldown;

        for (int i = 0; i < 5; ++i) {
            system.update(storage, bb, em);
            float curr_cooldown = bb.get<float>("ship.fire_cooldown_remaining");
            REQUIRE(curr_cooldown >= 0.0f);
            REQUIRE(curr_cooldown <= prev_cooldown);
            prev_cooldown = curr_cooldown;
        }
    }
}

// ============================================================================
// Feature: weaponize-the-game, Property 4: Live bullet count never exceeds max_live
//
// For rapid continuous fire sequences, the number of entities with BulletTag
// shall never exceed bullet.max_live.
//
// **Validates: Requirements 5.2, 5.3, 14.8**
// ============================================================================
TEST_CASE("Live bullet count never exceeds max_live", "[bullet][property]") {
    SECTION("BulletTag count <= max_live after each frame of continuous fire") {
        auto dt = GENERATE(take(NUM_OUTER_TESTS, random(0.001, 0.1)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        BulletSpawnSystem system;

        int max_live = 3;
        bb.set<double>("delta_time", dt);
        bb.set<float>("bullet.speed", 400.0f);
        bb.set<float>("bullet.lifetime", 2.0f);
        bb.set<float>("bullet.size", 4.0f);
        bb.set<int>("bullet.max_live", max_live);
        bb.set<float>("bullet.fire_cooldown", 0.0f);
        bb.set<int>("bullet.layer", 2);
        bb.set<int>("bullet.mask", 4);
        bb.set<float>("ship.fire_cooldown_remaining", 0.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Position{0.0f, 0.0f});
        storage.add_component(ship, Rotation{0.0f, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;

        for (int frame = 0; frame < 10; ++frame) {
            // Reset cooldown each frame to allow firing
            bb.set<float>("ship.fire_cooldown_remaining", 0.0f);
            system.update(storage, bb, em);

            auto bullet_count = storage.entities_with_component<BulletTag>().size();
            REQUIRE(static_cast<int>(bullet_count) <= max_live);
        }
    }
}

// ============================================================================
// Feature: weaponize-the-game, Property 5: No bullets spawn without fire input
//
// For any system state where fire is false on all Input entities, the number
// of entities with BulletTag shall remain unchanged after update.
//
// **Validates: Requirements 14.9**
// ============================================================================
TEST_CASE("No bullets spawn without fire input", "[bullet][property]") {
    SECTION("BulletTag count unchanged when fire is false") {
        auto existing_bullets = GENERATE(take(NUM_OUTER_TESTS, random(0, 5)));
        auto angle = GENERATE(take(NUM_INNER_TESTS, random(-6.2832f, 6.2832f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        BulletSpawnSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("bullet.speed", 400.0f);
        bb.set<float>("bullet.lifetime", 2.0f);
        bb.set<float>("bullet.size", 4.0f);
        bb.set<int>("bullet.max_live", 6);
        bb.set<float>("bullet.fire_cooldown", 0.25f);
        bb.set<int>("bullet.layer", 2);
        bb.set<int>("bullet.mask", 4);
        bb.set<float>("ship.fire_cooldown_remaining", 0.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Position{0.0f, 0.0f});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        // fire is false by default — no modification needed

        // Create some existing bullet entities
        for (int i = 0; i < existing_bullets; ++i) {
            Entity b = em.create_entity();
            storage.add_component(b, BulletTag{});
        }

        auto count_before = storage.entities_with_component<BulletTag>().size();

        system.update(storage, bb, em);

        auto count_after = storage.entities_with_component<BulletTag>().size();
        REQUIRE(count_after == count_before);
    }
}
