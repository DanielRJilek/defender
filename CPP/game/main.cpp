/**
 * Defender
 *
 * The first Asteroids game entity: a player-controlled ship that rotates,
 * thrusts, and wraps around the screen. No bullets, no asteroids — just
 * the ship with classic momentum-based physics.
 *
 * The ShipControlSystem (game-level) reads arrow key input and translates
 * it into rotation and thrust. Left/Right arrows set angular_velocity on
 * the Rotation component; the RotationSystem (engine-level) handles the
 * actual angle update. Up arrow applies acceleration in the ship's facing
 * direction using cos(angle) and sin(angle). There is no drag — the ship
 * retains momentum indefinitely.
 *
 * World space uses a centered origin: (0,0) is the center of the world,
 * positive X rightward, positive Y upward (Cartesian convention).
 *
 * System execution order:
 *   GameStateSystem → InputSystem → ShipControlSystem → RotationSystem → MovementSystem →
 *   WrapSystem → CollisionSystem → CollisionResponseSystem → AbductionSystem →
 *   AlienSpawnSystem → AstronautSpawnSystem → LifetimeSystem → ScriptSystem →
 *   destroy_marked_entities →
 *   CameraSystem → RenderSystem → HUDSystem → DebugHUDSystem
 *
 * Controls:
 *   Left/Right arrows — Rotate ship
 *   Up arrow          — Thrust forward
 *   H                 — Toggle HUD visibility
 *   F1                — Toggle debug pause
 *   F2                — Single step (while paused)
 *   F10               — Toggle debug HUD
 *   ESC / Close       — Exit
 *
 * Requirements: 8.1–8.7
 */

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <ctime>

#include "engine/ecs/entity_manager.hpp"
#include "engine/ecs/component_storage.hpp"
#include "engine/ecs/blackboard.hpp"
#include "engine/timer.hpp"
#include "engine/resource_manager.hpp"
#include "engine/project_paths.hpp"
#include "engine/ecs/systems/render_system.hpp"
#include "engine/ecs/systems/input_system.hpp"
#include "engine/ecs/systems/movement_system.hpp"
#include "engine/ecs/systems/hud_system.hpp"
#include "engine/ecs/systems/camera_system.hpp"
#include "engine/ecs/systems/camera_control_system.hpp"
#include "engine/ecs/systems/collision_system.hpp"
#include "engine/ecs/systems/brute_force_strategy.hpp"
#include "engine/ecs/systems/wrap_system.hpp"
#include "engine/ecs/systems/lifetime_system.hpp"
#include "engine/ecs/destruction.hpp"
#include "ship_control_system.hpp"
#include "laser_spawn_system.hpp"
#include "bullet_spawn_system.hpp"
#include "collision_response_system.hpp"
#include "hud_update_system.hpp"
#include "cli_parser.hpp"
#include "debug_state.hpp"
#include "engine/ecs/systems/debug_hud_system.hpp"
#include "engine/ecs/systems/screenshot_system.hpp"
#include "script_loader.hpp"
#include "engine/gamedata_loader.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "alien_spawn_system.hpp"
#include "game_state_system.hpp"
#include "astronaut_spawn_system.hpp"
#include "engine/lua_manager.hpp"
#include "engine/ecs/systems/script_system.hpp"
#include "direction_system.hpp"
#include "abduction_system.hpp"
#include "cleanup_system.hpp"
#include "hyperspace_system.hpp"

// Debug infrastructure for frame dump/trace
#include "engine/ecs/debug/json_serializer.hpp"
#include "engine/ecs/debug/debug_types.hpp"

/**
 * Custom deleter for SDL_Window
 * Ensures proper cleanup of SDL resources using RAII
 */
struct SDL_WindowDeleter {
    void operator()(SDL_Window* window) const {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
};

/**
 * Custom deleter for SDL_Renderer
 * Ensures proper cleanup of SDL resources using RAII
 */
struct SDL_RendererDeleter {
    void operator()(SDL_Renderer* renderer) const {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
    }
};

// Type aliases for smart pointers with custom deleters
using SDL_WindowPtr = std::unique_ptr<SDL_Window, SDL_WindowDeleter>;
using SDL_RendererPtr = std::unique_ptr<SDL_Renderer, SDL_RendererDeleter>;

/**
 * Concrete implementation of IBlackboardAccessor for the Blackboard class.
 * Allows JSONSerializer to access blackboard values for frame dump/trace.
 * Read-only interface — the serializer never writes to the blackboard.
 */
class BlackboardAccessor : public engine::debug::IBlackboardAccessor {
public:
    explicit BlackboardAccessor(Blackboard& blackboard) : blackboard_(blackboard) {}

    engine::debug::Value get_value(const std::string& key) const override {
        engine::debug::Value result;

        // Try int
        try {
            int val = blackboard_.get<int>(key);
            result.type = engine::debug::Value::Type::Int;
            result.data = static_cast<int64_t>(val);
            return result;
        } catch (...) {}

        // Try float
        try {
            float val = blackboard_.get<float>(key);
            result.type = engine::debug::Value::Type::Float;
            result.data = static_cast<double>(val);
            return result;
        } catch (...) {}

        // Try double
        try {
            double val = blackboard_.get<double>(key);
            result.type = engine::debug::Value::Type::Float;
            result.data = val;
            return result;
        } catch (...) {}

        // Try bool
        try {
            bool val = blackboard_.get<bool>(key);
            result.type = engine::debug::Value::Type::Bool;
            result.data = val;
            return result;
        } catch (...) {}

        // Try string
        try {
            std::string val = blackboard_.get<std::string>(key);
            result.type = engine::debug::Value::Type::String;
            result.data = val;
            return result;
        } catch (...) {}

        // Try uint64_t (for frame_count)
        try {
            uint64_t val = blackboard_.get<uint64_t>(key);
            result.type = engine::debug::Value::Type::Int;
            result.data = static_cast<int64_t>(val);
            return result;
        } catch (...) {}

        throw std::runtime_error("Blackboard key '" + key + "' not found or unsupported type");
    }

