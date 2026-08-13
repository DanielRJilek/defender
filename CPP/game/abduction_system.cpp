#include "abduction_system.hpp"

AbductionSystem::AbductionSystem(AlienSpawnSystem& alien_spawn_system)
    : alien_spawn_system_(alien_spawn_system) {}

bool AbductionSystem::astronaut_reached_bottom(ComponentStorage& storage, Entity abducted_entity) {
    auto pos_opt = storage.get_component<Position>(abducted_entity);
    if (!pos_opt.has_value()) {
        return false;
    }
    
    Position& pos = pos_opt->get();
    return pos.y < -100;
}

void AbductionSystem::move_hanging_entity_to_holder(ComponentStorage& storage, Entity abducted_entity, Entity abductor) {
    // Move the abducted entity to directly beneath the abductor
    auto abductor_pos_opt = storage.get_component<Position>(abductor);
    if (!abductor_pos_opt.has_value()) {
        return;
    }
    auto abductor_size_opt = storage.get_component<Size>(abductor);
    if (!abductor_size_opt.has_value()) {
        return;
    }
    auto astronaut_pos_opt = storage.get_component<Position>(abducted_entity);
    if (!astronaut_pos_opt.has_value()) {
        return;
    }
    auto astronaut_size_opt = storage.get_component<Size>(abducted_entity);
    if (!astronaut_size_opt.has_value()) {
        return;
    }
    Size& abductor_size = abductor_size_opt->get();
    Position& abductor_pos = abductor_pos_opt->get();
    Size& astronaut_size = astronaut_size_opt->get();
    float astronaut_x = abductor_pos.x + abductor_size.width / 2 - astronaut_size.width / 2;
    float astronaut_y = abductor_pos.y - abductor_size.height / 2 - astronaut_size.height / 2;
    astronaut_pos_opt->get().x = astronaut_x;
    astronaut_pos_opt->get().y = astronaut_y;
}

void AbductionSystem::update(ComponentStorage& storage, Blackboard& blackboard, EntityManager& entity_manager) {
    auto abducted_entities = storage.entities_with_component<AbductedTag>();
    auto abducting_entities = storage.entities_with_component<AbductingTag>();
    auto falling_entities = storage.entities_with_component<FallingTag>();
    auto rescued_entities = storage.entities_with_component<RescuedTag>();

    // Iterate all entities that have a AbductedTag component   
    for (Entity abducted_entity : abducted_entities) {
        auto tag_opt = storage.get_component<AbductedTag>(abducted_entity);
        if (!tag_opt.has_value()) {
            continue;
        }
        Entity abductor = tag_opt->get().abductor_id;
        if (!entity_manager.is_alive(abductor)) {
            storage.remove_component<AbductedTag>(abducted_entity);
            storage.add_component(abducted_entity, FallingTag{});
        }
        else {
            auto pos_opt = storage.get_component<Position>(abducted_entity);
            if (!pos_opt.has_value()) {
                continue;
            }
            Position& pos = pos_opt->get();
            if (pos.y > 300) {
                storage.add_component(abducted_entity, DestroyRequest{});
                storage.remove_component<AbductingTag>(abductor);
                alien_spawn_system_.spawn_alien(storage, blackboard, entity_manager, AlienType::baiter, pos.x, pos.y);
            }
            else {
                auto vel_opt = storage.get_component<Velocity>(abducted_entity);
                if (!vel_opt.has_value()) {
                    continue;
                }
                Velocity& vel = vel_opt->get();
                auto abductor_vel_opt = storage.get_component<Velocity>(abductor);
                if (!abductor_vel_opt.has_value()) {
                    continue;
                }
                Velocity& abductor_vel = abductor_vel_opt->get();
                vel.dx = abductor_vel.dx;
                vel.dy = abductor_vel.dy;
                move_hanging_entity_to_holder(storage, abducted_entity, abductor);
            }
        }
    }    

    // Iterate all entities that have a FallingTag component
    for (Entity falling_entity : falling_entities) {
        auto pos_opt = storage.get_component<Position>(falling_entity);
        if (!pos_opt.has_value()) {
            continue;
        }
        Position& pos = pos_opt->get();
        if (pos.y <= -375) {
            storage.add_component(falling_entity, DestroyRequest{});
        }
        else {
            auto vel_opt = storage.get_component<Velocity>(falling_entity);
            if (!vel_opt.has_value()) {
                continue;
            }
            Velocity& vel = vel_opt->get();
            vel.dy = -200.0;
        }
    }

    // Iterate all entities that have a RescuedTag component
    for (Entity rescued_entity : rescued_entities) {
        auto pos_opt = storage.get_component<Position>(rescued_entity);
        Entity ship = storage.entities_with_component<ShipTag>().front();
        if (!pos_opt.has_value()) {
            continue;
        }
        Position& pos = pos_opt->get();
        move_hanging_entity_to_holder(storage, rescued_entity, ship);
        if (pos.y <= -375) {
            storage.remove_component<RescuedTag>(rescued_entity);
            int points = blackboard.get_or<int>("astronaut.points", 0);

            int score = blackboard.get_or<int>("game.score", 0);
            blackboard.set("game.score", score + points);
        }
        else {
            auto vel_opt = storage.get_component<Velocity>(rescued_entity);
            if (!vel_opt.has_value()) {
                continue;
            }
            auto ship_vel_opt = storage.get_component<Velocity>(ship);
            if (!ship_vel_opt.has_value()) {
                continue;
            }
            Velocity& vel = vel_opt->get();
            Velocity& ship_vel = ship_vel_opt->get();
            vel.dx = ship_vel.dx;
            vel.dy = ship_vel.dy;
        }
    }

    for (Entity abducting_entity : abducting_entities) {
        auto tag_opt = storage.get_component<AbductingTag>(abducting_entity);
        if (!tag_opt.has_value()) {
            continue;
        }
        Entity target = tag_opt->get().target_id;
        if (!entity_manager.is_alive(target)) {
            storage.remove_component<AbductingTag>(abducting_entity);
        }
    }
}