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

#include "world/generator_actor.hpp"
#include "system/atoms.hpp"
#include "util/position.hpp"
#include "world/chunk.hpp"

#include "world/generators/flatgrass.hpp"


world_generator_actor::world_generator_actor (caf::actor_config& cfg)
  : caf::blocking_actor (cfg)
{
  this->generators.emplace_back (new flatgrass_world_generator ());
}


void
world_generator_actor::act ()
{
  bool running = true;
  this->receive_while ([&] { return running; }) (
      [this] (generate_atom, chunk_pos pos, std::string gen_name) {
        chunk ch (pos.x, pos.z);

        for (auto& gen : this->generators)
          if (gen_name == gen->get_name ())
            {
              gen->generate_chunk (pos.x, pos.z, ch);
            }

        ch.compute_initial_lighting ();
        return ch;
      },

      [&] (stop_atom) {
        running = false;
      });
}