#include "component_storage.hpp"

// Template specializations for get_storage() - maps component types to storage maps

template<>
std::unordered_map<Entity, Position>& ComponentStorage::get_storage<Position>() {
    return positions_;
}

template<>
std::unordered_map<Entity, Size>& ComponentStorage::get_storage<Size>() {
    return sizes_;
}

template<>
std::unordered_map<Entity, Color>& ComponentStorage::get_storage<Color>() {
    return colors_;
}

template<>
const std::unordered_map<Entity, Position>& ComponentStorage::get_storage<Position>() const {
    return positions_;
}

template<>
const std::unordered_map<Entity, Size>& ComponentStorage::get_storage<Size>() const {
    return sizes_;
}

template<>
const std::unordered_map<Entity, Color>& ComponentStorage::get_storage<Color>() const {
    return colors_;
}

template<>
std::unordered_map<Entity, Input>& ComponentStorage::get_storage<Input>() {
    return inputs_;
}

template<>
const std::unordered_map<Entity, Input>& ComponentStorage::get_storage<Input>() const {
    return inputs_;
}

template<>
std::unordered_map<Entity, Velocity>& ComponentStorage::get_storage<Velocity>() {
    return velocities_;
}

template<>
const std::unordered_map<Entity, Velocity>& ComponentStorage::get_storage<Velocity>() const {
    return velocities_;
}

template<>
std::unordered_map<Entity, Images>& ComponentStorage::get_storage<Images>() {
    return images_;
}

template<>
const std::unordered_map<Entity, Images>& ComponentStorage::get_storage<Images>() const {
    return images_;
}

template<>
std::unordered_map<Entity, Text>& ComponentStorage::get_storage<Text>() {
    return texts_;
}

template<>
const std::unordered_map<Entity, Text>& ComponentStorage::get_storage<Text>() const {
    return texts_;
}

template<>
std::unordered_map<Entity, ScreenPosition>& ComponentStorage::get_storage<ScreenPosition>() {
    return screen_positions_;
}

template<>
const std::unordered_map<Entity, ScreenPosition>& ComponentStorage::get_storage<ScreenPosition>() const {
    return screen_positions_;
}

template<>
std::unordered_map<Entity, Collider>& ComponentStorage::get_storage<Collider>() {
    return colliders_;
}

template<>
const std::unordered_map<Entity, Collider>& ComponentStorage::get_storage<Collider>() const {
    return colliders_;
}

template<>
std::unordered_map<Entity, Lifetime>& ComponentStorage::get_storage<Lifetime>() {
    return lifetimes_;
}

template<>
const std::unordered_map<Entity, Lifetime>& ComponentStorage::get_storage<Lifetime>() const {
    return lifetimes_;
}

template<>
std::unordered_map<Entity, WrapAround>& ComponentStorage::get_storage<WrapAround>() {
    return wrap_arounds_;
}

template<>
const std::unordered_map<Entity, WrapAround>& ComponentStorage::get_storage<WrapAround>() const {
    return wrap_arounds_;
}

template<>
std::unordered_map<Entity, DestroyRequest>& ComponentStorage::get_storage<DestroyRequest>() {
    return destroy_requests_;
}

template<>
const std::unordered_map<Entity, DestroyRequest>& ComponentStorage::get_storage<DestroyRequest>() const {
    return destroy_requests_;
}

template<>
std::unordered_map<Entity, CollidedWith>& ComponentStorage::get_storage<CollidedWith>() {
    return collided_withs_;
}

template<>
const std::unordered_map<Entity, CollidedWith>& ComponentStorage::get_storage<CollidedWith>() const {
    return collided_withs_;
}

template<>
std::unordered_map<Entity, BulletTag>& ComponentStorage::get_storage<BulletTag>() {
    return bullet_tags_;
}

template<>
const std::unordered_map<Entity, BulletTag>& ComponentStorage::get_storage<BulletTag>() const {
    return bullet_tags_;
}

template<>
std::unordered_map<Entity, ShipTag>& ComponentStorage::get_storage<ShipTag>() {
    return ship_tags_;
}

