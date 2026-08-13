-- lander.lua
-- Lander behavior for alien entities.
-- Sets velocity each frame; MovementSystem handles position.
--
-- Behavior:
--   if no nearby targets, either move randomly or move towards the player
--   ensure target still exists and is on the ground
--   move to be directly above it
--   move down towards it
--   fire bullets at player if nearby 

local SEEK_RANGE = 600.0
local ABOVE_OFFSET = 80.0
local ALIGN_TOLERANCE = 12.0
local PLAYER_THREAT_RANGE = 220.0
local WANDER_TIMER = 5.0

local function prefix(id)
    return "entity." .. id .. "."
end

local function is_grounded_astronaut(id)
    if not engine.has_component(id, "AstronautTag") then
        return false
    end
    -- Falling or already abducted astronauts are not valid ground targets
    if engine.has_component(id, "FallingTag") then
        return false
    end
    if engine.has_component(id, "AbductedTag") then
        return false
    end
    return engine.has_component(id, "Position")
end

local function find_nearest_astronaut(x, y)
    local best_id = nil
    local best_dist = SEEK_RANGE
    local astronauts = engine.entities_with("AstronautTag")
    for i = 1, #astronauts do
        local aid = astronauts[i]
        if is_grounded_astronaut(aid) then
            local ax, ay = engine.get_position(aid)
            if ax ~= nil then
                local d = engine.distance(x, y, ax, ay)
                if d < best_dist then
                    best_dist = d
                    best_id = aid
                end
            end
        end
    end
    return best_id
end

local function find_player()
    local ships = engine.entities_with("ShipTag")
    if #ships > 0 then
        return ships[1]
    end
    return nil
end

function on_init(id)
    math.randomseed(id * 9973 + 17)

    local speed = engine.get_blackboard("alien.lander.speed")
    if speed == nil then
        speed = 50.0
    end
    engine.set_blackboard(prefix(id) .. "speed", speed)

    engine.set_blackboard(prefix(id) .. "target", -1.0)
    engine.set_blackboard(prefix(id) .. "wander_dx", (math.random(0, 1) == 0) and -1.0 or 1.0)
    engine.set_blackboard(prefix(id) .. "wander_timer", 1.0 + math.random() * 2.0)
    engine.set_blackboard(prefix(id) .. "cooldown", 0.0)
end

function on_update(id, dt)
    local speed = engine.get_blackboard(prefix(id) .. "speed")
    if speed == nil then
        speed = 50.0
    end

    local x, y = engine.get_position(id)
    if x == nil then
        return
    end

    local target = engine.get_blackboard(prefix(id) .. "target")
    local cooldown = engine.get_blackboard(prefix(id) .. "cooldown")
    if cooldown == nil then
        cooldown = 2.0
    end
    cooldown = cooldown - dt
    engine.set_blackboard(prefix(id) .. "cooldown", cooldown)
    if cooldown <= 0 then
        engine.set_blackboard(prefix(id) .. "cooldown", 0.0)
    end

    -- Player nearby: fire a bullet
    local player = find_player()
    if player ~= nil then
        local px, py = engine.get_position(player)
        if px ~= nil then
            local pd = engine.distance(x, y, px, py)
            if pd < PLAYER_THREAT_RANGE and cooldown <= 0 then
                engine.set_bullet_fire_request(id)
                -- set cooldown
                engine.set_blackboard(prefix(id) .. "cooldown", 2.0)
                return
            end
        end
    end

    -- if alien has AbductingTag just slowly move up
    if engine.has_component(id, "AbductingTag") then
        engine.set_velocity(id, 0, 100.0)
        return
    end    

    -- Validate abduction target
    if target ~= nil and target >= 0 then
        local tid = math.floor(target + 0.5)
        if not is_grounded_astronaut(tid) then
            target = -1.0
            engine.set_blackboard(prefix(id) .. "target", target)
        end
    else
        target = -1.0
    end

    if target < 0 then
        local nearest = find_nearest_astronaut(x, y)
        if nearest ~= nil then
            target = nearest
            engine.set_blackboard(prefix(id) .. "target", target)
        end
    end

    -- No target: wander, occasionally drift toward the player
    if target < 0 then
        local timer = engine.get_blackboard(prefix(id) .. "wander_timer") or WANDER_TIMER
        local dx = engine.get_blackboard(prefix(id) .. "wander_dx") or 1.0
        local dy = engine.get_blackboard(prefix(id) .. "wander_dy") or -1.0
        timer = timer - dt
        if timer <= 0 then
            timer = 1.0 + math.random() * 2.0
            if player ~= nil and math.random() < 0.4 then
                local px, py = engine.get_position(player)
                if px ~= nil then
                    dx = (px >= x) and 1.0 or -1.0
                    dy = (py >= y) and 1.0 or -1.0
                else
                    dx = (math.random(0, 1) == 0) and -1.0 or 1.0
                    dy = (math.random(0, 1) == 0) and -1.0 or 1.0
                end
            else
                dx = (math.random(0, 1) == 0) and -1.0 or 1.0
                dy = (math.random(0, 1) == 0) and -1.0 or 1.0
            end
            engine.set_blackboard(prefix(id) .. "wander_dx", dx)
            engine.set_blackboard(prefix(id) .. "wander_dy", dy)
        end
        engine.set_blackboard(prefix(id) .. "wander_timer", timer)
        engine.set_velocity(id, dx * speed, dy * speed)
        return
    end

    -- Move above the astronaut, then descend onto them
    local tid = math.floor(target + 0.5)
    local tx, ty = engine.get_position(tid)
    if tx == nil then
        engine.set_blackboard(prefix(id) .. "target", -1.0)
        engine.set_velocity(id, 0, 0)
        return
    end

    local hover_y = ty + ABOVE_OFFSET
    local dx = tx - x
    local dy = hover_y - y

    if math.abs(dx) > ALIGN_TOLERANCE then
        -- Get horizontally aligned above the target
        local sx = (dx > 0) and speed or -speed
        local sy = 0.0
        if math.abs(dy) > ALIGN_TOLERANCE then
            sy = (dy > 0) and (speed * 0.5) or (-speed * 0.5)
        end
        engine.set_velocity(id, sx, sy)
    else
        -- Aligned: descend toward the astronaut
        local down = ty - y
        if math.abs(down) > 4.0 then
            local sy = (down > 0) and speed or -speed
            engine.set_velocity(id, 0, sy)
        else
            engine.set_velocity(id, 0, 0)
        end
    end
end
