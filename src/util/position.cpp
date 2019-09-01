//
// Created by Jacob Zhitomirsky on 05-Jul-19.
//

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
