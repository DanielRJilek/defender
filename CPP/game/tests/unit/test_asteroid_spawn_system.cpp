/**
 * Unit tests for AsteroidSpawnSystem
 *
 * These tests verify the AsteroidSpawnSystem correctly handles splitting,
 * wave spawning, component attachment, and game-over state.
 * Uses seeded constructor (seed=42) for deterministic results.
 *
 * Requirements tested: 16.4–16.6, 16.9
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/asteroid_spawn_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <cmath>

// Helper: set up a minimal Blackboard with default asteroid/spawn config
static void setup_blackboard(Blackboard& bb) {
    bb.set<int>("asteroid.large.size", 64);
    bb.set<float>("asteroid.large.speed_min", 50.0f);
    bb.set<float>("asteroid.large.speed_max", 100.0f);
    bb.set<int>("asteroid.large.split_count", 2);
    bb.set<std::string>("asteroid.large.texture", std::string("asteroid.png"));

    bb.set<int>("asteroid.medium.size", 32);
    bb.set<float>("asteroid.medium.speed_min", 75.0f);
    bb.set<float>("asteroid.medium.speed_max", 150.0f);
    bb.set<int>("asteroid.medium.split_count", 2);
    bb.set<std::string>("asteroid.medium.texture", std::string("medium_asteroid.png"));

    bb.set<int>("asteroid.small.size", 16);
    bb.set<float>("asteroid.small.speed_min", 100.0f);
    bb.set<float>("asteroid.small.speed_max", 200.0f);
    bb.set<int>("asteroid.small.split_count", 0);
    bb.set<std::string>("asteroid.small.texture", std::string("small_asteroid.png"));

    bb.set<int>("spawn.initial_large_count", 4);
    bb.set<int>("game.score", 0);
    bb.set<int>("game.lives", 3);
    bb.set<int>("game.wave", 0);
    bb.set<std::string>("game.state", std::string("PLAYING"));
}

// Helper: create an asteroid entity with the given tier and SplitCandidate
static Entity make_split_asteroid(EntityManager& em, ComponentStorage& storage,
                                   AsteroidTier tier, float x, float y) {
    Entity asteroid = em.create_entity();
    storage.add_component(asteroid, AsteroidTag{});
    storage.add_component(asteroid, Splittable{tier});
    storage.add_component(asteroid, Position{x, y});
    storage.add_component(asteroid, Size{64.0f, 64.0f});
    storage.add_component(asteroid, Velocity{10.0f, 5.0f});
    storage.add_component(asteroid, Rotation{0.5f, 0.1f});
    storage.add_component(asteroid, WrapAround{});
    storage.add_component(asteroid, Collider{64.0f, 64.0f, 4, 3});
    storage.add_component(asteroid, Images{{"asteroid.png"}});
    storage.add_component(asteroid, SplitCandidate{});
    return asteroid;
}

TEST_CASE("AsteroidSpawnSystem splitting", "[asteroid_spawn][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard blackboard;
    AsteroidSpawnSystem system(42); // seeded for determinism
    setup_blackboard(blackboard);

    SECTION("Large asteroid with SplitCandidate — 2 medium children, parent DestroyRequest") {
        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Large, 150.0f, 250.0f);

        system.update(storage, blackboard, em);

        // Parent gets DestroyRequest
        REQUIRE(storage.has_component<DestroyRequest>(parent));

        // Count medium asteroids (children) — exclude parent which is Large
        auto all_asteroids = storage.entities_with_component<AsteroidTag>();
        int medium_count = 0;
        for (Entity e : all_asteroids) {
            if (e == parent) continue;
            if (storage.has_component<Splittable>(e)) {
                auto& s = storage.get_component<Splittable>(e)->get();
                if (s.tier == AsteroidTier::Medium) medium_count++;
            }
        }
        REQUIRE(medium_count == 2);
    }

    SECTION("Medium asteroid with SplitCandidate — 2 small children, parent DestroyRequest") {
        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Medium, 100.0f, 100.0f);

        system.update(storage, blackboard, em);

        REQUIRE(storage.has_component<DestroyRequest>(parent));

        auto all_asteroids = storage.entities_with_component<AsteroidTag>();
        int small_count = 0;
        for (Entity e : all_asteroids) {
            if (e == parent) continue;
            if (storage.has_component<Splittable>(e)) {
                auto& s = storage.get_component<Splittable>(e)->get();
                if (s.tier == AsteroidTier::Small) small_count++;
            }
        }
        REQUIRE(small_count == 2);
    }

    SECTION("Small asteroid with SplitCandidate — no children, parent DestroyRequest") {
        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Small, 50.0f, 50.0f);

        size_t count_before = storage.entities_with_component<AsteroidTag>().size();

        system.update(storage, blackboard, em);

        REQUIRE(storage.has_component<DestroyRequest>(parent));

        // No new asteroids spawned (only the parent, which still has AsteroidTag)
        size_t count_after = storage.entities_with_component<AsteroidTag>().size();
        REQUIRE(count_after == count_before); // parent still counted, no children
    }

    SECTION("Split children have correct position — parent's position") {
        float px = 123.0f;
        float py = 456.0f;
        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Large, px, py);

        system.update(storage, blackboard, em);

        auto all_asteroids = storage.entities_with_component<AsteroidTag>();
        for (Entity e : all_asteroids) {
            if (e == parent) continue;
            auto& pos = storage.get_component<Position>(e)->get();
            REQUIRE(pos.x == Catch::Approx(px));
            REQUIRE(pos.y == Catch::Approx(py));
        }
    }
}

TEST_CASE("AsteroidSpawnSystem wave spawning", "[asteroid_spawn][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard blackboard;
    AsteroidSpawnSystem system(42);
    setup_blackboard(blackboard);

    SECTION("All asteroids destroyed — new wave spawned with correct count") {
        // No asteroids exist, wave=0, initial_large_count=4
        // Wave 1 should spawn: 4 + 1 - 1 = 4 large asteroids
        system.update(storage, blackboard, em);

        REQUIRE(blackboard.get<int>("game.wave") == 1);
        auto asteroids = storage.entities_with_component<AsteroidTag>();
        REQUIRE(asteroids.size() == 4);
    }

    SECTION("Wave 1 spawns initial_large_count asteroids") {
        // wave=0, initial_large_count=4 → wave becomes 1, count = 4 + 1 - 1 = 4
        system.update(storage, blackboard, em);

        REQUIRE(blackboard.get<int>("game.wave") == 1);
        REQUIRE(storage.entities_with_component<AsteroidTag>().size() == 4);
    }

    SECTION("Wave 2 spawns initial_large_count + 1 asteroids") {
        // Trigger wave 1 first
        system.update(storage, blackboard, em);
        REQUIRE(blackboard.get<int>("game.wave") == 1);

        // Destroy all wave-1 asteroids by adding DestroyRequest and removing AsteroidTag
        auto wave1 = storage.entities_with_component<AsteroidTag>();
        for (Entity e : wave1) {
            storage.remove_component<AsteroidTag>(e);
            storage.remove_component<Splittable>(e);
            storage.remove_component<Position>(e);
            storage.remove_component<Size>(e);
            storage.remove_component<Velocity>(e);
            storage.remove_component<Rotation>(e);
            storage.remove_component<WrapAround>(e);
            storage.remove_component<Collider>(e);
            storage.remove_component<Images>(e);
        }

        // Now no asteroids exist → wave 2 triggers
        system.update(storage, blackboard, em);

        REQUIRE(blackboard.get<int>("game.wave") == 2);
        // Wave 2: initial_large_count + wave - 1 = 4 + 2 - 1 = 5
        REQUIRE(storage.entities_with_component<AsteroidTag>().size() == 5);
    }

    SECTION("Spawned asteroid has all 9 required components") {
        system.update(storage, blackboard, em);

        auto asteroids = storage.entities_with_component<AsteroidTag>();
        REQUIRE(asteroids.size() > 0);

        Entity a = asteroids[0];
        REQUIRE(storage.has_component<Position>(a));
        REQUIRE(storage.has_component<Size>(a));
        REQUIRE(storage.has_component<Velocity>(a));
        REQUIRE(storage.has_component<Rotation>(a));
        REQUIRE(storage.has_component<WrapAround>(a));
        REQUIRE(storage.has_component<Collider>(a));
        REQUIRE(storage.has_component<Splittable>(a));
        REQUIRE(storage.has_component<AsteroidTag>(a));
        REQUIRE(storage.has_component<Images>(a));
    }
}

TEST_CASE("AsteroidSpawnSystem game over", "[asteroid_spawn][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard blackboard;
    AsteroidSpawnSystem system(42);
    setup_blackboard(blackboard);

    SECTION("Game over state — no spawning, no split processing") {
        blackboard.set<std::string>("game.state", std::string("GAME_OVER"));

        // Add a split candidate that should NOT be processed
        Entity asteroid = make_split_asteroid(em, storage, AsteroidTier::Large, 100.0f, 100.0f);

        size_t count_before = storage.entities_with_component<AsteroidTag>().size();

        system.update(storage, blackboard, em);

        // No new asteroids, no DestroyRequest on the split candidate
        size_t count_after = storage.entities_with_component<AsteroidTag>().size();
        REQUIRE(count_after == count_before);
        REQUIRE_FALSE(storage.has_component<DestroyRequest>(asteroid));
    }
}
