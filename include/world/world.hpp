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

#ifndef NOSTALGIA_WORLD_HPP
#define NOSTALGIA_WORLD_HPP

#include "system/consts.hpp"
#include "system/info.hpp"
#include "world/chunk.hpp"
#include <string>
#include <map>
#include <utility>
#include <stack>
#include <caf/all.hpp>


// forward decs:
class world_provider;

struct lighting_update
{
  block_pos pos;
};

class world : public caf::blocking_actor
{
  world_info info;
  std::map<std::pair<int, int>, std::unique_ptr<chunk>> chunks;

  caf::actor srv;
  caf::actor script_eng;
  caf::actor world_gen;

  std::stack<lighting_update> lighting_updates;

  std::unique_ptr<world_provider> provider;

 public:
  [[nodiscard]] inline typed_id get_typed_id () const { return { actor_type::world, this->info.id }; }

  world (caf::actor_config& cfg, unsigned int id, const std::string& name,
      const caf::actor& srv, const caf::actor& script_eng, const caf::actor& world_gen);

  virtual void act () override;

 private:
  //! \brief Handles actor messages.
  void handle_messages ();

  //! \brief Processes queued sky/block lighting updates
  void handle_lighting (int max_updates=max_lighting_updates);

  //! \brief Delegates changes made to the world to the world's provider.
  void save ();

  chunk* find_chunk (int cx, int cz);

  //! \brief Attempts to load a chunk at the specified coordinates.
  chunk* load_chunk (int cx, int cz);

  void set_block_id (int x, int y, int z, unsigned short id);
  unsigned short get_block_id (int x, int y, int z);

  void set_sky_light (int x, int y, int z, unsigned char val);
  unsigned char get_sky_light (int x, int y, int z);
};

#endif //NOSTALGIA_WORLD_HPP
