/**
 * Unit tests for collision math functions (layers_compatible)
 *
 * These tests verify layer/mask filtering with concrete game scenarios
 * from the Asteroids game: ship, bullet, asteroid collision groups.
 *
 * Requirements tested: 9.1, 9.2, 9.3, 9.4, 9.5, 9.6
 */

#include <catch2/catch_test_macros.hpp>
#include "engine/ecs/systems/collision_math.hpp"

TEST_CASE("layers_compatible filtering", "[collision][unit]") {
    // Ship:     layer=1 (0x01), mask=4 (0x04) — collides with asteroids
    // Bullet:   layer=2 (0x02), mask=4 (0x04) — collides with asteroids
    // Asteroid: layer=4 (0x04), mask=3 (0x03) — collides with ships and bullets

    SECTION("ShipAsteroidCompatible — Req 9.1") {
        // ship layer=1, mask=4 vs asteroid layer=4, mask=3
        // (1 & 3) != 0 → true, (4 & 4) != 0 → true → compatible
        REQUIRE(layers_compatible(1, 4, 4, 3) == true);
    }

    SECTION("ShipBulletIncompatible — Req 9.2") {
        // ship layer=1, mask=4 vs bullet layer=2, mask=4
        // (1 & 4) != 0 → false → incompatible
        REQUIRE(layers_compatible(1, 4, 2, 4) == false);
    }

    SECTION("AsteroidAsteroidIncompatible — Req 9.3") {
        // asteroid layer=4, mask=3 vs asteroid layer=4, mask=3
        // (4 & 3) != 0 → false → incompatible
        REQUIRE(layers_compatible(4, 3, 4, 3) == false);
    }

    SECTION("LayerZeroAlwaysFalse — Req 9.4") {
        // layer=0 means "no collision group"
        REQUIRE(layers_compatible(0, 0xFF, 4, 3) == false);
        REQUIRE(layers_compatible(4, 3, 0, 0xFF) == false);
    }

    SECTION("MaskZeroAlwaysFalse — Req 9.5") {
        // mask=0 means "collides with nothing"
        REQUIRE(layers_compatible(1, 0, 4, 3) == false);
        REQUIRE(layers_compatible(4, 3, 1, 0) == false);
    }

    SECTION("AllBitsSetCompatible — Req 9.6") {
        // layer=0xFF, mask=0xFF vs same → all-collide configuration
        REQUIRE(layers_compatible(0xFF, 0xFF, 0xFF, 0xFF) == true);
    }
}

TEST_CASE("aabb_overlap scenarios", "[Engine][collision][unit]") {
    // AABBs are defined by bottom-left corner (x, y) and dimensions (w, h).
    // Strict inequality: touching edges/corners do NOT count as overlap.
    // Zero or negative width/height yields no overlap.

    SECTION("Overlapping") {
        // Box A spans [0,10]x[0,10]; Box B spans [5,15]x[5,15] — clear intersection.
        REQUIRE(aabb_overlap(0, 0, 10, 10, 5, 5, 10, 10) == true);
    }

    SECTION("Separated") {
        // Box A spans [0,10]x[0,10]; Box B spans [100,110]x[100,110] — wide gap.
        REQUIRE(aabb_overlap(0, 0, 10, 10, 100, 100, 10, 10) == false);
    }

    SECTION("EdgeTouching") {
        // A right edge x=100 meets B left edge x=100; strict inequality → no overlap.
        REQUIRE(aabb_overlap(0, 0, 100, 100, 100, 0, 100, 100) == false);
    }

    SECTION("CornerTouching") {
        // A top-right corner (100,100) meets B bottom-left corner (100,100) → no overlap.
        REQUIRE(aabb_overlap(0, 0, 100, 100, 100, 100, 50, 50) == false);
    }

    SECTION("ZeroWidthDegenerate") {
        // Box A has zero width → degenerate, overlaps nothing.
        REQUIRE(aabb_overlap(0, 0, 0, 10, 0, 0, 10, 10) == false);
    }

    SECTION("Containment") {
        // Box B [25,35]x[25,35] lies fully inside Box A [0,100]x[0,100].
        REQUIRE(aabb_overlap(0, 0, 100, 100, 25, 25, 10, 10) == true);
    }
}
