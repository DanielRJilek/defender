#include "engine/gamedata_loader.hpp"
#include <nlohmann/json.hpp>
#include <array>
#include <fstream>
#include <stdexcept>
#include "game/wave_config.hpp"
#include "game/level_config.hpp"

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Helper: parse a single entity's "components" object and attach to ECS
// ---------------------------------------------------------------------------
static void parse_components(const json& components_obj,
                             Entity entity,
                             ComponentStorage& component_storage) {
    for (auto& [key, value] : components_obj.items()) {
        if (key == "position") {
            component_storage.add_component<Position>(entity, Position{
                value["x"].get<float>(),
                value["y"].get<float>()
            });
        } else if (key == "size") {
            component_storage.add_component<Size>(entity, Size{
                value["width"].get<float>(),
                value["height"].get<float>()
            });
        } else if (key == "color") {
            component_storage.add_component<Color>(entity, Color{
                value["r"].get<uint8_t>(),
                value["g"].get<uint8_t>(),
                value["b"].get<uint8_t>(),
                value["a"].get<uint8_t>()
            });
        } else if (key == "velocity") {
            component_storage.add_component<Velocity>(entity, Velocity{
                value["dx"].get<float>(),
                value["dy"].get<float>()
            });
        } else if (key == "input") {
            component_storage.add_component<Input>(entity, Input{});
        } else if (key == "images") {
            component_storage.add_component<Images>(entity, Images{
                value["names"].get<std::vector<std::string>>(),
                value["active_index"].get<size_t>()
            });
        } else if (key == "text") {
            auto& color_obj = value["color"];
            SDL_Color sdl_color{
                color_obj["r"].get<uint8_t>(),
                color_obj["g"].get<uint8_t>(),
                color_obj["b"].get<uint8_t>(),
                color_obj["a"].get<uint8_t>()
            };
            component_storage.add_component<Text>(entity, Text{
                value["content"].get<std::string>(),
                value["font_name"].get<std::string>(),
                value["font_size"].get<float>(),
                sdl_color
            });
        } else if (key == "screen_position") {
            component_storage.add_component<ScreenPosition>(entity, ScreenPosition{
                value["x"].get<float>(),
                value["y"].get<float>()
            });
        }
        else if (key == "script") {
            component_storage.add_component<Script>(entity, Script{
                value["filename"].get<std::string>(),
                false
            });
        }
        // Unrecognized keys are silently skipped
    }
}

// ---------------------------------------------------------------------------
// Helper: process an entity array ("entities" or "hud_entities")
// ---------------------------------------------------------------------------
static void process_entity_array(const json& arr,
                                 const std::string& array_name,
                                 EntityManager& entity_manager,
                                 ComponentStorage& component_storage,
                                 Blackboard& blackboard) {
    for (size_t i = 0; i < arr.size(); ++i) {
        const auto& element = arr[i];

        if (!element.contains("id")) {
            throw std::runtime_error(
                "Entity at index " + std::to_string(i) +
                " in '" + array_name + "' is missing 'id' field");
        }

        std::string id = element["id"].get<std::string>();
        Entity entity = entity_manager.create_entity();
        blackboard.set<Entity>("entity.id." + id, entity);

        if (element.contains("components")) {
            parse_components(element["components"], entity, component_storage);
        }
    }
}

