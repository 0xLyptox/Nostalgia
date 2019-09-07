//
// Created by Jacob Zhitomirsky on 05-Jul-19.
//

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
