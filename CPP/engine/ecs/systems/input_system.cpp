/**
 * InputSystem implementation
 * 
 * Uses SDL_GetKeyboardState for continuous key polling (no OS repeat delay).
 * SDL events are only used for quit/close detection.
 * Each frame, the keyboard state array is read and Input component flags
 * are set directly from the current physical key state.
 */

#include "engine/ecs/systems/input_system.hpp"

void InputSystem::process_events(ComponentStorage& storage, bool& running) {
    // 1. Process SDL events — only for quit/close detection
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                break;
            case SDL_EVENT_QUIT:
                running = false;
                break;
            default:
                break;
        }
    }

    // 2. Poll keyboard state for continuous input (no repeat delay)
    const bool* keys = SDL_GetKeyboardState(nullptr);

    auto entities = storage.entities_with_component<Input>();
    for (Entity entity : entities) {
        auto input_opt = storage.get_component<Input>(entity);
        if (input_opt.has_value()) {
            Input& input = input_opt->get();
            input.up    = keys[SDL_SCANCODE_UP];
            input.down  = keys[SDL_SCANCODE_DOWN];
            input.left  = keys[SDL_SCANCODE_LEFT];
            input.right = keys[SDL_SCANCODE_RIGHT];
            input.fire  = keys[SDL_SCANCODE_SPACE];
            input.hyperspace = keys[SDL_SCANCODE_X];
        }
    }
}
