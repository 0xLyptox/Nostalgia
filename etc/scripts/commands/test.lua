--[[
COMMAND: test
--]]



function docommand (player)
    pos = player:get_position()
    for k, v in pairs(pos) do
        player:message(k .. ': ' .. v)
    end
    player:message('-------------')
end
