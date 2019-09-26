--
--
--

cmd = cmd or {}

function cmd.do_weird(player, msg)
    -- split message into words
    local params = {}
    for each in msg:gmatch('([^%s]+)') do
        table.insert(params, each)
    end

    if #params < 3 then
        player:message(RED, 'Error: ', GRAY, 'Usage: /weird <chdir> <count>')
        return
    end

    math.randomseed(os.time())

    local chdir = tonumber(params[2])
    local count = tonumber(params[3])

    player:add_event_listener('changeblock', function (ev)
        local start_pos = ev.block_pos
        local world = player:get_world()

        local block = 1383 + math.random(0, 15)
        local x = math.floor(start_pos.x)
        local y = math.floor(start_pos.y)
        local z = math.floor(start_pos.z)

        local dx, dy, dz = 0, 1, 0

        for i = 1, count do
            world:set_block(x, y, z, block)

            x = x + dx
            y = y + dy
            z = z + dz

            if world:get_block(x, y, z) ~= 0 then
                local attempts = 0
                while attempts < 10 and world:get_block(x, y, z) ~= 0 do
                    block = 1383 + math.random(0, 15)
                    local choice = math.random(1, 6)
                    if choice == 0 then
                        dx, dy, dz = -1, 0, 0
                    elseif choice == 1 then
                        dx, dy, dz = 1, 0, 0
                    elseif choice == 2 then
                        dx, dy, dz = 0, -1, 0
                    elseif choice == 3 then
                        dx, dy, dz = 0, 1, 0
                    elseif choice == 4 then
                        dx, dy, dz = 0, 0, -1
                    elseif choice == 5 then
                        dx, dy, dz = 0, 0, 1
                    end

                    x = x + dx
                    y = y + dy
                    z = z + dz
                    attempts = attempts + 1
                end

                if attempts == 10 then
                    break
                end
            elseif math.random(1, chdir) == 1 then
                block = 1383 + math.random(0, 15)
                local choice = math.random(1, 6)
                if choice == 0 then
                    dx, dy, dz = -1, 0, 0
                elseif choice == 1 then
                    dx, dy, dz = 1, 0, 0
                elseif choice == 2 then
                    dx, dy, dz = 0, -1, 0
                elseif choice == 3 then
                    dx, dy, dz = 0, 1, 0
                elseif choice == 4 then
                    dx, dy, dz = 0, 0, -1
                elseif choice == 5 then
                    dx, dy, dz = 0, 0, 1
                end
            end

        end
    end)

    player:message('Place a block')

    --local x, y, z = 0, 0, 0
    --for i in 1..count do
    --
    --end
end