    std::vector<std::string> get_all_keys() const override {
        return blackboard_.get_all_keys();
    }

private:
    Blackboard& blackboard_;
};

// --- Helper functions to reduce Value construction boilerplate ---
inline std::unique_ptr<engine::debug::Value> make_float_value(double v) {
    auto val = std::make_unique<engine::debug::Value>();
    val->type = engine::debug::Value::Type::Float;
    val->data = v;
    return val;
}
inline std::unique_ptr<engine::debug::Value> make_int_value(int64_t v) {
    auto val = std::make_unique<engine::debug::Value>();
    val->type = engine::debug::Value::Type::Int;
    val->data = v;
    return val;
}
inline std::unique_ptr<engine::debug::Value> make_bool_value(bool v) {
    auto val = std::make_unique<engine::debug::Value>();
    val->type = engine::debug::Value::Type::Bool;
    val->data = v;
    return val;
}
inline std::unique_ptr<engine::debug::Value> make_string_value(const std::string& v) {
    auto val = std::make_unique<engine::debug::Value>();
    val->type = engine::debug::Value::Type::String;
    val->data = v;
    return val;
}

// --- ComponentBridge adapter: type-erased component access ---
template<typename T>
class ComponentBridgeAdapter : public engine::debug::IComponentBridge {
public:
    std::optional<std::any> get_as_any(Entity entity, ComponentStorage& cs) const override {
        auto opt = cs.get_component<T>(entity);
        if (!opt.has_value()) return std::nullopt;
        return std::any(opt->get());
    }
    bool has(Entity entity, const ComponentStorage& cs) const override {
        return cs.has_component<T>(entity);
    }
};

// --- PropertyAccessor adapter: component registry ---
class PropertyAccessorAdapter : public engine::debug::IPropertyAccessor {
public:
    template<typename T>
    void register_component(const std::string& name) {
        bridges_.push_back({name, std::make_unique<ComponentBridgeAdapter<T>>()});
    }
    std::vector<std::pair<std::string, const engine::debug::IComponentBridge*>>
    registered_components() const override {
        std::vector<std::pair<std::string, const engine::debug::IComponentBridge*>> result;
        for (const auto& [name, bridge] : bridges_) {
            result.push_back({name, bridge.get()});
        }
        std::sort(result.begin(), result.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        return result;
    }
private:
    std::vector<std::pair<std::string, std::unique_ptr<engine::debug::IComponentBridge>>> bridges_;
};

// --- EntityMapper adapter ---
class EntityMapperAdapter : public engine::debug::IEntityMapper {
public:
    void map(int logical_id, Entity actual_id) {
        mapping_[logical_id] = actual_id;
    }
    /**
     * Walk all live tagged entities (ship, bullets, asteroids, HUD-bearing
     * entities) and register each with logical_id == actual_id. Idempotent.
     * Existing baseline registrations (e.g., map(1, ship) from main()) are
     * preserved — this method only ADDS entries, never removes.
     *
     * Used by the dump and trace paths to ensure the JSONSerializer sees
     * every live ECS entity, not just the ones registered at startup.
     *
     * Defect 1.25, 1.26 — Req 2.24, 2.25.
     */
    void map_all_live_entities(const ComponentStorage& cs) {
        const auto add_each = [&](const std::vector<Entity>& entities) {
            for (Entity e : entities) {
                mapping_[static_cast<int>(e)] = e;
            }
        };
        add_each(cs.entities_with_component<ShipTag>());
        add_each(cs.entities_with_component<BulletTag>());
        add_each(cs.entities_with_component<LaserTag>());
        add_each(cs.entities_with_component<ScreenPosition>());
    }
    std::vector<int> get_logical_ids() const override {
        std::vector<int> ids;
        for (const auto& [lid, _] : mapping_) ids.push_back(lid);
        return ids;
    }
    Entity get_actual_id(int logical_id) const override {
        return mapping_.at(logical_id);
    }
private:
    std::unordered_map<int, Entity> mapping_;
};

// --- TypeIntrospector adapter: type metadata and value conversion ---
class TypeIntrospectorAdapter : public engine::debug::ITypeIntrospector {
public:
    using ValueConverter = std::function<engine::debug::Value(const std::any&)>;

    void register_struct(const std::string& name,
                         std::vector<engine::debug::FieldInfo> fields,
                         ValueConverter converter) {
        engine::debug::TypeInfo info;
        info.name = name;
        info.kind = engine::debug::TypeInfo::Kind::Struct;
        info.fields = std::move(fields);
        types_[name] = std::move(info);
        converters_[name] = std::move(converter);
    }