template<>
const std::unordered_map<Entity, ShipTag>& ComponentStorage::get_storage<ShipTag>() const {
    return ship_tags_;
}

template<>
std::unordered_map<Entity, AlienTag>& ComponentStorage::get_storage<AlienTag>() {
    return alien_tags_;
}

template<>
const std::unordered_map<Entity, AlienTag>& ComponentStorage::get_storage<AlienTag>() const {
    return alien_tags_;
}

template<>
std::unordered_map<Entity, AstronautTag>& ComponentStorage::get_storage<AstronautTag>() {
    return astronaut_tags_;
}

template<>
const std::unordered_map<Entity, AstronautTag>& ComponentStorage::get_storage<AstronautTag>() const {
    return astronaut_tags_;
}

template<>
std::unordered_map<Entity, FallingTag>& ComponentStorage::get_storage<FallingTag>() {
    return falling_tags_;
}

template<>
const std::unordered_map<Entity, FallingTag>& ComponentStorage::get_storage<FallingTag>() const {
    return falling_tags_;
}

template<>
std::unordered_map<Entity, RescuedTag>& ComponentStorage::get_storage<RescuedTag>() {
    return rescued_tags_;
}

template<>
const std::unordered_map<Entity, RescuedTag>& ComponentStorage::get_storage<RescuedTag>() const {
    return rescued_tags_;
}

template<>
std::unordered_map<Entity, AbductingTag>& ComponentStorage::get_storage<AbductingTag>() {
    return abducting_tags_;
}

template<>
const std::unordered_map<Entity, AbductingTag>& ComponentStorage::get_storage<AbductingTag>() const {
    return abducting_tags_;
}

template<>
std::unordered_map<Entity, AbductedTag>& ComponentStorage::get_storage<AbductedTag>() {
    return abducted_tags_;
}

template<>
const std::unordered_map<Entity, AbductedTag>& ComponentStorage::get_storage<AbductedTag>() const {
    return abducted_tags_;
}

template<>
std::unordered_map<Entity, AlienType>& ComponentStorage::get_storage<AlienType>() {
    return alien_types_;
}

template<>
const std::unordered_map<Entity, AlienType>& ComponentStorage::get_storage<AlienType>() const {
    return alien_types_;
}

template<>
std::unordered_map<Entity, Script>& ComponentStorage::get_storage<Script>() {
    return scripts_;
}

template<>
const std::unordered_map<Entity, Script>& ComponentStorage::get_storage<Script>() const {
    return scripts_;
}

template<>
std::unordered_map<Entity, Direction>& ComponentStorage::get_storage<Direction>() {
    return directions_;
}

template<>
const std::unordered_map<Entity, Direction>& ComponentStorage::get_storage<Direction>() const {
    return directions_;
}

template<>
std::unordered_map<Entity, BulletFireRequest>& ComponentStorage::get_storage<BulletFireRequest>() {
    return bullet_fire_requests_;
}

template<>
const std::unordered_map<Entity, BulletFireRequest>& ComponentStorage::get_storage<BulletFireRequest>() const {
    return bullet_fire_requests_;
}

template<>
std::unordered_map<Entity, LaserTag>& ComponentStorage::get_storage<LaserTag>() {
    return laser_tags_;
}

template<>
const std::unordered_map<Entity, LaserTag>& ComponentStorage::get_storage<LaserTag>() const {
    return laser_tags_;
}

template<>
std::unordered_map<Entity, SpriteSheet>& ComponentStorage::get_storage<SpriteSheet>() {
    return sprite_sheets_;
}

template<>
const std::unordered_map<Entity, SpriteSheet>& ComponentStorage::get_storage<SpriteSheet>() const {
    return sprite_sheets_;
}

template<>
std::unordered_map<Entity, Animation>& ComponentStorage::get_storage<Animation>() {
    return animations_;
}

template<>
const std::unordered_map<Entity, Animation>& ComponentStorage::get_storage<Animation>() const {
    return animations_;
}

template<>
std::unordered_map<Entity, AnimationState>& ComponentStorage::get_storage<AnimationState>() {
    return animation_states_;
}

template<>
const std::unordered_map<Entity, AnimationState>& ComponentStorage::get_storage<AnimationState>() const {
    return animation_states_;
}

