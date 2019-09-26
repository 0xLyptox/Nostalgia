/*
 * Nostalgia - A custom Minecraft server.
 * Copyright (C) 2019  Jacob Zhitomirsky
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "util/position.hpp"


chunk_pos::chunk_pos (const block_pos& bpos)
  : x (bpos.x >> 4), z (bpos.z >> 4)
{
  // nop
}

chunk_pos::chunk_pos (const player_pos& pos)
  : x ((int)pos.x >> 4), z ((int)pos.z >> 4)
{
  // nop
}


block_pos::block_pos (const player_pos& pos)
  : x ((int)pos.x), y ((int)pos.y), z ((int)pos.z)
{
  // nop
}
