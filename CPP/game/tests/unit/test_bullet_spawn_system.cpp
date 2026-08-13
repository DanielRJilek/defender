/**
 * Unit tests for BulletSpawnSystem
 *
 * These tests verify the BulletSpawnSystem correctly spawns bullets with
 * the right components and respects cooldown and max live cap.
 *
 * Requirements tested: 13.1, 13.2, 13.3, 13.4, 13.5, 13.6, 13.7, 13.8, 13.9
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/bullet_spawn_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <cmath>

TEST_CASE("BulletSpawnSystem spawning", "[bullet][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard blackboard;
    BulletSpawnSystem system;
    blackboard.set<double>("delta_time", 0.016);
    blackboard.set<float>("bullet.speed", 400.0f);
    blackboard.set<float>("bullet.lifetime", 2.0f);
    blackboard.set<float>("bullet.size", 4.0f);
    blackboard.set<int>("bullet.max_live", 6);
    blackboard.set<float>("bullet.fire_cooldown", 0.25f);
    blackboard.set<int>("bullet.layer", 2);
    blackboard.set<int>("bullet.mask", 4);
    blackboard.set<float>("ship.fire_cooldown_remaining", 0.0f);

    // Create ship entity
    Entity ship = em.create_entity();
    storage.add_component(ship, Input{});
    storage.add_component(ship, Position{0.0f, 0.0f});
    storage.add_component(ship, Rotation{0.0f, 0.0f});
    storage.add_component(ship, Velocity{0.0f, 0.0f});

    SECTION("SuccessfulSpawn — Req 13.1") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;

        system.update(storage, blackboard, em);

        // Find the bullet entity (not the ship)
        auto bullets = storage.entities_with_component<BulletTag>();
        REQUIRE(bullets.size() == 1);
        Entity bullet = bullets[0];

        REQUIRE(storage.has_component<BulletTag>(bullet));
        REQUIRE(storage.has_component<Position>(bullet));
        REQUIRE(storage.has_component<Size>(bullet));
        REQUIRE(storage.has_component<Velocity>(bullet));
        REQUIRE(storage.has_component<Lifetime>(bullet));
        REQUIRE(storage.has_component<WrapAround>(bullet));
        REQUIRE(storage.has_component<Collider>(bullet));
        REQUIRE(storage.has_component<Color>(bullet));
    }

    SECTION("NoFire — Req 13.2") {
        // fire is false by default
        auto count_before = storage.entities_with_component<BulletTag>().size();

        system.update(storage, blackboard, em);

        auto count_after = storage.entities_with_component<BulletTag>().size();
        REQUIRE(count_after == count_before);
    }

    SECTION("CooldownActive — Req 13.3") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;
        blackboard.set<float>("ship.fire_cooldown_remaining", 0.2f);

        auto count_before = storage.entities_with_component<BulletTag>().size();

        system.update(storage, blackboard, em);

        auto count_after = storage.entities_with_component<BulletTag>().size();
        REQUIRE(count_after == count_before);
    }

    SECTION("MaxLiveCap — Req 13.4") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;

        // Create 6 existing bullet entities
        for (int i = 0; i < 6; ++i) {
            Entity b = em.create_entity();
            storage.add_component(b, BulletTag{});
        }

        auto count_before = storage.entities_with_component<BulletTag>().size();
        REQUIRE(count_before == 6);

        system.update(storage, blackboard, em);

        auto count_after = storage.entities_with_component<BulletTag>().size();
        REQUIRE(count_after == count_before);
    }

    SECTION("Angle0Velocity — Req 13.5") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;
        // Direction at angle 0: (-sin(0), cos(0)) = (0, 400)

        system.update(storage, blackboard, em);

        auto bullets = storage.entities_with_component<BulletTag>();
        REQUIRE(bullets.size() == 1);
        auto vel = storage.get_component<Velocity>(bullets[0]);
        REQUIRE(vel.has_value());
        REQUIRE(vel->get().dx == Catch::Approx(0.0f).margin(1e-3f));
        REQUIRE(vel->get().dy == Catch::Approx(400.0f).margin(1e-3f));
    }

    SECTION("AnglePiOver2Velocity — Req 13.6") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;
        auto& rot = storage.get_component<Rotation>(ship)->get();
        rot.angle = static_cast<float>(M_PI / 2.0);
        // Direction at angle π/2: (-sin(π/2), cos(π/2)) = (-400, 0)

        system.update(storage, blackboard, em);

        auto bullets = storage.entities_with_component<BulletTag>();
        REQUIRE(bullets.size() == 1);
        auto vel = storage.get_component<Velocity>(bullets[0]);
        REQUIRE(vel.has_value());
        REQUIRE(vel->get().dx == Catch::Approx(-400.0f).margin(1e-3f));
        REQUIRE(vel->get().dy == Catch::Approx(0.0f).margin(1e-3f));
    }

    SECTION("ShipVelocityAddition — Req 13.7") {
        auto& input = storage.get_component<Input>(ship)->get();
        input.fire = true;
        auto& vel = storage.get_component<Velocity>(ship)->get();
        vel.dx = 100.0f;
        vel.dy = 50.0f;
        // At angle 0: bullet = (100 + 0, 50 + 400) = (100, 450)

        system.update(storage, blackboard, em);

        auto bullets = storage.entities_with_component<BulletTag>();
        REQUIRE(bullets.size() == 1);
        auto bvel = storage.get_component<Velocity>(bullets[0]);
        REQUIRE(bvel.has_value());
        REQUIRE(bvel->get().dx == Catch::Approx(100.0f).margin(1e-3f));
        REQUIRE(bvel->get().dy == Catch::Approx(450.0f).margin(1e-3f));
    }

    SECTION("MissingPosition — Req 13.8") {
        // Create entity with Input but no Position
        Entity e2 = em.create_entity();
        storage.add_component(e2, Input{});
        auto& input2 = storage.get_component<Input>(e2)->get();
        input2.fire = true;

        REQUIRE_NOTHROW(system.update(storage, blackboard, em));
    }

    SECTION("EmptyStorage — Req 13.9") {
        // Remove all components from ship so no Input entities exist
        storage.remove_component<Input>(ship);
        storage.remove_component<Position>(ship);
        storage.remove_component<Rotation>(ship);
        storage.remove_component<Velocity>(ship);

        REQUIRE_NOTHROW(system.update(storage, blackboard, em));
    }
}
