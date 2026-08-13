/**
 * Unit tests for ShipControlSystem
 *
 * These tests verify the ShipControlSystem correctly translates input
 * into rotation (angular_velocity) and thrust (velocity changes).
 *
 * Requirements tested: 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7, 12.8, 12.9
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/ship_control_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <cmath>

TEST_CASE("ShipControlSystem input handling", "[ship][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard blackboard;
    ShipControlSystem system;
    blackboard.set<double>("delta_time", 0.5);
    blackboard.set<float>("ship.rotation_speed", 4.0f);
    blackboard.set<float>("ship.thrust", 200.0f);

    Entity ship = em.create_entity();
    storage.add_component(ship, Input{});
    storage.add_component(ship, Rotation{0.0f, 0.0f});
    storage.add_component(ship, Velocity{0.0f, 0.0f});

    SECTION("LeftInput — Req 12.1") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.left = true;

        system.update(storage, blackboard);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angular_velocity == 4.0f);
    }

    SECTION("RightInput — Req 12.2") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.right = true;

        system.update(storage, blackboard);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angular_velocity == -4.0f);
    }

    SECTION("NoLeftRight — Req 12.3") {
        // No input flags set — default is all false
        system.update(storage, blackboard);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angular_velocity == 0.0f);
    }

    SECTION("BothLeftRight — Req 12.4") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.left = true;
        input.right = true;

        system.update(storage, blackboard);

        auto rot = storage.get_component<Rotation>(ship);
        REQUIRE(rot.has_value());
        REQUIRE(rot->get().angular_velocity == 0.0f);
    }

    SECTION("ThrustAtAngle0 — Req 12.5") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.up = true;
        // angle is already 0.0f
        // Direction: (-sin(0), cos(0)) = (0, 1) → thrust goes up (positive dy)

        system.update(storage, blackboard);

        auto vel = storage.get_component<Velocity>(ship);
        REQUIRE(vel.has_value());
        REQUIRE(vel->get().dx == Catch::Approx(0.0f).margin(1e-5));
        REQUIRE(vel->get().dy > 0.0f);
    }

    SECTION("ThrustAtAnglePiOver2 — Req 12.6") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.up = true;
        auto& rot = storage.get_component<Rotation>(ship)->get();
        rot.angle = static_cast<float>(M_PI / 2.0);
        // Direction: (-sin(π/2), cos(π/2)) = (-1, 0) → thrust goes left (negative dx)

        system.update(storage, blackboard);

        auto vel = storage.get_component<Velocity>(ship);
        REQUIRE(vel.has_value());
        REQUIRE(vel->get().dx < 0.0f);
        REQUIRE(vel->get().dy == Catch::Approx(0.0f).margin(1e-5));
    }

    SECTION("NoThrust — Req 12.7") {
        // input.up is false by default
        auto& vel = storage.get_component<Velocity>(ship)->get();
        vel.dx = 50.0f;
        vel.dy = 75.0f;

        system.update(storage, blackboard);

        auto vel_after = storage.get_component<Velocity>(ship);
        REQUIRE(vel_after.has_value());
        REQUIRE(vel_after->get().dx == 50.0f);
        REQUIRE(vel_after->get().dy == 75.0f);
    }

    SECTION("MissingRotation — Req 12.8") {
        // Create entity with Input but no Rotation
        Entity e2 = em.create_entity();
        storage.add_component(e2, Input{});
        storage.add_component(e2, Velocity{0.0f, 0.0f});

        REQUIRE_NOTHROW(system.update(storage, blackboard));
    }

    SECTION("EmptyStorage — Req 12.9") {
        // Remove all components from ship so no Input entities exist
        storage.remove_component<Input>(ship);
        storage.remove_component<Rotation>(ship);
        storage.remove_component<Velocity>(ship);

        REQUIRE_NOTHROW(system.update(storage, blackboard));
    }
}