    bool has_type(const std::string& type_name) const override {
        return types_.count(type_name) > 0;
    }
    const engine::debug::TypeInfo& get_type(const std::string& type_name) const override {
        return types_.at(type_name);
    }
    engine::debug::Value cpp_to_value(const std::any& cpp_value,
                                       const std::string& type_name) const override {
        return converters_.at(type_name)(cpp_value);
    }
private:
    std::unordered_map<std::string, engine::debug::TypeInfo> types_;
    std::unordered_map<std::string, ValueConverter> converters_;
};

// --- Local timestamp helper (replaces JSONSerializer::current_timestamp() which is private) ---
static std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H-%M-%S", &tm_buf);
    return std::string(buf);
}

int main(int argc, char* argv[]) {
    // Parse command-line debug options before any SDL initialization
    auto opts = parse_command_line(argc, argv);
    if (opts.help_requested) return 0;
    if (opts.parse_error) return 1;

    if (!opts.script_file.empty()) {
        try {
            std::string script_path = opts.script_file;
            opts = load_script(script_path);
            opts.script_file = script_path;
            std::cout << "Loaded script: " << script_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    // Initialize SDL3 video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL3_ttf for font rendering (required by HUDSystem)
    if (!TTF_Init()) {
        std::cerr << "Failed to initialize SDL_ttf: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create window (800x600)
    SDL_WindowPtr window(
        SDL_CreateWindow(
            "Defender",
            2000, 800,
            SDL_WINDOW_RESIZABLE
        )
    );

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create renderer with hardware acceleration
    SDL_RendererPtr renderer(
        SDL_CreateRenderer(window.get(), nullptr)
    );

    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // === ECS Setup ===

    EntityManager entity_manager;
    ComponentStorage component_storage;
    Blackboard blackboard;

    // Initialize Blackboard with game state
    blackboard.set("hud_visible", true);

    // Load entities from GameData.json
    std::string gamedata_path = project_paths::assets_dir() + "/GameData.json";
    load_game_data(gamedata_path, entity_manager, component_storage, blackboard);

    // Create ResourceManager (loads textures/fonts from assets directory).
    // Held by unique_ptr so it can be torn down explicitly during shutdown
    // BEFORE TTF_Quit()/SDL_Quit() — its destructor calls SDL_DestroyTexture and
    // TTF_CloseFont, which must run while SDL/TTF are still initialized.
    auto resource_manager_ptr =
        std::make_unique<ResourceManager>(renderer.get(), project_paths::assets_dir());
    ResourceManager& resource_manager = *resource_manager_ptr;

    // Create Timer with target FPS (CLI override or default 60)
    Timer timer(opts.fps > 0 ? static_cast<double>(opts.fps) : 60.0);
    // --seed pins spawn RNG; fixed timestep pins per-frame integration.
    if (opts.seed.has_value()) {
        timer.set_deterministic(true);
    }
    timer.update_blackboard(blackboard);

    // Create engine systems
    InputSystem input_system;
    MovementSystem movement_system;
    RenderSystem render_system(renderer.get(), resource_manager);
    render_system.set_background_texture("");
    CameraSystem camera_system;
    CameraControlSystem camera_control_system;
    HUDSystem hud_system(renderer.get(), resource_manager, 800, 600);

    // Create new Defender systems
    ShipControlSystem ship_control_system;
    LaserSpawnSystem laser_spawn_system;
    BulletSpawnSystem bullet_spawn_system;
    BruteForceStrategy brute_force_strategy;
    CollisionSystem collision_system(brute_force_strategy);
    WrapSystem wrap_system;
    LifetimeSystem lifetime_system;
    AlienSpawnSystem alien_spawn_system;
    AstronautSpawnSystem astronaut_spawn_system;
    GameStateSystem game_state_system;
    DirectionSystem direction_system;
    AbductionSystem abduction_system(alien_spawn_system);
    CleanupSystem cleanup_system;
    HyperspaceSystem hyperspace_system;
    // Collision response and asteroid spawn systems
    CollisionResponseSystem collision_response_system;
    HUDUpdateSystem hud_update_system;

    // Lua scripting
    LuaManager lua_manager;
    ScriptSystem script_system(lua_manager);

    // Debug state
    bool debug_paused = false;
    bool debug_hud_visible = false;
    bool step_requested = false;

    // Interactive dump/trace capture (J/T keys). One-shot: set when the key is
    // pressed or injected, folded into is_dump_frame/is_trace_frame, then cleared
    // after the step completes (alongside step_requested).
    bool force_dump_this_step = false;   // J: dump this frame (before -> advance -> after)
    bool force_trace_this_step = false;  // T: full per-system trace of this frame

    // Apply --paused CLI option
    if (opts.paused) {
        debug_paused = true;
        debug_hud_visible = true;
    }

    // Create log directory for this run
    std::string log_dir = create_log_directory(opts.clear_logs);
    std::cout << "Log directory: " << log_dir << std::endl;

    // Edge detection state
    bool f1_was_pressed = false;
    bool f2_was_pressed = false;
    bool f10_was_pressed = false;
    bool j_was_pressed = false;
    bool t_was_pressed = false;

    // Key injection state: track whether arrow keys were injected last frame
    bool injected_arrow_last_frame = false;

    // Create DebugHUDSystem (overlay alpha from GameData.json via Blackboard)
    uint8_t debug_overlay_alpha = static_cast<uint8_t>(
        blackboard.get_or<int>("debug.overlay_alpha", 128));
    ScreenshotSystem screenshot_system(renderer.get(), log_dir);
    DebugHUDSystem debug_hud_system(renderer.get(), resource_manager, 800, 600,
                                    debug_overlay_alpha);

    // Retrieve ship entity from Blackboard
    Entity ship = blackboard.get<Entity>("entity.id.ship");

    // === Debug infrastructure for frame dump/trace ===
    std::unique_ptr<PropertyAccessorAdapter> prop_accessor;
    std::unique_ptr<TypeIntrospectorAdapter> introspector;
    std::unique_ptr<EntityMapperAdapter> ent_mapper;
    std::unique_ptr<BlackboardAccessor> bb_accessor;
    std::unique_ptr<engine::debug::JSONSerializer> json_serializer;

    bool dump_trace_enabled = !opts.dump_frames.empty() || !opts.trace_frames.empty();

    // The serializer is needed for frame-number-driven --dump/--trace AND for the
    // interactive J/T keys. dump_trace_enabled keeps its meaning: it gates ONLY the
    // frame-number auto-dump matching below. The debug stack is built whenever either
    // is in play. (J/T serialize nothing until pressed.)
    bool serializer_needed = dump_trace_enabled || opts.debug_keys;

    if (serializer_needed) {
        prop_accessor = std::make_unique<PropertyAccessorAdapter>();
        introspector = std::make_unique<TypeIntrospectorAdapter>();
        ent_mapper = std::make_unique<EntityMapperAdapter>();

        // Register component types with adapter classes
        prop_accessor->register_component<Position>("Position");
        introspector->register_struct("Position",
            {{"x", "float"}, {"y", "float"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& p = std::any_cast<const Position&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["x"] = make_float_value(static_cast<double>(p.x));
                obj["y"] = make_float_value(static_cast<double>(p.y));
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Size>("Size");
        introspector->register_struct("Size",
            {{"height", "float"}, {"width", "float"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& s = std::any_cast<const Size&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["width"] = make_float_value(static_cast<double>(s.width));
                obj["height"] = make_float_value(static_cast<double>(s.height));
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Color>("Color");
        introspector->register_struct("Color",
            {{"a", "uint8_t"}, {"b", "uint8_t"}, {"g", "uint8_t"}, {"r", "uint8_t"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& c = std::any_cast<const Color&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["r"] = make_int_value(static_cast<int64_t>(c.r));
                obj["g"] = make_int_value(static_cast<int64_t>(c.g));
                obj["b"] = make_int_value(static_cast<int64_t>(c.b));
                obj["a"] = make_int_value(static_cast<int64_t>(c.a));
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Velocity>("Velocity");
        introspector->register_struct("Velocity",
            {{"dx", "float"}, {"dy", "float"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& vel = std::any_cast<const Velocity&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["dx"] = make_float_value(static_cast<double>(vel.dx));
                obj["dy"] = make_float_value(static_cast<double>(vel.dy));
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Input>("Input");
        introspector->register_struct("Input",
            {{"down", "bool"}, {"fire", "bool"}, {"left", "bool"}, {"right", "bool"}, {"up", "bool"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& inp = std::any_cast<const Input&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["up"] = make_bool_value(inp.up);
                obj["down"] = make_bool_value(inp.down);
                obj["left"] = make_bool_value(inp.left);
                obj["right"] = make_bool_value(inp.right);
                obj["fire"] = make_bool_value(inp.fire);
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<WrapAround>("WrapAround");
        introspector->register_struct("WrapAround",
            {},
            [](const std::any&) -> engine::debug::Value {
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<BulletTag>("BulletTag");
        introspector->register_struct("BulletTag",
            {},
            [](const std::any&) -> engine::debug::Value {
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                val.data = std::move(obj);
                return val;
            });
        prop_accessor->register_component<LaserTag>("LaserTag");
        introspector->register_struct("LaserTag",
            {},
            [](const std::any&) -> engine::debug::Value {
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<ShipTag>("ShipTag");
        introspector->register_struct("ShipTag",
            {},
            [](const std::any&) -> engine::debug::Value {
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Images>("Images");
        introspector->register_struct("Images",
            {{"active_index", "uint32_t"}, {"filenames", "vector<std::string>"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& img = std::any_cast<const Images&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["active_index"] = make_int_value(static_cast<int64_t>(img.active_index));
                // filenames as array of strings
                std::vector<std::unique_ptr<engine::debug::Value>> arr;
                for (const auto& fn : img.filenames) {
                    arr.push_back(make_string_value(fn));
                }
                auto arr_val = std::make_unique<engine::debug::Value>();
                arr_val->type = engine::debug::Value::Type::Array;
                arr_val->data = std::move(arr);
                obj["filenames"] = std::move(arr_val);
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<ScreenPosition>("ScreenPosition");
        introspector->register_struct("ScreenPosition",
            {{"x", "float"}, {"y", "float"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& sp = std::any_cast<const ScreenPosition&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["x"] = make_float_value(static_cast<double>(sp.x));
                obj["y"] = make_float_value(static_cast<double>(sp.y));
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Collider>("Collider");
        introspector->register_struct("Collider",
            {{"width", "float"}, {"height", "float"}, {"layer", "int"}, {"mask", "int"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& col = std::any_cast<const Collider&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["width"] = make_float_value(static_cast<double>(col.width));
                obj["height"] = make_float_value(static_cast<double>(col.height));
                obj["layer"] = make_int_value(static_cast<int64_t>(col.layer));
                obj["mask"] = make_int_value(static_cast<int64_t>(col.mask));
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<CollidedWith>("CollidedWith");
        introspector->register_struct("CollidedWith",
            {{"entities", "array"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& cw = std::any_cast<const CollidedWith&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                // entities as array of int (entity IDs)
                std::vector<std::unique_ptr<engine::debug::Value>> arr;
                for (const auto& e : cw.entities) {
                    arr.push_back(make_int_value(static_cast<int64_t>(e)));
                }
                auto arr_val = std::make_unique<engine::debug::Value>();
                arr_val->type = engine::debug::Value::Type::Array;
                arr_val->data = std::move(arr);
                obj["entities"] = std::move(arr_val);
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<DestroyRequest>("DestroyRequest");
        introspector->register_struct("DestroyRequest",
            {},
            [](const std::any&) -> engine::debug::Value {
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Lifetime>("Lifetime");
        introspector->register_struct("Lifetime",
            {{"remaining", "float"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& lt = std::any_cast<const Lifetime&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["remaining"] = make_float_value(static_cast<double>(lt.remaining));
                val.data = std::move(obj);
                return val;
            });

        prop_accessor->register_component<Text>("Text");
        introspector->register_struct("Text",
            {{"content", "string"}, {"font_name", "string"}, {"font_size", "float"},
             {"color_r", "int"}, {"color_g", "int"}, {"color_b", "int"}, {"color_a", "int"}},
            [](const std::any& v) -> engine::debug::Value {
                const auto& txt = std::any_cast<const Text&>(v);
                engine::debug::Value val;
                val.type = engine::debug::Value::Type::Object;
                std::unordered_map<std::string, std::unique_ptr<engine::debug::Value>> obj;
                obj["content"] = make_string_value(txt.content);
                obj["font_name"] = make_string_value(txt.font_name);
                obj["font_size"] = make_float_value(static_cast<double>(txt.font_size));
                obj["color_r"] = make_int_value(static_cast<int64_t>(txt.color.r));
                obj["color_g"] = make_int_value(static_cast<int64_t>(txt.color.g));
                obj["color_b"] = make_int_value(static_cast<int64_t>(txt.color.b));
                obj["color_a"] = make_int_value(static_cast<int64_t>(txt.color.a));
                val.data = std::move(obj);
                return val;
            });

        // Map ship entity to logical ID
        ent_mapper->map(1, ship);

        // Create BlackboardAccessor and JSONSerializer
        bb_accessor = std::make_unique<BlackboardAccessor>(blackboard);
        json_serializer = std::make_unique<engine::debug::JSONSerializer>(
            *prop_accessor, *introspector, *ent_mapper, bb_accessor.get());
    }

    std::cout << "Asteroids initialized:" << std::endl;
    std::cout << "  Active entities: " << entity_manager.active_count() << std::endl;
    std::cout << "  Ship entity ID: " << ship << std::endl;
    std::cout << "  Target FPS: " << (opts.fps > 0 ? opts.fps : 60) << std::endl;
    std::cout << "  Window: 800x600" << std::endl;
    std::cout << "  Coordinate system: World space (0,0 = center), Y up" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Left/Right arrows - Rotate ship" << std::endl;
    std::cout << "  Up arrow          - Thrust forward" << std::endl;
    std::cout << "  H                 - Toggle HUD visibility" << std::endl;
    std::cout << "  F1                - Toggle debug pause" << std::endl;
    std::cout << "  F2                - Single step (while paused)" << std::endl;
    std::cout << "  J                 - Dump this frame (before/after)" << std::endl;
    std::cout << "  T                 - Trace this frame (per-system)" << std::endl;
    std::cout << "  F10               - Toggle debug HUD" << std::endl;
    std::cout << "  ESC / Close       - Exit" << std::endl;

    // === Main Loop ===

    bool running = true;

    while (running) {
        // 1. Start frame timing
        timer.start_frame();

        // 2. Process input events and update Input components
        input_system.process_events(component_storage, running);

        // 3. H key toggle for HUD visibility (edge detection via keyboard state)
        {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            static bool h_was_pressed = false;
            bool h_is_pressed = keys[SDL_SCANCODE_H];
            if (h_is_pressed && !h_was_pressed) {
                bool hud_visible = blackboard.get_or<bool>("hud_visible", true);
                blackboard.set("hud_visible", !hud_visible);
            }
            h_was_pressed = h_is_pressed;
        }

        // Debug key processing (edge detection via keyboard state)
        {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            bool f1_is_pressed = keys[SDL_SCANCODE_F1];
            bool f2_is_pressed = keys[SDL_SCANCODE_F2];
            bool f10_is_pressed = keys[SDL_SCANCODE_F10];

            // F1: toggle pause
            if (should_toggle(f1_was_pressed, f1_is_pressed)) {
                apply_pause_toggle(debug_paused, debug_hud_visible);
            }

            // F10: toggle debug HUD visibility
            if (should_toggle(f10_was_pressed, f10_is_pressed)) {
                debug_hud_visible = !debug_hud_visible;
            }

            // F2: single step (only when paused)
            if (should_step(debug_paused, f2_was_pressed, f2_is_pressed)) {
                step_requested = true;
            }

            // J: dump this frame, T: trace this frame (interactive, one-shot).
            // Arms the capture and, when paused, forces exactly one frame to advance.
            // Guarded by json_serializer so J/T are no-ops under --no-debug-keys.
            bool j_is_pressed = keys[SDL_SCANCODE_J];
            bool t_is_pressed = keys[SDL_SCANCODE_T];
            if (should_toggle(j_was_pressed, j_is_pressed) && json_serializer) {
                force_dump_this_step = true;
                if (debug_paused) step_requested = true;
            }
            if (should_toggle(t_was_pressed, t_is_pressed) && json_serializer) {
                force_trace_this_step = true;
                if (debug_paused) step_requested = true;
            }

            f1_was_pressed = f1_is_pressed;
            f2_was_pressed = f2_is_pressed;
            f10_was_pressed = f10_is_pressed;
            j_was_pressed = j_is_pressed;
            t_was_pressed = t_is_pressed;
        }

        // --- Key action injection from --keys CLI option ---
        // Clear arrow key flags injected on the previous frame (single-frame press)
        if (injected_arrow_last_frame) {
            auto entities = component_storage.entities_with_component<Input>();
            for (Entity e : entities) {
                auto input_opt = component_storage.get_component<Input>(e);
                if (input_opt.has_value()) {
                    Input& input = input_opt->get();
                    input.left = false;
                    input.right = false;
                    input.up = false;
                    input.down = false;
                    input.fire = false;
                }
            }
            injected_arrow_last_frame = false;
        }

        // Inject synthetic key presses for the current frame
        {
            uint64_t current_frame = timer.get_frame_count();
            bool arrow_injected = false;
            for (const auto& ka : opts.keys) {
                if (ka.frame != current_frame) continue;

                // Arrow keys: set Input component flags
                if (ka.key == "LEFT" || ka.key == "RIGHT" ||
                    ka.key == "UP" || ka.key == "DOWN") {
                    auto entities = component_storage.entities_with_component<Input>();
                    for (Entity e : entities) {
                        auto input_opt = component_storage.get_component<Input>(e);
                        if (input_opt.has_value()) {
                            Input& input = input_opt->get();
                            if (ka.key == "LEFT")  input.left = true;
                            if (ka.key == "RIGHT") input.right = true;
                            if (ka.key == "UP")    input.up = true;
                            if (ka.key == "DOWN")  input.down = true;
                        }
                    }
                    arrow_injected = true;
                }
                // Debug/control keys: simulate their effect directly
                else if (ka.key == "F1") {
                    apply_pause_toggle(debug_paused, debug_hud_visible);
                }
                else if (ka.key == "F2") {
                    if (debug_paused) {
                        step_requested = true;
                    }
                }
                else if (ka.key == "F10") {
                    debug_hud_visible = !debug_hud_visible;
                }
                else if (ka.key == "H") {
                    bool hud_visible = blackboard.get_or<bool>("hud_visible", true);
                    blackboard.set("hud_visible", !hud_visible);
                }
                else if (ka.key == "J") {
                    // Dump this frame (one-shot); advance one frame when paused.
                    if (json_serializer) {
                        force_dump_this_step = true;
                        if (debug_paused) step_requested = true;
                    }
                }
                else if (ka.key == "T") {
                    // Trace this frame (one-shot); advance one frame when paused.
                    if (json_serializer) {
                        force_trace_this_step = true;
                        if (debug_paused) step_requested = true;
                    }
                }
                else if (ka.key == "ESC") {
                    running = false;
                }
                else if (ka.key == "SPACE") {
                    auto entities = component_storage.entities_with_component<Input>();
                    for (Entity e : entities) {
                        auto input_opt = component_storage.get_component<Input>(e);
                        if (input_opt.has_value()) {
                            input_opt->get().fire = true;
                        }
                    }
                    arrow_injected = true;  // reuse the same clear mechanism
                }
            }
            injected_arrow_last_frame = arrow_injected;
        }

        // Capture step_requested before simulation clears it (for timer logic)
        bool step_executed = step_requested;

        // Determine if this frame is a dump or trace frame
        uint64_t frame_for_dump_trace = timer.get_frame_count();
        // Frame-number-driven matching OR an interactive J/T capture armed this frame.
        bool is_dump_frame = (dump_trace_enabled &&
            std::find(opts.dump_frames.begin(), opts.dump_frames.end(), frame_for_dump_trace) != opts.dump_frames.end())
            || force_dump_this_step;
        bool is_trace_frame = (dump_trace_enabled &&
            std::find(opts.trace_frames.begin(), opts.trace_frames.end(), frame_for_dump_trace) != opts.trace_frames.end())
            || force_trace_this_step;

        // Simulation systems — skipped when paused, unless stepping
        if (!debug_paused || step_requested) {
            // Override delta_time for step frames (deterministic)
            if (step_requested) {
                blackboard.set("delta_time", step_delta_time(60.0));
            }

            // --- Frame trace logic ---
            if (is_trace_frame) {
                // Create trace subdirectory
                std::ostringstream trace_dir_ss;
                trace_dir_ss << log_dir << "/trace-"
                             << std::setw(6) << std::setfill('0') << frame_for_dump_trace;
                std::string trace_dir = trace_dir_ss.str();
                std::filesystem::create_directories(trace_dir);

                // Write 00-before.json (state before any systems)
                {
                    ent_mapper->map_all_live_entities(component_storage);
                    std::string json = json_serializer->serialize_state(
                        frame_for_dump_trace, "game", "050-asteroids", "before", component_storage);
                    std::ofstream out(trace_dir + "/00-before.json");
                    out << json;
                }

                // System names and execution lambdas (matching simulation order)
                struct SystemStep {
                    std::string name;
                    std::function<void()> run;
                };

                std::string game_state = blackboard.get_or<std::string>("game.state", std::string("PLAYING"));
                if (game_state != "GAME_OVER") {

                std::vector<SystemStep> system_steps = {
                    {"GameStateSystem", [&]() {
                        game_state_system.update(component_storage, blackboard);
                    }},
                    {"ShipControlSystem", [&]() {
                        ship_control_system.update(component_storage, blackboard);
                    }},
                    {"HyperspaceSystem", [&]() {
                        hyperspace_system.update(component_storage, blackboard);
                    }},
                    {"LaserSpawnSystem", [&]() {
                        laser_spawn_system.update(component_storage, blackboard, entity_manager);
                    }},
                    {"BulletSpawnSystem", [&]() {
                        bullet_spawn_system.update(component_storage, blackboard, entity_manager);
                    }},
                    {"MovementSystem", [&]() {
                        movement_system.update(component_storage, blackboard);
                    }},
                    {"DirectionSystem", [&]() {
                        direction_system.update(component_storage, blackboard);
                    }},
                    {"WrapSystem", [&]() {
                        wrap_system.update(component_storage, blackboard);
                    }},
                    {"CollisionSystem", [&]() {
                        collision_system.update(component_storage);
                    }},
                    {"CollisionResponseSystem", [&]() {
                        collision_response_system.update(component_storage, blackboard);
                    }},
                    {"AbductionSystem", [&]() {
                        abduction_system.update(component_storage, blackboard, entity_manager);
                    }},
                    {"AlienSpawnSystem", [&]() {
                        alien_spawn_system.update(component_storage, blackboard, entity_manager);
                    }},
                    {"AstronautSpawnSystem", [&]() {
                        astronaut_spawn_system.update(component_storage, blackboard, entity_manager);
                    }},
                    {"CleanupSystem", [&]() {
                        cleanup_system.update(component_storage, blackboard, entity_manager);
                    }},
                    {"LifetimeSystem", [&]() {
                        lifetime_system.update(component_storage, blackboard);
                    }},
                    {"ScriptSystem", [&]() {
                        script_system.update(component_storage, entity_manager, blackboard);
                    }},
                };

                // Run each system individually with intermediate snapshots
                std::vector<std::string> system_names;
                for (size_t i = 0; i < system_steps.size(); ++i) {
                    system_steps[i].run();
                    system_names.push_back(system_steps[i].name);

                    // Write NN-SystemName.json
                    std::ostringstream step_filename;
                    step_filename << trace_dir << "/"
                                  << std::setw(2) << std::setfill('0') << (i + 1)
                                  << "-" << system_steps[i].name << ".json";
                    ent_mapper->map_all_live_entities(component_storage);
                    std::string json = json_serializer->serialize_state(
                        frame_for_dump_trace, "game", "050-asteroids",
                        system_steps[i].name, component_storage);
                    std::ofstream out(step_filename.str());
                    out << json;
                }

                // Write summary.json
                size_t total_files = system_steps.size() + 2;  // before + per-system + summary
                std::string summary = json_serializer->serialize_trace_summary(
                    frame_for_dump_trace, "game", "050-asteroids",
                    system_names, total_files,
                    current_timestamp());
                {
                    std::ofstream out(trace_dir + "/summary.json");
                    out << summary;
                }

                } // end if (game_state != "GAME_OVER")

                // Destroy entities marked with DestroyRequest
                destroy_marked_entities(entity_manager, component_storage);

                if (opts.verbose) {
                    std::cout << "  [Trace] Frame " << frame_for_dump_trace
                              << " → " << trace_dir << std::endl;
                }
            }
            // --- Frame dump logic ---
            else if (is_dump_frame) {
                // Serialize before-state
                ent_mapper->map_all_live_entities(component_storage);
                std::string before_json = json_serializer->serialize_state(
                    frame_for_dump_trace, "game", "050-asteroids", "before", component_storage);

                // Run all simulation systems normally
                std::string game_state = blackboard.get_or<std::string>("game.state", std::string("PLAYING"));
                if (game_state != "GAME_OVER") {
                    game_state_system.update(component_storage, blackboard);
                    ship_control_system.update(component_storage, blackboard);
                    hyperspace_system.update(component_storage, blackboard);
                    laser_spawn_system.update(component_storage, blackboard, entity_manager);
                    movement_system.update(component_storage, blackboard);
                    direction_system.update(component_storage, blackboard);
                    wrap_system.update(component_storage, blackboard);
                    collision_system.update(component_storage);
                    collision_response_system.update(component_storage, blackboard);
                    abduction_system.update(component_storage, blackboard, entity_manager);
                    astronaut_spawn_system.update(component_storage, blackboard, entity_manager);
                    alien_spawn_system.update(component_storage, blackboard, entity_manager);
                    cleanup_system.update(component_storage, blackboard, entity_manager);
                    lifetime_system.update(component_storage, blackboard);
                    script_system.update(component_storage, entity_manager, blackboard);
                } // end if (game_state != "GAME_OVER")
                destroy_marked_entities(entity_manager, component_storage);

                // Serialize after-state
                ent_mapper->map_all_live_entities(component_storage);
                std::string after_json = json_serializer->serialize_state(
                    frame_for_dump_trace, "game", "050-asteroids", "after", component_storage);

                // Write both files to log directory (6-digit zero-padded)
                std::ostringstream before_path, after_path;
                before_path << log_dir << "/"
                            << std::setw(6) << std::setfill('0') << frame_for_dump_trace
                            << "-before.json";
                after_path << log_dir << "/"
                           << std::setw(6) << std::setfill('0') << frame_for_dump_trace
                           << "-after.json";
                {
                    std::ofstream out(before_path.str());
                    out << before_json;
                }
                {
                    std::ofstream out(after_path.str());
                    out << after_json;
                }

                if (opts.verbose) {
                    std::cout << "  [Dump] Frame " << frame_for_dump_trace
                              << " → " << before_path.str() << ", " << after_path.str() << std::endl;
                }
            }
            // --- Normal simulation (no dump/trace) ---
            else {
                std::string game_state = blackboard.get_or<std::string>("game.state", std::string("PLAYING"));
                if (game_state != "GAME_OVER" && game_state != "WIN") {
                    // 1. Update game state
                    game_state_system.update(component_storage, blackboard);

                    // 2. Ship controls: translate input to rotation + thrust
                    ship_control_system.update(component_storage, blackboard);
                    hyperspace_system.update(component_storage, blackboard);

                    // 3. Spawn lasers from fire input
                    laser_spawn_system.update(component_storage, blackboard, entity_manager);

                    // 3.5. Spawn bullets from fire input
                    bullet_spawn_system.update(component_storage, blackboard, entity_manager);

                    // 4. Update positions based on velocity and delta_time
                    movement_system.update(component_storage, blackboard);

                    // 5. Update direction based on velocity
                    direction_system.update(component_storage, blackboard);

                    // 6. Wrap entities at world boundaries
                    wrap_system.update(component_storage, blackboard);

                    // 7. Collision detection
                    collision_system.update(component_storage);

                    // 8. Collision response: score, lives, split candidates
                    collision_response_system.update(component_storage, blackboard);

                    // 9. Abduction: process abductions
                    abduction_system.update(component_storage, blackboard, entity_manager);

                    // 10. Astronaut spawning: spawn astronauts
                    astronaut_spawn_system.update(component_storage, blackboard, entity_manager);

                    // 11. Alien spawning: spawn waves
                    alien_spawn_system.update(component_storage, blackboard, entity_manager);

                    // 12. Decrement lifetimes, mark expired entities
                    cleanup_system.update(component_storage, blackboard, entity_manager);
                    lifetime_system.update(component_storage, blackboard);

                    // 13. Update scripts
                    script_system.update(component_storage, entity_manager, blackboard);
                } // end if (game_state != "GAME_OVER")

                // 14. Destroy entities marked with DestroyRequest
                destroy_marked_entities(entity_manager, component_storage);
            }

            // Clear step flag after simulation completes
            if (step_requested) {
                apply_step_complete(step_requested, debug_paused);
            }

            // Clear interactive capture flags — one-shot per J/T press.
            force_dump_this_step = false;
            force_trace_this_step = false;
        }

        // Always-run systems (camera, rendering, HUD) — execute every frame
        // CameraControlSystem for debug zoom/pan (+/-, WASD)
        camera_control_system.update(blackboard);

        // CameraSystem for debug panning
        camera_system.update(component_storage, blackboard);

        // Render frame
        render_system.clear_background();
        render_system.render(component_storage, blackboard);
        render_system.draw_world_border(blackboard);
        hud_update_system.update(component_storage, blackboard);
        hud_system.render(component_storage, blackboard);
        debug_hud_system.render(debug_hud_visible, debug_paused, timer.get_frame_count());

        // Screenshot capture BEFORE present — after present, the back buffer
        // contents are undefined (the swap chain has rotated to a new buffer)
        // and SDL_RenderReadPixels would return stale data from a previous frame.
        if (!opts.screenshot_frames.empty()) {
            uint64_t cur = timer.get_frame_count();
            if (std::find(opts.screenshot_frames.begin(), opts.screenshot_frames.end(), cur)
                    != opts.screenshot_frames.end()) {
                blackboard.set("screenshot_frame", cur);
            }
        }
        screenshot_system.update(blackboard);

        render_system.present();

        // Conditional timer end-frame
        if (!debug_paused || step_executed) {
            timer.end_frame();
        } else {
            timer.end_frame_no_advance();
        }
        timer.update_blackboard(blackboard);

        // Stop-frame check
        if (opts.stop_frame.has_value() &&
            timer.get_frame_count() >= opts.stop_frame.value()) {
            running = false;
        }

        // Verbose per-frame logging
        if (opts.verbose) {
            std::cout << "[Frame " << timer.get_frame_count() << "] "
                      << "dt=" << std::fixed << std::setprecision(6)
                      << blackboard.get_or<double>("delta_time", 0.0)
                      << " fps=" << std::fixed << std::setprecision(1)
                      << timer.get_fps()
                      << " paused=" << (debug_paused ? "true" : "false")
                      << std::endl;
        }
    }

    // Cleanup
    std::cout << std::endl;
    std::cout << "Shutting down..." << std::endl;
    std::cout << "  Total frames: " << timer.get_frame_count() << std::endl;
    std::cout << "  Final FPS: " << timer.get_fps() << std::endl;

    // Destroy SDL-backed resources BEFORE quitting the subsystems so every
    // deleter (SDL_DestroyTexture/TTF_CloseFont via ResourceManager, then
    // SDL_DestroyRenderer/SDL_DestroyWindow via the smart pointers) runs while
    // SDL/TTF are still initialized.
    // Order: resource_manager → renderer → window → TTF_Quit → SDL_Quit.
    resource_manager_ptr.reset();
    renderer.reset();
    window.reset();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