template<typename T>
void ComponentStorage::add_component(Entity entity, const T& component) {
    // Get the storage map for this component type
    auto& storage = get_storage<T>();
    
    // Insert or update the component
    // If the entity already has this component type, this replaces it
    storage[entity] = component;
}

template<typename T>
std::optional<std::reference_wrapper<T>> ComponentStorage::get_component(Entity entity) {
    // Get the storage map for this component type
    auto& storage = get_storage<T>();
    
    // Look up the entity in the storage map
    auto it = storage.find(entity);
    
    if (it != storage.end()) {
        // Component found - return a reference wrapper to it
        return std::ref(it->second);
    } else {
        // Component not found - return empty optional
        return std::nullopt;
    }
}

template<typename T>
std::optional<std::reference_wrapper<const T>> ComponentStorage::get_component(Entity entity) const {
    // Get the storage map for this component type
    const auto& storage = get_storage<T>();
    
    // Look up the entity in the storage map
    auto it = storage.find(entity);
    
    if (it != storage.end()) {
        // Component found - return a const reference wrapper to it
        return std::cref(it->second);
    } else {
        // Component not found - return empty optional
        return std::nullopt;
    }
}

template<typename T>
void ComponentStorage::remove_component(Entity entity) {
    // Get the storage map for this component type
    auto& storage = get_storage<T>();
    
    // Remove the entity from the storage map
    // If the entity doesn't have this component, erase() is a no-op
    storage.erase(entity);
}

template<typename T>
bool ComponentStorage::has_component(Entity entity) const {
    // Get the storage map for this component type
    const auto& storage = get_storage<T>();
    
    // Check if the entity exists in the storage map
    return storage.find(entity) != storage.end();
}

template<typename T>
std::vector<Entity> ComponentStorage::entities_with_component() const {
    // Get the storage map for this component type
    const auto& storage = get_storage<T>();
    
    // Collect all entity IDs from the storage map
    std::vector<Entity> entities;
    entities.reserve(storage.size());  // Pre-allocate for efficiency
    
    for (const auto& [entity, component] : storage) {
        entities.push_back(entity);
    }
    
    return entities;
}

// Explicit template instantiations for the three component types
// This ensures the template methods are compiled for Position, Size, and Color

template void ComponentStorage::add_component<Position>(Entity, const Position&);
template void ComponentStorage::add_component<Size>(Entity, const Size&);
template void ComponentStorage::add_component<Color>(Entity, const Color&);
template void ComponentStorage::add_component<Input>(Entity, const Input&);
template void ComponentStorage::add_component<Velocity>(Entity, const Velocity&);

template void ComponentStorage::add_component<Images>(Entity, const Images&);

template void ComponentStorage::add_component<Text>(Entity, const Text&);
template void ComponentStorage::add_component<ScreenPosition>(Entity, const ScreenPosition&);

template void ComponentStorage::add_component<Collider>(Entity, const Collider&);
template void ComponentStorage::add_component<Lifetime>(Entity, const Lifetime&);
template void ComponentStorage::add_component<WrapAround>(Entity, const WrapAround&);
template void ComponentStorage::add_component<DestroyRequest>(Entity, const DestroyRequest&);
template void ComponentStorage::add_component<CollidedWith>(Entity, const CollidedWith&);
template void ComponentStorage::add_component<BulletTag>(Entity, const BulletTag&);
template void ComponentStorage::add_component<ShipTag>(Entity, const ShipTag&);

template void ComponentStorage::add_component<AlienTag>(Entity, const AlienTag&);
template void ComponentStorage::add_component<AstronautTag>(Entity, const AstronautTag&);
template void ComponentStorage::add_component<FallingTag>(Entity, const FallingTag&);
template void ComponentStorage::add_component<RescuedTag>(Entity, const RescuedTag&);
template void ComponentStorage::add_component<AbductingTag>(Entity, const AbductingTag&);
template void ComponentStorage::add_component<AbductedTag>(Entity, const AbductedTag&);
template void ComponentStorage::add_component<AlienType>(Entity, const AlienType&);
template void ComponentStorage::add_component<Script>(Entity, const Script&);
template void ComponentStorage::add_component<Direction>(Entity, const Direction&);
template void ComponentStorage::add_component<BulletFireRequest>(Entity, const BulletFireRequest&);
template void ComponentStorage::add_component<LaserTag>(Entity, const LaserTag&);
template void ComponentStorage::add_component<SpriteSheet>(Entity, const SpriteSheet&);
template void ComponentStorage::add_component<Animation>(Entity, const Animation&);
template void ComponentStorage::add_component<AnimationState>(Entity, const AnimationState&);