// ---------------------------------------------------------------------------
// load_game_data
// ---------------------------------------------------------------------------
void load_game_data(const std::string& file_path,
                    EntityManager& entity_manager,
                    ComponentStorage& component_storage,
                    Blackboard& blackboard) {
    // 1. Read file
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + file_path);
    }

    // 2. Parse JSON
    json data;
    try {
        data = json::parse(file);
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }

    // 3. Window configuration
    if (data.contains("window")) {
        const auto& win = data["window"];
        blackboard.set<int>("window_width", win["width"].get<int>());
        blackboard.set<int>("window_height", win["height"].get<int>());
    }

    // 4. Camera configuration (defaults if absent)
    if (data.contains("camera")) {
        const auto& cam = data["camera"];
        blackboard.set<float>("camera.lookat.x", cam.value("lookat_x", 0.0f));
        blackboard.set<float>("camera.lookat.y", cam.value("lookat_y", 0.0f));
        blackboard.set<float>("camera.zoom", cam.value("zoom", 1.0f));
    } else {
        blackboard.set<float>("camera.lookat.x", 0.0f);
        blackboard.set<float>("camera.lookat.y", 0.0f);
        blackboard.set<float>("camera.zoom", 1.0f);
    }

    // 4.5 World bounds (optional — used for debug border rendering)
    if (data.contains("world")) {
        const auto& world = data["world"];
        blackboard.set<float>("world.x", world.value("x", 0.0f));
        blackboard.set<float>("world.y", world.value("y", 0.0f));
        blackboard.set<float>("world.width", world.value("width", 0.0f));
        blackboard.set<float>("world.height", world.value("height", 0.0f));
    }

    // 4.8 Atlas configuration (optional)
    if (data.contains("atlas")) {
        const auto& atlas = data["atlas"];
        blackboard.set<std::string>("atlas.filename", atlas["filename"].get<std::string>());
        for (auto& [key, value] : atlas.items()) {
            if (!value.is_object()) continue;
            blackboard.set<int>("atlas." + key + ".frame_width", value["frame_width"].get<int>());
            blackboard.set<int>("atlas." + key + ".frame_height", value["frame_height"].get<int>());
            blackboard.set<int>("atlas." + key + ".columns", value["columns"].get<int>());
            blackboard.set<int>("atlas." + key + ".start_height", value["start_height"].get<int>());
        }
    }

    // 4.12 Animation definitions (optional)
    if (data.contains("animation_definitions")) {
        const auto& anim_defs = data["animation_definitions"];
        for (auto& [entity, states_obj] : anim_defs.items()) {
            for (auto& [state_name, params] : states_obj.items()) {
                std::string prefix = "anim_def." + entity + "." + state_name;
                blackboard.set<int>(prefix + ".start_frame", params["start_frame"].get<int>());
                blackboard.set<int>(prefix + ".frame_count", params["frame_count"].get<int>());
                blackboard.set<float>(prefix + ".frame_duration", params["frame_duration"].get<float>());
                blackboard.set<bool>(prefix + ".looping", params["looping"].get<bool>());
                blackboard.set<std::string>(prefix + ".row", params["row"].get<std::string>());
            }
        }
    }

    // 4.13 Initial animation states (optional)
    if (data.contains("initial_animation_states")) {
        const auto& initial_states = data["initial_animation_states"];
        for (auto& [entity, state] : initial_states.items()) {
            blackboard.set<std::string>("anim_def." + entity + ".initial_state", state.get<std::string>());
        }
    }

    // 4.6 Debug configuration (optional)
    if (data.contains("debug")) {
        const auto& debug = data["debug"];
        blackboard.set<int>("debug.overlay_alpha", debug.value("overlay_alpha", 128));
    }

    // 5. Entities
    if (data.contains("entities")) {
        process_entity_array(data["entities"], "entities",
                             entity_manager, component_storage, blackboard);
    }

    // 6. HUD entities
    if (data.contains("hud_entities")) {
        process_entity_array(data["hud_entities"], "hud_entities",
                             entity_manager, component_storage, blackboard);
    }

    // 7. Load ship entity from "ship" object
    if (data.contains("ship")) {
        auto& ship = data["ship"];
        auto& size = ship["size"];
        float width = size["width"].get<float>();
        float height = size["height"].get<float>();
        float thrust = ship.value("thrust", 200.0f);
        int layer = ship.value("layer", 1);
        int mask = ship.value("mask", 14);

        // Read world bounds for centering (already loaded above)
        float world_x = blackboard.get_or<float>("world.x", 0.0f);
        float world_y = blackboard.get_or<float>("world.y", 0.0f);
        float world_w = blackboard.get_or<float>("world.width", 800.0f);
        float world_h = blackboard.get_or<float>("world.height", 600.0f);

        Entity ship_entity = entity_manager.create_entity();

        // Position centered in world
        float half_size = static_cast<float>(width) / 2.0f;
        component_storage.add_component(ship_entity, Position{
            world_x + world_w / 2.0f - half_size,
            world_y + world_h / 2.0f - half_size
        });
        component_storage.add_component(ship_entity, Size{
            width, height
        });
        component_storage.add_component(ship_entity, Velocity{200.0f, 0.0f});
        component_storage.add_component(ship_entity, Input{});
        component_storage.add_component(ship_entity, WrapAround{});
        component_storage.add_component(ship_entity, Collider{
            width, height,
            static_cast<uint8_t>(layer), static_cast<uint8_t>(mask)
        });
        if (ship.contains("images")) {
            auto& img = ship["images"];
            component_storage.add_component(ship_entity, Images{
                img["names"].get<std::vector<std::string>>(),
                img.value("active_index", size_t{0})
            });
        }

        // Attach ShipTag to the ship entity
        component_storage.add_component(ship_entity, ShipTag{});

        component_storage.add_component(ship_entity, Direction{1});
        

        // Store ship config on Blackboard
        blackboard.set("ship.thrust", thrust);
        blackboard.set("entity.id.ship", ship_entity);
    }

    // 8. Load laser configuration from "laser" object
    if (data.contains("laser")) {
        auto& laser = data["laser"];
        blackboard.set("laser.speed", laser.value("speed", 400.0f));
        blackboard.set("laser.lifetime", laser.value("lifetime", 2.0f));
        blackboard.set("laser.size", Size{
            laser["size"]["width"].get<float>(),
            laser["size"]["height"].get<float>()
        });
        blackboard.set("laser.max_live", laser.value("max_live", 6));
        blackboard.set("laser.fire_cooldown", laser.value("fire_cooldown", 0.25f));
        blackboard.set("laser.layer", laser.value("layer", 2));
        blackboard.set("laser.mask", laser.value("mask", 4));
        if (laser.contains("images")) {
            auto& img = laser["images"];
            blackboard.set("laser.images", Images{
                img["names"].get<std::vector<std::string>>(),
                img.value("active_index", size_t{0})
            });
        }
    }

    // 8.5 Load bullet configuration from "bullet" object
    if (data.contains("bullet")) {
        auto& bullet = data["bullet"];
        blackboard.set("bullet.speed", bullet.value("speed", 600.0f));
        blackboard.set("bullet.lifetime", bullet.value("lifetime", 0.8f));
        blackboard.set("bullet.size", bullet.value("size", 4.0f));
        blackboard.set("bullet.max_live", bullet.value("max_live", 6));
        blackboard.set("bullet.fire_cooldown", bullet.value("fire_cooldown", 0.25f));
        blackboard.set("bullet.layer", bullet.value("layer", 2));
        blackboard.set("bullet.mask", bullet.value("mask", 4));
    }

    // 9. Load alien configuration from "aliens" object
    if (data.contains("aliens")) {
        auto& aliens = data["aliens"];
        blackboard.set("alien.lander.size", Size{
            aliens["lander"]["size"]["width"].get<float>(),
            aliens["lander"]["size"]["height"].get<float>()
        });
        blackboard.set("alien.swarmer.size", Size{
            aliens["swarmer"]["size"]["width"].get<float>(),
            aliens["swarmer"]["size"]["height"].get<float>()
        });
        blackboard.set("alien.baiter.size", Size{
            aliens["baiter"]["size"]["width"].get<float>(),
            aliens["baiter"]["size"]["height"].get<float>()
        });
        blackboard.set("alien.lander.speed", aliens["lander"].value("speed", 50.0f));
        blackboard.set("alien.swarmer.speed", aliens["swarmer"].value("speed", 75.0f));
        blackboard.set("alien.baiter.speed", aliens["baiter"].value("speed", 100.0f));
        blackboard.set("alien.lander.points", aliens["lander"].value("points", 150));
        blackboard.set("alien.swarmer.points", aliens["swarmer"].value("points", 150));
        blackboard.set("alien.baiter.points", aliens["baiter"].value("points", 200));
        for (auto& [key, value] : aliens.items()) {
            if (key == "lander" || key == "swarmer" || key == "baiter") {
                if (value.contains("images")) {
                    auto& img = value["images"];
                    blackboard.set("alien." + key + ".images", Images{
                        img["names"].get<std::vector<std::string>>(),
                        img.value("active_index", size_t{0})
                    });
                }
                else if (value.contains("texture")) {
                    blackboard.set("alien." + key + ".images", Images{
                        {value["texture"].get<std::string>()}, 0
                    });
                }
                if (value.contains("script")) {
                    std::string filename = value["script"].value("filename", "wander");
                    blackboard.set("alien." + key + ".script", Script{filename, false});
                }
                if (value.contains("animations")) {
                    blackboard.set("alien." + key + ".animations", value["animations"].get<std::vector<std::string>>());
                }
            }
        }
    }

    // 10. Load astronaut configuration from "astronaut" object
    if (data.contains("astronaut")) {
        auto& astronaut = data["astronaut"];
        blackboard.set("astronaut.size", Size{
            astronaut["size"]["width"].get<float>(),
            astronaut["size"]["height"].get<float>()
        });
        blackboard.set("astronaut.speed", astronaut.value("speed", 100.0f));
        blackboard.set("astronaut.points", astronaut.value("points", 200));
        if (astronaut.contains("images")) {
            auto& img = astronaut["images"];
            blackboard.set("astronaut.images", Images{
                img["names"].get<std::vector<std::string>>(),
                img.value("active_index", size_t{0})
            });
        }
        if (astronaut.contains("script")) {
            std::string filename = astronaut["script"].value("filename", "wander");
            if (filename.size() > 4 &&
                filename.substr(filename.size() - 4) == ".lua") {
                filename = filename.substr(0, filename.size() - 4);
            }
            blackboard.set("astronaut.script", Script{filename, false});
        }
    }
    
    // 4.14 Game configuration (optional — wave spawning and player lives)
    {
        auto level_configs = std::make_shared<std::vector<LevelConfig>>();

        if (data.contains("game")) {
            const auto& game_obj = data["game"];

            if (game_obj.contains("levels")) {
                const auto& levels_arr = game_obj["levels"];
                for (size_t i = 0; i < levels_arr.size(); ++i) {
                    const auto& l = levels_arr[i];
                    if (l.contains("waves")) {
                        std::vector<WaveConfig> wave_configs;
                        const auto& waves_arr = l["waves"];
                        for (size_t j = 0; j < waves_arr.size(); ++j) {
                            const auto& w = waves_arr[j];
                            WaveConfig wc;
                            wc.lander_count = w.value("lander_count", 5);
                            wc.swarmer_count = w.value("swarmer_count", 0);
                            wc.baiter_count = w.value("baiter_count", 0);
                            wc.elapsed_time = 0.0f;
                            wc.last_baiter_timer = 0.0f;
                            // Validate wave parameters
                            if (wc.lander_count <= 0 && wc.swarmer_count <= 0 && wc.baiter_count <= 0) {
                                throw std::runtime_error(
                                    "Wave " + std::to_string(i) + ": at least one of lander_count, swarmer_count, or baiter_count must be > 0");
                            }

                            wave_configs.push_back(wc);
                        }
                        LevelConfig lc;
                        lc.waves = wave_configs;
                        lc.astronaut_count = l.value("astronaut_count", 0);
                        lc.elapsed_time = l.value("elapsed_time", 0.0f);
                        lc.wave_time_limit = l.value("wave_time_limit", 10.0f);
                        lc.spawned_baiters = l.value("spawned_baiters", 0);
                        level_configs->push_back(lc);
                    }
                }
            }
        } 

        blackboard.set<std::shared_ptr<std::vector<LevelConfig>>>("level_configs", level_configs);
    }

    // 11. Load lives and initialize game state
    blackboard.set("game.lives", data.value("lives", 3));
    blackboard.set("game.score", 0);
    blackboard.set("game.wave", 0);
    blackboard.set("game.level", 0);
    blackboard.set("game.state", std::string("GAME_START"));
    blackboard.set("game.spawn_delay", 2.0f);
    blackboard.set("game.spawn_delay_timer", 0.0f);
    blackboard.set("game.saved_score", 0);
    blackboard.set("game.hyperspace_state", std::string("INACTIVE"));
    blackboard.set("game.hyperspace_uses", 1);
}

