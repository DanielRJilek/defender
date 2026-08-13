-- swarmer.lua
-- Swarmer behavior for alien entities.
-- Sets velocity each frame; MovementSystem handles position.
--
-- Behavior:
--   always hunts the player
--   moves towards the player
--   fires bullets at the player

local PLAYER_THREAT_RANGE = 300.0
local WANDER_TIMER = 1.0

local function prefix(id)
    return "entity." .. id .. "."
end

local function find_player()
    local ships = engine.entities_with("ShipTag")
    if #ships > 0 then
        return ships[1]
    end
    return nil
end

function on_init(id)
    local speed = engine.get_blackboard(prefix(id) .. "speed")
    engine.set_blackboard(prefix(id) .. "speed", speed)
    engine.set_blackboard(prefix(id) .. "cooldown", 0.0)
    engine.set_blackboard(prefix(id) .. "wander_timer", WANDER_TIMER + math.random() * 2.0)
    engine.set_velocity(id, speed, 0.0)
end

function on_update(id, dt)
    local x, y = engine.get_position(id)
    if x == nil then
        return
    end
    local player = find_player()
    local cooldown = engine.get_blackboard(prefix(id) .. "cooldown")
    if cooldown == nil then
        cooldown = 2.0
    end
    cooldown = cooldown - dt
    engine.set_blackboard(prefix(id) .. "cooldown", cooldown)
    if cooldown <= 0 then
        engine.set_blackboard(prefix(id) .. "cooldown", 0.0)
    end

    -- if player in range, move towards them and fire bullet
    if player then
        local px, py = engine.get_position(player)
        if px ~= nil then
            local pd = engine.distance(x, y, px, py)
            if pd < PLAYER_THREAT_RANGE then
                local dx = (px >= x) and 1.0 or -1.0
                local dy = (py >= y) and 1.0 or -1.0
                local speed = engine.get_blackboard(prefix(id) .. "speed")
                engine.set_velocity(id, dx * speed, dy * speed)
                if cooldown <= 0 then
                    engine.set_bullet_fire_request(id)
                    engine.set_blackboard(prefix(id) .. "cooldown", 2.0)
                    return
                end
            end
            -- if wander timer is up, adjust trajectory toward player
            local wander_timer = engine.get_blackboard(prefix(id) .. "wander_timer")
            if wander_timer <= 0 then
                local dx = (px >= x) and 1.0 or -1.0
                local dy = (py >= y) and 1.0 or -1.0
                -- small random chance that vertical velocity is adjusted
                if math.random() < 0.01 then
                    dy = (py >= y) and -1.0 or 1.0
                end
                local speed = engine.get_blackboard(prefix(id) .. "speed")
                engine.set_velocity(id, dx * speed, dy * speed)
                wander_timer = WANDER_TIMER + math.random() * 2.0
                engine.set_blackboard(prefix(id) .. "wander_timer", wander_timer)
                return
            end
            wander_timer = wander_timer - dt
            engine.set_blackboard(prefix(id) .. "wander_timer", wander_timer)
        end
    end
end