template std::optional<std::reference_wrapper<Position>> ComponentStorage::get_component<Position>(Entity);
template std::optional<std::reference_wrapper<Size>> ComponentStorage::get_component<Size>(Entity);
template std::optional<std::reference_wrapper<Color>> ComponentStorage::get_component<Color>(Entity);
template std::optional<std::reference_wrapper<Input>> ComponentStorage::get_component<Input>(Entity);
template std::optional<std::reference_wrapper<Velocity>> ComponentStorage::get_component<Velocity>(Entity);

template std::optional<std::reference_wrapper<Images>> ComponentStorage::get_component<Images>(Entity);

template std::optional<std::reference_wrapper<Text>> ComponentStorage::get_component<Text>(Entity);
template std::optional<std::reference_wrapper<ScreenPosition>> ComponentStorage::get_component<ScreenPosition>(Entity);

template std::optional<std::reference_wrapper<Collider>> ComponentStorage::get_component<Collider>(Entity);
template std::optional<std::reference_wrapper<Lifetime>> ComponentStorage::get_component<Lifetime>(Entity);
template std::optional<std::reference_wrapper<WrapAround>> ComponentStorage::get_component<WrapAround>(Entity);
template std::optional<std::reference_wrapper<DestroyRequest>> ComponentStorage::get_component<DestroyRequest>(Entity);
template std::optional<std::reference_wrapper<CollidedWith>> ComponentStorage::get_component<CollidedWith>(Entity);
template std::optional<std::reference_wrapper<BulletTag>> ComponentStorage::get_component<BulletTag>(Entity);
template std::optional<std::reference_wrapper<ShipTag>> ComponentStorage::get_component<ShipTag>(Entity);

template std::optional<std::reference_wrapper<AlienTag>> ComponentStorage::get_component<AlienTag>(Entity);
template std::optional<std::reference_wrapper<AstronautTag>> ComponentStorage::get_component<AstronautTag>(Entity);
template std::optional<std::reference_wrapper<FallingTag>> ComponentStorage::get_component<FallingTag>(Entity);
template std::optional<std::reference_wrapper<RescuedTag>> ComponentStorage::get_component<RescuedTag>(Entity);
template std::optional<std::reference_wrapper<AbductingTag>> ComponentStorage::get_component<AbductingTag>(Entity);
template std::optional<std::reference_wrapper<AbductedTag>> ComponentStorage::get_component<AbductedTag>(Entity);
template std::optional<std::reference_wrapper<AlienType>> ComponentStorage::get_component<AlienType>(Entity);
template std::optional<std::reference_wrapper<Script>> ComponentStorage::get_component<Script>(Entity);
template std::optional<std::reference_wrapper<Direction>> ComponentStorage::get_component<Direction>(Entity);
template std::optional<std::reference_wrapper<BulletFireRequest>> ComponentStorage::get_component<BulletFireRequest>(Entity);
template std::optional<std::reference_wrapper<LaserTag>> ComponentStorage::get_component<LaserTag>(Entity);
template std::optional<std::reference_wrapper<SpriteSheet>> ComponentStorage::get_component<SpriteSheet>(Entity);
template std::optional<std::reference_wrapper<Animation>> ComponentStorage::get_component<Animation>(Entity);
template std::optional<std::reference_wrapper<AnimationState>> ComponentStorage::get_component<AnimationState>(Entity);
template std::optional<std::reference_wrapper<const Position>> ComponentStorage::get_component<Position>(Entity) const;
template std::optional<std::reference_wrapper<const Size>> ComponentStorage::get_component<Size>(Entity) const;
template std::optional<std::reference_wrapper<const Color>> ComponentStorage::get_component<Color>(Entity) const;
template std::optional<std::reference_wrapper<const Input>> ComponentStorage::get_component<Input>(Entity) const;
template std::optional<std::reference_wrapper<const Velocity>> ComponentStorage::get_component<Velocity>(Entity) const;

