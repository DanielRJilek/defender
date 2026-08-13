#include "destruction.hpp"
#include "entity_manager.hpp"
#include "component_storage.hpp"

void destroy_marked_entities(EntityManager& em, ComponentStorage& storage) {
    // Get a snapshot (vector copy) of all entities marked for destruction.
    // Iterating a copy is safe even though we modify the maps during the loop.
    auto marked = storage.entities_with_component<DestroyRequest>();

    for (Entity entity : marked) {
        // Remove ALL 19 component types — each call is a no-op if the
        // entity doesn't have that particular component.
        storage.remove_component<Position>(entity);
        storage.remove_component<Size>(entity);
        storage.remove_component<Color>(entity);
        storage.remove_component<Input>(entity);
        storage.remove_component<Velocity>(entity);
        storage.remove_component<Images>(entity);
        storage.remove_component<Text>(entity);
        storage.remove_component<ScreenPosition>(entity);
        storage.remove_component<Collider>(entity);
        storage.remove_component<Lifetime>(entity);
        storage.remove_component<WrapAround>(entity);
        storage.remove_component<CollidedWith>(entity);
        storage.remove_component<BulletTag>(entity);
        storage.remove_component<ShipTag>(entity);
        storage.remove_component<DestroyRequest>(entity);
        storage.remove_component<AlienTag>(entity);
        storage.remove_component<AstronautTag>(entity);
        storage.remove_component<FallingTag>(entity);
        storage.remove_component<AbductingTag>(entity);
        storage.remove_component<AbductedTag>(entity);
        storage.remove_component<AlienType>(entity);
        storage.remove_component<Script>(entity);
        storage.remove_component<Direction>(entity);
        storage.remove_component<BulletFireRequest>(entity);
        storage.remove_component<LaserTag>(entity);
        storage.remove_component<RescuedTag>(entity);
        storage.remove_component<SpriteSheet>(entity);
        storage.remove_component<Animation>(entity);
        storage.remove_component<AnimationState>(entity);
        
        em.destroy_entity(entity);
    }
}
