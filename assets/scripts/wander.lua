-- wander.lua
-- Random wandering behavior for entities.
-- Sets velocity each frame; MovementSystem handles position + boundary clamping.

local DEFAULT_SPEED = 40.0

function on_init(id)
    math.randomseed(id)

    -- Only set speed if not already pre-configured in the blackboard
    local existing_speed = engine.get_blackboard("entity." .. id .. ".wander_speed")
    if existing_speed == nil then
        engine.set_blackboard("entity." .. id .. ".wander_speed", DEFAULT_SPEED)
    end

    -- Random initial direction (unit vector from random angle)
    engine.set_blackboard("entity." .. id .. ".wander_dx", math.random(-1, 1))
    engine.set_blackboard("entity." .. id .. ".wander_timer", 1.0 + math.random() * 2.0)
end

function on_update(id, dt)

    -- if entity has AbductedTag, slowly move up
    if engine.has_component(id, "AbductedTag") or engine.has_component(id, "FallingTag") or engine.has_component(id, "RescuedTag") then
        return
    end

    -- Normal wander logic
    local speed = engine.get_blackboard("entity." .. id .. ".wander_speed")
    local dx = engine.get_blackboard("entity." .. id .. ".wander_dx")
    local timer = engine.get_blackboard("entity." .. id .. ".wander_timer")

    -- Timer-based direction change
    timer = timer - dt
    if timer <= 0 then
        timer = 1.0 + math.random() * 2.0
        dx = math.random(-1, 1)
        engine.set_blackboard("entity." .. id .. ".wander_dx", dx)
    end
    engine.set_blackboard("entity." .. id .. ".wander_timer", timer)

    -- Set velocity -- MovementSystem applies position + boundary clamping
    engine.set_velocity(id, dx * speed, 0)
end
