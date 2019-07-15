//
// Created by Jacob Zhitomirsky on 06-Jul-19.
//

#include "world/generator.hpp"
#include "util/position.hpp"
#include "world/chunk.hpp"


world_generator::world_generator (caf::actor_config& cfg)
  : caf::blocking_actor (cfg)
{

}


void
world_generator::act ()
{
  bool running = true;
  this->receive_while ([&] { return running; }) (
      [this] (caf::atom_constant<caf::atom ("generate")>, chunk_pos pos) {
        caf::aout (this) << "Got request to generate chunk at " << pos.x << "," << pos.z << std::endl;

        chunk ch (pos.x, pos.z);

        for (int x = 0; x < 16; ++x)
          for (int z = 0; z < 16; ++z)
            {
              for (int y = 0; y < 64; ++y)
                ch.set_block_id_unsafe (x, y, z, 1);

            }

        return ch;
      },

      [&] (caf::atom_constant<caf::atom ("stop")>) {
        running = false;
      });
}