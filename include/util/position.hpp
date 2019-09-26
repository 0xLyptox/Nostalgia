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

#ifndef NOSTALGIA_POSITION_HPP
#define NOSTALGIA_POSITION_HPP

#include <caf/all.hpp>


struct block_pos;
struct player_pos;

struct chunk_pos
{
  int x, z;

  chunk_pos () : x (0), z (0) { }
  chunk_pos (int x, int z) : x (x), z (z) { }
  chunk_pos (const block_pos& bpos);
  chunk_pos (const player_pos& pos);
  chunk_pos (const chunk_pos& pos) = default;

  inline bool operator== (const chunk_pos& other) const
  { return this->x == other.x && this->z == other.z; }

  inline bool operator!= (const chunk_pos& other) const
  { return !this->operator== (other); }
};

struct block_pos
{
  int x, y, z;

  block_pos () : x (0), y (0), z (0) { }
  block_pos (int x, int y, int z) : x (x), y (y), z (z) { }
  block_pos (chunk_pos cpos) : x (cpos.x * 16), y (0), z (cpos.z * 16) { }
  block_pos (const player_pos& pos);
  block_pos (const block_pos& pos) = default;
};

struct player_pos
{
  double x, y, z;

  player_pos () : x (0), y (0), z (0) { }
  player_pos (double x, double y, double z) : x (x), y (y), z (z) { }
  player_pos (chunk_pos cpos) : x (cpos.x * 16.0), y (0), z (cpos.z * 16.0) { }
  player_pos (block_pos bpos) : x (bpos.x), y (bpos.y), z (bpos.z) { }
  player_pos (const player_pos& pos) = default;
};

struct player_rot
{
  float yaw, pitch;

  player_rot () : yaw (0.0f), pitch (0.0f) { }
  player_rot (float yaw, float pitch) : yaw (yaw), pitch (pitch) { }
  player_rot (const player_rot& rot) = default;
};




template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, chunk_pos pos)
{
  return f (caf::meta::type_name ("chunk_pos"), pos.x, pos.z);
}

template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, block_pos pos)
{
  return f (caf::meta::type_name ("block_pos"), pos.x, pos.y, pos.z);
}

template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, player_pos pos)
{
  return f (caf::meta::type_name ("player_pos"), pos.x, pos.y, pos.z);
}

template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, player_rot rot)
{
  return f (caf::meta::type_name ("player_rot"), rot.yaw, rot.pitch);
}


#endif //NOSTALGIA_POSITION_HPP