template std::optional<std::reference_wrapper<const Images>> ComponentStorage::get_component<Images>(Entity) const;

template std::optional<std::reference_wrapper<const Text>> ComponentStorage::get_component<Text>(Entity) const;
template std::optional<std::reference_wrapper<const ScreenPosition>> ComponentStorage::get_component<ScreenPosition>(Entity) const;

template std::optional<std::reference_wrapper<const Collider>> ComponentStorage::get_component<Collider>(Entity) const;
template std::optional<std::reference_wrapper<const Lifetime>> ComponentStorage::get_component<Lifetime>(Entity) const;
template std::optional<std::reference_wrapper<const WrapAround>> ComponentStorage::get_component<WrapAround>(Entity) const;
template std::optional<std::reference_wrapper<const DestroyRequest>> ComponentStorage::get_component<DestroyRequest>(Entity) const;
template std::optional<std::reference_wrapper<const CollidedWith>> ComponentStorage::get_component<CollidedWith>(Entity) const;
template std::optional<std::reference_wrapper<const BulletTag>> ComponentStorage::get_component<BulletTag>(Entity) const;
template std::optional<std::reference_wrapper<const ShipTag>> ComponentStorage::get_component<ShipTag>(Entity) const;

template std::optional<std::reference_wrapper<const AlienTag>> ComponentStorage::get_component<AlienTag>(Entity) const;
template std::optional<std::reference_wrapper<const AstronautTag>> ComponentStorage::get_component<AstronautTag>(Entity) const;
template std::optional<std::reference_wrapper<const FallingTag>> ComponentStorage::get_component<FallingTag>(Entity) const;
template std::optional<std::reference_wrapper<const RescuedTag>> ComponentStorage::get_component<RescuedTag>(Entity) const;
template std::optional<std::reference_wrapper<const AbductingTag>> ComponentStorage::get_component<AbductingTag>(Entity) const;
template std::optional<std::reference_wrapper<const AbductedTag>> ComponentStorage::get_component<AbductedTag>(Entity) const;
template std::optional<std::reference_wrapper<const AlienType>> ComponentStorage::get_component<AlienType>(Entity) const;
template std::optional<std::reference_wrapper<const Script>> ComponentStorage::get_component<Script>(Entity) const;
template std::optional<std::reference_wrapper<const Direction>> ComponentStorage::get_component<Direction>(Entity) const;
template std::optional<std::reference_wrapper<const BulletFireRequest>> ComponentStorage::get_component<BulletFireRequest>(Entity) const;
template std::optional<std::reference_wrapper<const LaserTag>> ComponentStorage::get_component<LaserTag>(Entity) const;
template std::optional<std::reference_wrapper<const SpriteSheet>> ComponentStorage::get_component<SpriteSheet>(Entity) const;
template std::optional<std::reference_wrapper<const Animation>> ComponentStorage::get_component<Animation>(Entity) const;
template std::optional<std::reference_wrapper<const AnimationState>> ComponentStorage::get_component<AnimationState>(Entity) const;

template void ComponentStorage::remove_component<Position>(Entity);
template void ComponentStorage::remove_component<Size>(Entity);
template void ComponentStorage::remove_component<Color>(Entity);
template void ComponentStorage::remove_component<Input>(Entity);
template void ComponentStorage::remove_component<Velocity>(Entity);

template void ComponentStorage::remove_component<Images>(Entity);

template void ComponentStorage::remove_component<Text>(Entity);
template void ComponentStorage::remove_component<ScreenPosition>(Entity);

template void ComponentStorage::remove_component<Collider>(Entity);
template void ComponentStorage::remove_component<Lifetime>(Entity);
template void ComponentStorage::remove_component<WrapAround>(Entity);
template void ComponentStorage::remove_component<DestroyRequest>(Entity);
template void ComponentStorage::remove_component<CollidedWith>(Entity);
template void ComponentStorage::remove_component<BulletTag>(Entity);
template void ComponentStorage::remove_component<ShipTag>(Entity);

