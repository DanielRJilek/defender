/**
 * DebugHUDSystem implementation
 *
 * Renders a debug overlay with pause state and step instructions.
 * Uses the same rendering pattern as HUDSystem: load font, render text
 * to surface, create texture, get dimensions, compute dest rect with
 * Y-flip, draw, and cleanup. Layout is programmatic (not from GameData.json).
 */

#include "engine/ecs/systems/debug_hud_system.hpp"
#include "engine/resource_manager.hpp"
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

DebugHUDSystem::DebugHUDSystem(SDL_Renderer* renderer, ResourceManager& rm,
                               int window_width, int window_height,
                               uint8_t overlay_alpha)
    : renderer_(renderer), resource_manager_(rm),
      window_width_(window_width), window_height_(window_height),
      overlay_alpha_(overlay_alpha) {
}

float DebugHUDSystem::render_text_line(const char* text, float game_x, float game_y) {
    TTF_Font* font = resource_manager_.load_font("default.ttf", 18);
    if (!font) return 0.0f;

    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, yellow);
    if (!surface) return 0.0f;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_DestroySurface(surface);
    if (!texture) return 0.0f;

    float text_width, text_height;
    SDL_GetTextureSize(texture, &text_width, &text_height);

    // Y-axis flip: sdl_y = window_height - game_y - text_height
    SDL_FRect dest;
    dest.x = game_x;
    dest.y = static_cast<float>(window_height_) - game_y - text_height;
    dest.w = text_width;
    dest.h = text_height;

    SDL_RenderTexture(renderer_, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);

    return text_height;
}

void DebugHUDSystem::render(bool debug_hud_visible, bool debug_paused, uint64_t frame_count) {
    // Visibility gate — render nothing if debug HUD is hidden
    if (!debug_hud_visible) return;

    // Draw full-screen semi-transparent black overlay
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, overlay_alpha_);
    SDL_FRect overlay = {0.0f, 0.0f,
                         static_cast<float>(window_width_),
                         static_cast<float>(window_height_)};
    SDL_RenderFillRect(renderer_, &overlay);

    // Position in bottom-left game coordinates:
    //   x = window_width - 300 (right side of screen)
    //   y = window_height - 25 (25px from top in screen space)
    float game_x = static_cast<float>(window_width_ - 300);
    float game_y = static_cast<float>(window_height_ - 25);

    // Status indicator (always shown — different text per state)
    const char* status_text = debug_paused
        ? "PAUSED (F1: Resume, F2: Step, J: Dump, T: Trace)"
        : "RUNNING: F1 Pause";
    float h = render_text_line(status_text, game_x, game_y);
    game_y -= (h + 2.0f);  // Move down for next line (decrease Y in bottom-left coords)

    // Frame counter (always shown when debug HUD visible)
    std::string frame_text = "Frame: " + std::to_string(frame_count);
    render_text_line(frame_text.c_str(), game_x, game_y);
}
