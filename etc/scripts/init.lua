--
--
--

COLOR_ESCAPE_SEQUENCE = '\x07'

function cc (num)
    return COLOR_ESCAPE_SEQUENCE .. num
end

BLACK = cc('0')
DARK_BLUE = cc('1')
DARK_GREEN = cc('2')
DARK_AQUA = cc('3')
DARK_RED = cc('4')
DARK_PURPLE = cc('5')
GOLD = cc('6')
GRAY = cc('7')
DARK_GRAY = cc('8')
BLUE = cc('9')
GREEN = cc('a')
AQUA = cc('b')
RED = cc('c')
LIGHT_PURPLE = cc('d')
YELLOW = cc('e')
WHITE = cc('f')
