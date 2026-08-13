#ifndef COLLISION_MATH_HPP
#define COLLISION_MATH_HPP

#include <cstdint>

/**
 * Determines whether two axis-aligned bounding boxes overlap.
 *
 * Each AABB is defined by its bottom-left corner (x, y) and dimensions
 * (width, height). Uses strict inequality — touching edges/corners
 * do not count as overlap. Returns false if either AABB has zero or
 * negative width/height.
 *
 * Bottom-left coordinate system: (x, y) is the bottom-left corner,
 * top-right corner is (x + width, y + height).
 */
bool aabb_overlap(float ax, float ay, float aw, float ah,
                  float bx, float by, float bw, float bh);

/**
 * Determines whether two entities can collide based on layer/mask filtering.
 *
 * Returns true when BOTH of these conditions hold:
 *   (layer_a & mask_b) != 0
 *   (layer_b & mask_a) != 0
 *
 * Returns false if either layer is 0 (no collision group) or either mask
 * is 0 (collides with nothing), since the bitwise AND with 0 is always 0.
 */
bool layers_compatible(uint8_t layer_a, uint8_t mask_a,
                       uint8_t layer_b, uint8_t mask_b);

#endif // COLLISION_MATH_HPP