template void ComponentStorage::remove_component<AlienTag>(Entity);
template void ComponentStorage::remove_component<AstronautTag>(Entity);
template void ComponentStorage::remove_component<FallingTag>(Entity);
template void ComponentStorage::remove_component<RescuedTag>(Entity);
template void ComponentStorage::remove_component<AbductingTag>(Entity);
template void ComponentStorage::remove_component<AbductedTag>(Entity);
template void ComponentStorage::remove_component<AlienType>(Entity);
template void ComponentStorage::remove_component<Script>(Entity);
template void ComponentStorage::remove_component<Direction>(Entity);
template void ComponentStorage::remove_component<BulletFireRequest>(Entity);
template void ComponentStorage::remove_component<LaserTag>(Entity);
template void ComponentStorage::remove_component<SpriteSheet>(Entity);
template void ComponentStorage::remove_component<Animation>(Entity);
template void ComponentStorage::remove_component<AnimationState>(Entity);
template bool ComponentStorage::has_component<Position>(Entity) const;
template bool ComponentStorage::has_component<Size>(Entity) const;
template bool ComponentStorage::has_component<Color>(Entity) const;
template bool ComponentStorage::has_component<Input>(Entity) const;
template bool ComponentStorage::has_component<Velocity>(Entity) const;

template bool ComponentStorage::has_component<Images>(Entity) const;

template bool ComponentStorage::has_component<Text>(Entity) const;
template bool ComponentStorage::has_component<ScreenPosition>(Entity) const;

template bool ComponentStorage::has_component<Collider>(Entity) const;
template bool ComponentStorage::has_component<Lifetime>(Entity) const;
template bool ComponentStorage::has_component<WrapAround>(Entity) const;
template bool ComponentStorage::has_component<DestroyRequest>(Entity) const;
template bool ComponentStorage::has_component<CollidedWith>(Entity) const;
template bool ComponentStorage::has_component<BulletTag>(Entity) const;
template bool ComponentStorage::has_component<ShipTag>(Entity) const;

template bool ComponentStorage::has_component<AlienTag>(Entity) const;
template bool ComponentStorage::has_component<AstronautTag>(Entity) const;
template bool ComponentStorage::has_component<FallingTag>(Entity) const;
template bool ComponentStorage::has_component<RescuedTag>(Entity) const;
template bool ComponentStorage::has_component<AbductingTag>(Entity) const;
template bool ComponentStorage::has_component<AbductedTag>(Entity) const;
template bool ComponentStorage::has_component<AlienType>(Entity) const;
template bool ComponentStorage::has_component<Script>(Entity) const;
template bool ComponentStorage::has_component<Direction>(Entity) const;
template bool ComponentStorage::has_component<BulletFireRequest>(Entity) const;
template bool ComponentStorage::has_component<LaserTag>(Entity) const;
template bool ComponentStorage::has_component<SpriteSheet>(Entity) const;
template bool ComponentStorage::has_component<Animation>(Entity) const;
template bool ComponentStorage::has_component<AnimationState>(Entity) const;
template std::vector<Entity> ComponentStorage::entities_with_component<Position>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Size>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Color>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Input>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Velocity>() const;

template std::vector<Entity> ComponentStorage::entities_with_component<Images>() const;

template std::vector<Entity> ComponentStorage::entities_with_component<Text>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<ScreenPosition>() const;

template std::vector<Entity> ComponentStorage::entities_with_component<Collider>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Lifetime>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<WrapAround>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<DestroyRequest>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<CollidedWith>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<BulletTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<ShipTag>() const;

template std::vector<Entity> ComponentStorage::entities_with_component<AlienTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<AstronautTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<FallingTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<RescuedTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<AbductingTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<AbductedTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<AlienType>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Script>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Direction>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<BulletFireRequest>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<LaserTag>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<SpriteSheet>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<Animation>() const;
template std::vector<Entity> ComponentStorage::entities_with_component<AnimationState>() const;