/**
 * Property-based tests for HUDUpdateSystem
 *
 * These tests verify universal formatting invariants of the HUDUpdateSystem
 * across random game state values.
 *
 * Feature: 050-09-hud-polish
 * Requirements tested: 3.2, 4.1, 5.1, 6.1, 6.2, 11.4, 11.5, 11.6, 11.7, 11.8
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include "game/hud_update_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include <string>

// Configurable test iteration counts
constexpr int NUM_OUTER_TESTS = 10;
constexpr int NUM_INNER_TESTS = 5;

// Helper: create a HUD entity with a Text component and register on Blackboard
static Entity make_hud(EntityManager& em, ComponentStorage& storage,
                       Blackboard& bb, const std::string& bb_key) {
    Entity e = em.create_entity();
    storage.add_component(e, Text{"", "default.ttf", 20.0f, {255,255,255,255}});
    storage.add_component(e, ScreenPosition{0.0f, 0.0f});
    bb.set<Entity>(bb_key, e);
    return e;
}

// ============================================================================
// Feature: 050-09-hud-polish, Property 1: Numeric HUD text formatting
//
// For any non-negative integer score, any integer lives in [0,10], and any
// positive integer wave, after HUDUpdateSystem updates, the Text content of
// hud_score equals "Score: <score>", hud_lives equals "Lives: <lives>",
// and hud_wave equals "Wave: <wave>".
//
// **Validates: Requirements 3.2, 4.1, 5.1, 11.4, 11.5, 11.6**
// ============================================================================
TEST_CASE("Numeric HUD text formatting", "[hud_update][property]") {
    SECTION("Score formatting matches 'Score: N'") {
        auto score = GENERATE(take(NUM_OUTER_TESTS, random(0, 100000)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        HUDUpdateSystem system;

        Entity e = make_hud(em, storage, bb, "entity.id.hud_score");
        bb.set<int>("game.score", score);

        system.update(storage, bb);

        std::string expected = "Score: " + std::to_string(score);
        REQUIRE(storage.get_component<Text>(e)->get().content == expected);
    }

    SECTION("Lives formatting matches 'Lives: N'") {
        auto lives = GENERATE(take(NUM_OUTER_TESTS, random(0, 10)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        HUDUpdateSystem system;

        Entity e = make_hud(em, storage, bb, "entity.id.hud_lives");
        bb.set<int>("game.lives", lives);

        system.update(storage, bb);

        std::string expected = "Lives: " + std::to_string(lives);
        REQUIRE(storage.get_component<Text>(e)->get().content == expected);
    }

    SECTION("Wave formatting matches 'Wave: N'") {
        auto wave = GENERATE(take(NUM_OUTER_TESTS, random(1, 1000)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        HUDUpdateSystem system;

        Entity e = make_hud(em, storage, bb, "entity.id.hud_wave");
        bb.set<int>("game.wave", wave);

        system.update(storage, bb);

        std::string expected = "Wave: " + std::to_string(wave);
        REQUIRE(storage.get_component<Text>(e)->get().content == expected);
    }
}

// ============================================================================
// Feature: 050-09-hud-polish, Property 2: Game-over overlay determined by game state
//
// For any combination of score, lives, and wave values, when game.state equals
// "GAME_OVER" the overlay text is "GAME OVER", and when "PLAYING" it is "".
//
// **Validates: Requirements 6.1, 6.2, 11.7, 11.8**
// ============================================================================
TEST_CASE("Game-over overlay determined by game state", "[hud_update][property]") {
    SECTION("GAME_OVER state shows overlay regardless of score/lives/wave") {
        auto score = GENERATE(take(NUM_OUTER_TESTS, random(0, 100000)));
        auto lives = GENERATE(take(NUM_INNER_TESTS, random(0, 10)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        HUDUpdateSystem system;

        Entity e = make_hud(em, storage, bb, "entity.id.hud_game_over");
        bb.set<int>("game.score", score);
        bb.set<int>("game.lives", lives);
        bb.set<std::string>("game.state", std::string("GAME_OVER"));

        system.update(storage, bb);

        REQUIRE(storage.get_component<Text>(e)->get().content == "GAME OVER");
    }

    SECTION("PLAYING state hides overlay regardless of score/lives/wave") {
        auto score = GENERATE(take(NUM_OUTER_TESTS, random(0, 100000)));
        auto wave = GENERATE(take(NUM_INNER_TESTS, random(1, 100)));

        EntityManager em;
        ComponentStorage storage;
        Blackboard bb;
        HUDUpdateSystem system;

        Entity e = make_hud(em, storage, bb, "entity.id.hud_game_over");
        bb.set<int>("game.score", score);
        bb.set<int>("game.wave", wave);
        bb.set<std::string>("game.state", std::string("PLAYING"));

        system.update(storage, bb);

        REQUIRE(storage.get_component<Text>(e)->get().content == "");
    }
}
