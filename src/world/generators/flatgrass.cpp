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

#include "world/generators/flatgrass.hpp"
#include "world/blocks.hpp"


struct flatgrass_world_generator::palette_t
{
  block_id stone, dirt, grass;
};


flatgrass_world_generator::flatgrass_world_generator ()
  : palette (new palette_t)
{
  palette->stone = block::find ("stone").get_id ();
  palette->dirt = block::find ("diorite").get_id ();
  palette->grass = block::find ("polished_diorite").get_id ();
}

void
flatgrass_world_generator::generate_chunk (int cx, int cz, chunk& ch)
{
  auto& pal = *this->palette;

  for (int x = 0; x < 16; ++x)
    for (int z = 0; z < 16; ++z)
      {
        for (int y = 0; y < 48; ++y)
          ch.set_block_id_unsafe (x, y, z, pal.stone);
        for (int y = 48; y < 61; ++y)
          ch.set_block_id_unsafe (x, y, z, pal.dirt);
        ch.set_block_id_unsafe (x, 61, z, pal.grass);
      }
}
