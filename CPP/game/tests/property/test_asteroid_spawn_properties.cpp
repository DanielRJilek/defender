/**
 * Property-based tests for AsteroidSpawnSystem
 *
 * These tests verify universal invariants of the AsteroidSpawnSystem across
 * random inputs: split child counts, wave spawn counts, spawn position bounds,
 * spawn speed ranges, and game-over behavior.
 *
 * Feature: 050-08-lets-rock-the-game
 * Requirements tested: 7.4–7.7, 8.2–8.4, 9.1, 9.4, 17.6–17.10
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include "game/asteroid_spawn_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <cmath>

// Configurable test iteration counts
constexpr int NUM_OUTER_TESTS = 10;
constexpr int NUM_INNER_TESTS = 5;

// Helper: set up a minimal Blackboard with default asteroid/spawn config
static void setup_blackboard(Blackboard& bb) {
    bb.set<int>("asteroid.large.size", 64);
    bb.set<float>("asteroid.large.speed_min", 50.0f);
    bb.set<float>("asteroid.large.speed_max", 100.0f);
    bb.set<int>("asteroid.large.points", 20);
    bb.set<int>("asteroid.large.split_count", 2);
    bb.set<std::string>("asteroid.large.texture", std::string("asteroid.png"));

    bb.set<int>("asteroid.medium.size", 32);
    bb.set<float>("asteroid.medium.speed_min", 75.0f);
    bb.set<float>("asteroid.medium.speed_max", 150.0f);
    bb.set<int>("asteroid.medium.points", 50);
    bb.set<int>("asteroid.medium.split_count", 2);
    bb.set<std::string>("asteroid.medium.texture", std::string("medium_asteroid.png"));

    bb.set<int>("asteroid.small.size", 16);
    bb.set<float>("asteroid.small.speed_min", 100.0f);
    bb.set<float>("asteroid.small.speed_max", 200.0f);
    bb.set<int>("asteroid.small.points", 100);
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

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 7: Split produces correct child count and destroys parent
//
// For any tier with SplitCandidate: Large -> 2 Medium children,
// Medium -> 2 Small children, Small -> 0 children.
// Parent always gets DestroyRequest.
//
// **Validates: Requirements 7.4, 7.5, 7.6, 7.7, 17.6, 17.7, 17.8**
// ============================================================================
TEST_CASE("Split produces correct child count and destroys parent", "[asteroid_spawn][property]") {
    SECTION("Large asteroid splits into 2 Medium children") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto parent_x = GENERATE(take(NUM_INNER_TESTS, random(-400.0f, 400.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);

        // Add a non-split asteroid so wave doesn't trigger after split
        Entity blocker = em.create_entity();
        storage.add_component(blocker, AsteroidTag{});

        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Large, parent_x, 100.0f);

        system.update(storage, bb, em);

        REQUIRE(storage.has_component<DestroyRequest>(parent));

        auto all = storage.entities_with_component<AsteroidTag>();
        int medium_count = 0;
        for (Entity e : all) {
            if (e == parent || e == blocker) continue;
            if (storage.has_component<Splittable>(e)) {
                auto& s = storage.get_component<Splittable>(e)->get();
                if (s.tier == AsteroidTier::Medium) medium_count++;
            }
        }
        REQUIRE(medium_count == 2);
    }

    SECTION("Medium asteroid splits into 2 Small children") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto parent_x = GENERATE(take(NUM_INNER_TESTS, random(-400.0f, 400.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);

        Entity blocker = em.create_entity();
        storage.add_component(blocker, AsteroidTag{});

        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Medium, parent_x, 100.0f);

        system.update(storage, bb, em);

        REQUIRE(storage.has_component<DestroyRequest>(parent));

        auto all = storage.entities_with_component<AsteroidTag>();
        int small_count = 0;
        for (Entity e : all) {
            if (e == parent || e == blocker) continue;
            if (storage.has_component<Splittable>(e)) {
                auto& s = storage.get_component<Splittable>(e)->get();
                if (s.tier == AsteroidTier::Small) small_count++;
            }
        }
        REQUIRE(small_count == 2);
    }

    SECTION("Small asteroid produces 0 children") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto parent_x = GENERATE(take(NUM_INNER_TESTS, random(-400.0f, 400.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);

        Entity blocker = em.create_entity();
        storage.add_component(blocker, AsteroidTag{});

        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Small, parent_x, 100.0f);

        size_t count_before = storage.entities_with_component<AsteroidTag>().size();

        system.update(storage, bb, em);

        REQUIRE(storage.has_component<DestroyRequest>(parent));

        // No new asteroids beyond parent and blocker
        size_t count_after = storage.entities_with_component<AsteroidTag>().size();
        REQUIRE(count_after == count_before);
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 8: Wave spawn count matches formula
//
// For any wave number and initial_large_count, spawned large asteroid count
// equals initial_large_count + wave - 1, and game.wave increments by 1.
//
// **Validates: Requirements 8.2, 8.3**
// ============================================================================
TEST_CASE("Wave spawn count matches formula", "[asteroid_spawn][property]") {
    SECTION("spawned count equals initial_large_count + wave - 1") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto initial_count = GENERATE(take(NUM_INNER_TESTS, random(1, 8)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);
        bb.set<int>("spawn.initial_large_count", initial_count);

        // No asteroids exist → triggers wave 1
        system.update(storage, bb, em);

        int wave = bb.get<int>("game.wave");
        REQUIRE(wave == 1);

        int expected_count = initial_count + wave - 1;
        auto asteroids = storage.entities_with_component<AsteroidTag>();
        REQUIRE(static_cast<int>(asteroids.size()) == expected_count);

        // Verify all are Large tier
        for (Entity e : asteroids) {
            REQUIRE(storage.has_component<Splittable>(e));
            auto& s = storage.get_component<Splittable>(e)->get();
            REQUIRE(s.tier == AsteroidTier::Large);
        }
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 9: Spawn position within bounds and outside safe zone
//
// For any large asteroid spawned during a wave, position satisfies:
// x in [-400, 400], y in [-300, 300], sqrt(x^2 + y^2) > 100.
//
// **Validates: Requirements 9.1, 17.9**
// ============================================================================
TEST_CASE("Spawn position within bounds and outside safe zone", "[asteroid_spawn][property]") {
    SECTION("all wave-spawned asteroids within bounds and outside safe zone") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto initial_count = GENERATE(take(NUM_INNER_TESTS, random(1, 8)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);
        bb.set<int>("spawn.initial_large_count", initial_count);

        system.update(storage, bb, em);

        auto asteroids = storage.entities_with_component<AsteroidTag>();
        REQUIRE(asteroids.size() > 0);

        for (Entity e : asteroids) {
            auto& pos = storage.get_component<Position>(e)->get();
            REQUIRE(pos.x >= -400.0f);
            REQUIRE(pos.x <= 400.0f);
            REQUIRE(pos.y >= -300.0f);
            REQUIRE(pos.y <= 300.0f);

            float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y);
            REQUIRE(dist > 100.0f);
        }
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 10: Spawn speed within configured range
//
// For any spawned asteroid (wave or split child), speed magnitude
// sqrt(dx^2 + dy^2) is within [speed_min, speed_max] for the tier.
//
// **Validates: Requirements 9.4, 17.10**
// ============================================================================
TEST_CASE("Spawn speed within configured range", "[asteroid_spawn][property]") {
    SECTION("wave-spawned large asteroids have speed in [speed_min, speed_max]") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto initial_count = GENERATE(take(NUM_INNER_TESTS, random(1, 8)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);
        bb.set<int>("spawn.initial_large_count", initial_count);

        float speed_min = bb.get<float>("asteroid.large.speed_min");
        float speed_max = bb.get<float>("asteroid.large.speed_max");

        system.update(storage, bb, em);

        auto asteroids = storage.entities_with_component<AsteroidTag>();
        for (Entity e : asteroids) {
            auto& vel = storage.get_component<Velocity>(e)->get();
            float speed = std::sqrt(vel.dx * vel.dx + vel.dy * vel.dy);
            REQUIRE(speed >= speed_min - 0.01f);
            REQUIRE(speed <= speed_max + 0.01f);
        }
    }

    SECTION("split children have speed in tier range") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto parent_x = GENERATE(take(NUM_INNER_TESTS, random(-400.0f, 400.0f)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);

        float medium_speed_min = bb.get<float>("asteroid.medium.speed_min");
        float medium_speed_max = bb.get<float>("asteroid.medium.speed_max");

        // Add blocker so wave doesn't trigger
        Entity blocker = em.create_entity();
        storage.add_component(blocker, AsteroidTag{});

        Entity parent = make_split_asteroid(em, storage, AsteroidTier::Large, parent_x, 100.0f);

        system.update(storage, bb, em);

        auto all = storage.entities_with_component<AsteroidTag>();
        for (Entity e : all) {
            if (e == parent || e == blocker) continue;
            auto& vel = storage.get_component<Velocity>(e)->get();
            float speed = std::sqrt(vel.dx * vel.dx + vel.dy * vel.dy);
            REQUIRE(speed >= medium_speed_min - 0.01f);
            REQUIRE(speed <= medium_speed_max + 0.01f);
        }
    }
}

// ============================================================================
// Feature: 050-08-lets-rock-the-game, Property 11: Game over stops asteroid spawning and split processing
//
// For any game state where game.state == "GAME_OVER", AsteroidSpawnSystem
// does not spawn or process SplitCandidates (entity counts unchanged).
//
// **Validates: Requirements 8.4**
// ============================================================================
TEST_CASE("Game over stops asteroid spawning and split processing", "[asteroid_spawn][property]") {
    SECTION("no spawning or split processing when game over") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));
        auto tier_index = GENERATE(take(NUM_INNER_TESTS, random(0, 2)));

        AsteroidTier tier;
        switch (tier_index) {
            case 0: tier = AsteroidTier::Large;  break;
            case 1: tier = AsteroidTier::Medium; break;
            default: tier = AsteroidTier::Small; break;
        }

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);
        bb.set<std::string>("game.state", std::string("GAME_OVER"));

        // Add a split candidate that should NOT be processed
        Entity asteroid = make_split_asteroid(em, storage, tier, 100.0f, 100.0f);

        size_t count_before = storage.entities_with_component<AsteroidTag>().size();

        system.update(storage, bb, em);

        size_t count_after = storage.entities_with_component<AsteroidTag>().size();
        REQUIRE(count_after == count_before);
        REQUIRE_FALSE(storage.has_component<DestroyRequest>(asteroid));
    }

    SECTION("no wave spawning when game over and no asteroids exist") {
        auto seed = GENERATE(take(NUM_OUTER_TESTS, random(0u, 100000u)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        AsteroidSpawnSystem system(seed);
        setup_blackboard(bb);
        bb.set<std::string>("game.state", std::string("GAME_OVER"));

        // No asteroids exist, but game is over — should NOT spawn wave
        system.update(storage, bb, em);

        auto asteroids = storage.entities_with_component<AsteroidTag>();
        REQUIRE(asteroids.size() == 0);
        REQUIRE(bb.get<int>("game.wave") == 0);
    }
}
