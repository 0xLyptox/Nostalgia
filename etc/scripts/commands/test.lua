--
--
--

cmd = cmd or {}


function table_str(table)
    s = '{'
    for k, v in pairs(table) do
        s = s .. k .. ': ' .. v .. ', '
    end
    return s .. '}'
end

function sphere(world, x, y, z, radius, block)
    for xx = x - radius, x + radius do
        for yy = y - radius, y + radius do
            for zz = z - radius, z + radius do
                dist2 = (x - xx)^2 + (y - yy)^2 + (z - zz)^2
                if dist2 <= radius^2 then
                    world:set_block(xx, yy, zz, block)
                end
            end
        end
    end
end

function cmd.do_test(player, msg)
    player:add_event_listener('changeblock', function (ev)
        world = ev.target:get_world()
        height = 10

        sphere(world, ev.block_pos.x, ev.block_pos.y + height, ev.block_pos.z, 5, 144)

        for i = 0, height do
            world:set_block(ev.block_pos.x, ev.block_pos.y + i, ev.block_pos.z, 73)
        end

        player:message('Planted tree at: ' , GREEN, table_str(ev.block_pos))
    end)

    player:message('Added event')
end
