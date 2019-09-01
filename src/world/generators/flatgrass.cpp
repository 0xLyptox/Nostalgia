//
// Created by Jacob Zhitomirsky on 10-Aug-19.
//

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