// ---------------------------------------------------------------------------
// serialize_game_data
// ---------------------------------------------------------------------------
std::string serialize_game_data(
    const ComponentStorage& component_storage,
    const Blackboard& blackboard,
    const std::vector<std::pair<std::string, Entity>>& entity_ids) {

    json root;

    // Window
    root["window"] = {
        {"width",  blackboard.get<int>("window_width")},
        {"height", blackboard.get<int>("window_height")}
    };

    // Camera
    root["camera"] = {
        {"lookat_x", blackboard.get<float>("camera.lookat.x")},
        {"lookat_y", blackboard.get<float>("camera.lookat.y")},
        {"zoom",     blackboard.get<float>("camera.zoom")}
    };

    json entities_arr = json::array();
    json hud_arr      = json::array();

    for (const auto& [id, entity] : entity_ids) {
        json entry;
        entry["id"] = id;
        json comps = json::object();

        // Determine if this is a HUD entity (has both Text and ScreenPosition)
        bool is_hud = component_storage.has_component<Text>(entity) &&
                      component_storage.has_component<ScreenPosition>(entity);

        // Position
        if (component_storage.has_component<Position>(entity)) {
            auto pos = component_storage.get_component<Position>(entity);
            comps["position"] = {{"x", pos->get().x}, {"y", pos->get().y}};
        }

        // Size
        if (component_storage.has_component<Size>(entity)) {
            auto sz = component_storage.get_component<Size>(entity);
            comps["size"] = {{"width", sz->get().width}, {"height", sz->get().height}};
        }

        // Color
        if (component_storage.has_component<Color>(entity)) {
            auto c = component_storage.get_component<Color>(entity);
            comps["color"] = {
                {"r", c->get().r}, {"g", c->get().g},
                {"b", c->get().b}, {"a", c->get().a}
            };
        }

        // Velocity
        if (component_storage.has_component<Velocity>(entity)) {
            auto v = component_storage.get_component<Velocity>(entity);
            comps["velocity"] = {{"dx", v->get().dx}, {"dy", v->get().dy}};
        }

        // Input
        if (component_storage.has_component<Input>(entity)) {
            comps["input"] = json::object();
        }

        // Images
        if (component_storage.has_component<Images>(entity)) {
            auto img = component_storage.get_component<Images>(entity);
            comps["images"] = {
                {"names",        img->get().filenames},
                {"active_index", img->get().active_index}
            };
        }

        // Text
        if (component_storage.has_component<Text>(entity)) {
            auto t = component_storage.get_component<Text>(entity);
            comps["text"] = {
                {"content",   t->get().content},
                {"font_name", t->get().font_name},
                {"font_size", t->get().font_size},
                {"color", {
                    {"r", t->get().color.r}, {"g", t->get().color.g},
                    {"b", t->get().color.b}, {"a", t->get().color.a}
                }}
            };
        }

        // ScreenPosition
        if (component_storage.has_component<ScreenPosition>(entity)) {
            auto sp = component_storage.get_component<ScreenPosition>(entity);
            comps["screen_position"] = {{"x", sp->get().x}, {"y", sp->get().y}};
        }

        entry["components"] = comps;

        if (is_hud) {
            hud_arr.push_back(entry);
        } else {
            entities_arr.push_back(entry);
        }
    }

    root["entities"]     = entities_arr;
    root["hud_entities"] = hud_arr;

    return root.dump(2);
}
