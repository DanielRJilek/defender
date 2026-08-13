/**
 * Unit tests for HUDUpdateSystem
 *
 * Verifies the HUDUpdateSystem correctly formats and updates HUD text
 * from Blackboard values for score, lives, wave, and game-over overlay.
 *
 * Requirements tested: 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7
 */

#include <catch2/catch_test_macros.hpp>
#include "game/hud_update_system.hpp"
#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"

// Helper: create a HUD entity with a Text component and register it on the Blackboard
static Entity make_hud_entity(EntityManager& em, ComponentStorage& storage,
                              Blackboard& bb, const std::string& bb_key,
                              float sx, float sy) {
    Entity e = em.create_entity();
    storage.add_component(e, Text{"", "default.ttf", 20.0f, {255,255,255,255}});
    storage.add_component(e, ScreenPosition{sx, sy});
    bb.set<Entity>(bb_key, e);
    return e;
}

TEST_CASE("HUDUpdateSystem score display", "[hud_update][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard bb;
    HUDUpdateSystem system;

    make_hud_entity(em, storage, bb, "entity.id.hud_score", 10.0f, 580.0f);

    SECTION("Score 0 — Req 10.1") {
        bb.set<int>("game.score", 0);
        system.update(storage, bb);
        Entity e = bb.get<Entity>("entity.id.hud_score");
        REQUIRE(storage.get_component<Text>(e)->get().content == "Score: 0");
    }

    SECTION("Score 1500 — Req 10.2") {
        bb.set<int>("game.score", 1500);
        system.update(storage, bb);
        Entity e = bb.get<Entity>("entity.id.hud_score");
        REQUIRE(storage.get_component<Text>(e)->get().content == "Score: 1500");
    }
}

TEST_CASE("HUDUpdateSystem lives display", "[hud_update][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard bb;
    HUDUpdateSystem system;

    make_hud_entity(em, storage, bb, "entity.id.hud_lives", 10.0f, 555.0f);

    SECTION("Lives 3 — Req 10.3") {
        bb.set<int>("game.lives", 3);
        system.update(storage, bb);
        Entity e = bb.get<Entity>("entity.id.hud_lives");
        REQUIRE(storage.get_component<Text>(e)->get().content == "Lives: 3");
    }

    SECTION("Lives 0 — Req 10.4") {
        bb.set<int>("game.lives", 0);
        system.update(storage, bb);
        Entity e = bb.get<Entity>("entity.id.hud_lives");
        REQUIRE(storage.get_component<Text>(e)->get().content == "Lives: 0");
    }
}

TEST_CASE("HUDUpdateSystem wave display", "[hud_update][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard bb;
    HUDUpdateSystem system;

    make_hud_entity(em, storage, bb, "entity.id.hud_wave", 10.0f, 530.0f);

    SECTION("Wave 5 — Req 10.5") {
        bb.set<int>("game.wave", 5);
        system.update(storage, bb);
        Entity e = bb.get<Entity>("entity.id.hud_wave");
        REQUIRE(storage.get_component<Text>(e)->get().content == "Wave: 5");
    }
}

TEST_CASE("HUDUpdateSystem game-over overlay", "[hud_update][unit]") {
    EntityManager em;
    ComponentStorage storage;
    Blackboard bb;
    HUDUpdateSystem system;

    make_hud_entity(em, storage, bb, "entity.id.hud_game_over", 300.0f, 310.0f);

    SECTION("State GAME_OVER — Req 10.6") {
        bb.set<std::string>("game.state", std::string("GAME_OVER"));
        system.update(storage, bb);
        Entity e = bb.get<Entity>("entity.id.hud_game_over");
        REQUIRE(storage.get_component<Text>(e)->get().content == "GAME OVER");
    }

    SECTION("State PLAYING — Req 10.7") {
        bb.set<std::string>("game.state", std::string("PLAYING"));
        system.update(storage, bb);
        Entity e = bb.get<Entity>("entity.id.hud_game_over");
        REQUIRE(storage.get_component<Text>(e)->get().content == "");
    }
}
