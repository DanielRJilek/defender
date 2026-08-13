/**
 * Property-based tests for ShipControlSystem
 *
 * These tests verify universal invariants of the ShipControlSystem across
 * random inputs: rotation direction, angle preservation, thrust direction,
 * velocity preservation, and thrust magnitude.
 *
 * Feature: 050-06-ship-controls
 * Requirements tested: 1.1, 1.2, 1.3, 1.4, 1.5, 2.1, 2.2, 2.3, 2.6,
 *                      10.1, 10.2, 10.3, 13.1–13.9
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include "game/ship_control_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <cmath>

// Configurable test iteration counts
constexpr int NUM_OUTER_TESTS = 10;
constexpr int NUM_INNER_TESTS = 5;

// ============================================================================
// Feature: 050-06-ship-controls, Property 1: Left input produces positive angular velocity
//
// For any positive rotation_speed, when left=true and right=false,
// angular_velocity shall equal +rotation_speed.
//
// **Validates: Requirements 1.1, 13.8**
// ============================================================================
TEST_CASE("Left input produces positive angular velocity", "[ship][property]") {
    SECTION("angular_velocity equals +rotation_speed for left-only input") {
        auto rotation_speed = GENERATE(take(NUM_OUTER_TESTS, random(0.1f, 20.0f)));
        auto angle = GENERATE(take(NUM_INNER_TESTS, random(-6.2832f, 6.2832f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        ShipControlSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("ship.rotation_speed", rotation_speed);
        bb.set<float>("ship.thrust", 200.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        auto& input = storage.get_component<Input>(ship)->get();
        input.left = true;
        input.right = false;

        system.update(storage, bb);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angular_velocity == rotation_speed);
    }
}

// ============================================================================
// Feature: 050-06-ship-controls, Property 2: Right input produces negative angular velocity
//
// For any positive rotation_speed, when right=true and left=false,
// angular_velocity shall equal -rotation_speed.
//
// **Validates: Requirements 1.2, 13.9**
// ============================================================================
TEST_CASE("Right input produces negative angular velocity", "[ship][property]") {
    SECTION("angular_velocity equals -rotation_speed for right-only input") {
        auto rotation_speed = GENERATE(take(NUM_OUTER_TESTS, random(0.1f, 20.0f)));
        auto angle = GENERATE(take(NUM_INNER_TESTS, random(-6.2832f, 6.2832f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        ShipControlSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("ship.rotation_speed", rotation_speed);
        bb.set<float>("ship.thrust", 200.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        auto& input = storage.get_component<Input>(ship)->get();
        input.left = false;
        input.right = true;

        system.update(storage, bb);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angular_velocity == -rotation_speed);
    }
}

// ============================================================================
// Feature: 050-06-ship-controls, Property 3: Equal left/right input produces zero angular velocity
//
// When left and right are equal (both true or both false),
// angular_velocity shall be zero.
//
// **Validates: Requirements 1.3, 1.4**
// ============================================================================
TEST_CASE("Equal left/right input produces zero angular velocity", "[ship][property]") {
    SECTION("angular_velocity is zero when left == right") {
        auto both_val = GENERATE(take(NUM_OUTER_TESTS, random(0, 1)));
        auto rotation_speed = GENERATE(take(NUM_INNER_TESTS, random(0.1f, 20.0f)));

        bool both = static_cast<bool>(both_val);

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        ShipControlSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("ship.rotation_speed", rotation_speed);
        bb.set<float>("ship.thrust", 200.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Rotation{0.0f, 99.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        auto& input = storage.get_component<Input>(ship)->get();
        input.left = both;
        input.right = both;

        system.update(storage, bb);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angular_velocity == 0.0f);
    }
}

// ============================================================================
// Feature: 050-06-ship-controls, Property 4: ShipControlSystem preserves angle
//
// For any initial angle, after one update the angle field shall be unchanged.
//
// **Validates: Requirements 1.5**
// ============================================================================
TEST_CASE("ShipControlSystem preserves angle", "[ship][property]") {
    SECTION("angle is unchanged after update") {
        auto angle = GENERATE(take(NUM_OUTER_TESTS, random(-6.2832f, 6.2832f)));
        auto left_val = GENERATE(take(NUM_INNER_TESTS, random(0, 1)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        ShipControlSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("ship.rotation_speed", 4.0f);
        bb.set<float>("ship.thrust", 200.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        auto& input = storage.get_component<Input>(ship)->get();
        input.left = static_cast<bool>(left_val);
        input.up = true;

        system.update(storage, bb);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angle == angle);
    }
}

// ============================================================================
// Feature: 050-06-ship-controls, Property 5: Thrust direction is parallel to facing direction
//
// When up=true, the velocity change vector shall be parallel to
// (cos(angle), sin(angle)) — verified by cross product near zero.
//
// **Validates: Requirements 2.1, 2.2, 2.6, 13.6**
// ============================================================================
TEST_CASE("Thrust direction is parallel to facing direction", "[ship][property]") {
    SECTION("cross product of velocity change and facing direction is near zero") {
        auto angle = GENERATE(take(NUM_OUTER_TESTS, random(-6.2832f, 6.2832f)));
        auto thrust = GENERATE(take(NUM_INNER_TESTS, random(10.0f, 1000.0f)));
        auto dt = GENERATE(take(NUM_INNER_TESTS, random(0.001, 0.1)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        ShipControlSystem system;

        bb.set<double>("delta_time", dt);
        bb.set<float>("ship.rotation_speed", 4.0f);
        bb.set<float>("ship.thrust", thrust);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        auto& input = storage.get_component<Input>(ship)->get();
        input.up = true;

        system.update(storage, bb);

        auto vel = storage.get_component<Velocity>(ship);
        REQUIRE(vel.has_value());

        float ddx = vel->get().dx;
        float ddy = vel->get().dy;
        float face_x = -std::sin(angle);
        float face_y = std::cos(angle);

        // Cross product: ddx * face_y - ddy * face_x should be ~0
        float cross = ddx * face_y - ddy * face_x;
        REQUIRE(cross == Catch::Approx(0.0f).margin(1e-3f));
    }
}

// ============================================================================
// Feature: 050-06-ship-controls, Property 6: Zero input preserves velocity
//
// When up=false, left=false, right=false, velocity shall be unchanged.
//
// **Validates: Requirements 2.3, 10.1, 10.2, 10.3, 13.7**
// ============================================================================
TEST_CASE("Zero input preserves velocity", "[ship][property]") {
    SECTION("velocity unchanged when all input flags are false") {
        auto init_dx = GENERATE(take(NUM_OUTER_TESTS, random(-500.0f, 500.0f)));
        auto init_dy = GENERATE(take(NUM_INNER_TESTS, random(-500.0f, 500.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        ShipControlSystem system;

        bb.set<double>("delta_time", 0.016);
        bb.set<float>("ship.rotation_speed", 4.0f);
        bb.set<float>("ship.thrust", 200.0f);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Rotation{0.0f, 0.0f});
        storage.add_component(ship, Velocity{init_dx, init_dy});

        // All input flags default to false — no modification needed

        system.update(storage, bb);

        auto vel = storage.get_component<Velocity>(ship);
        REQUIRE(vel.has_value());
        REQUIRE(vel->get().dx == init_dx);
        REQUIRE(vel->get().dy == init_dy);
    }
}

// ============================================================================
// Feature: 050-06-ship-controls, Property 7: Thrust magnitude equals thrust * dt
//
// When up=true, the magnitude of velocity change shall equal thrust * dt.
//
// **Validates: Requirements 13.5**
// ============================================================================
TEST_CASE("Thrust magnitude equals thrust times dt", "[ship][property]") {
    SECTION("|velocity_change| approximately equals thrust * dt") {
        auto angle = GENERATE(take(NUM_OUTER_TESTS, random(-6.2832f, 6.2832f)));
        auto thrust = GENERATE(take(NUM_INNER_TESTS, random(10.0f, 1000.0f)));
        auto dt = GENERATE(take(NUM_INNER_TESTS, random(0.001, 0.1)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        ShipControlSystem system;

        bb.set<double>("delta_time", dt);
        bb.set<float>("ship.rotation_speed", 4.0f);
        bb.set<float>("ship.thrust", thrust);

        Entity ship = em.create_entity();
        storage.add_component(ship, Input{});
        storage.add_component(ship, Rotation{angle, 0.0f});
        storage.add_component(ship, Velocity{0.0f, 0.0f});

        auto& input = storage.get_component<Input>(ship)->get();
        input.up = true;

        system.update(storage, bb);

        auto vel = storage.get_component<Velocity>(ship);
        REQUIRE(vel.has_value());

        float ddx = vel->get().dx;
        float ddy = vel->get().dy;
        float magnitude = std::sqrt(ddx * ddx + ddy * ddy);
        float expected = thrust * static_cast<float>(dt);

        REQUIRE(magnitude == Catch::Approx(expected).margin(1e-3f));
    }
}
