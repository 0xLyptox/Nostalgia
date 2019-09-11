
cmd = cmd or {}

function cmd.do_test (player)
    player:message(YELLOW, 'Hello!')
    player:message(YELLOW, 'Your name is: ', GREEN, player.name)
    player:message(YELLOW, 'Your UUID is: ', AQUA, player.uuid)

    pos = player:get_position()
    player:message(YELLOW, 'Your position is: ', RED, pos.x, YELLOW, ', ', RED, pos.y, YELLOW, ', ', RED, pos.z)
end
