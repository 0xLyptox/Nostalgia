//
// Created by Jacob Zhitomirsky on 06-Jul-19.
//

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