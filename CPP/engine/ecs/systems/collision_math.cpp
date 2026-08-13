#include "collision_math.hpp"

bool aabb_overlap(float ax, float ay, float aw, float ah,
                  float bx, float by, float bw, float bh) {
    // Zero/negative dimensions → no valid AABB
    if (aw <= 0 || ah <= 0 || bw <= 0 || bh <= 0) return false;

    // Compute edges
    float right_a = ax + aw;
    float top_a   = ay + ah;
    float right_b = bx + bw;
    float top_b   = by + bh;

    // Separation test (strict inequality — touching = no overlap)
    if (right_a <= bx || right_b <= ax || top_a <= by || top_b <= ay) return false;

    return true;
}

bool layers_compatible(uint8_t layer_a, uint8_t mask_a,
                       uint8_t layer_b, uint8_t mask_b) {
    return (layer_a & mask_b) != 0 && (layer_b & mask_a) != 0;
}
