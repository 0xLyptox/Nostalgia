//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#include "world/world.hpp"


world::world (caf::actor_config& cfg, const std::string& name, caf::actor world_gen)
  : caf::event_based_actor (cfg), name (name), world_gen (world_gen)
{
  // nop
}


caf::behavior
world::make_behavior ()
{
  return {
    [this] (caf::atom_constant<caf::atom ("reqcdata")>, int cx, int cz, caf::actor broker) {
      caf::aout (this) << "Got request for chunk: " << cx << ", " << cz << std::endl;

      // find this chunk
      auto ch = this->find_chunk (cx, cz);
      if (ch)
        // chunk already exists, send it.
        this->send (broker, caf::atom ("packet"), ch->make_chunk_data_packet ().move_data ());
      else
        {
          // chunk does not exist, generate it.
          caf::aout (this) << "Requesting chunk from world generator." << std::endl;
          this->request (this->world_gen, caf::infinite, caf::atom ("generate"), chunk_pos (cx, cz)).then (
              [this, cx, cz, broker] (chunk ch) {
                caf::aout (this) << "Received chunk from world generator." << std::endl;

                auto key = std::make_pair (cx, cz);
                auto& new_ch = this->chunks[key] = std::make_unique<chunk> (std::move (ch));

                // send chunk to player.
                this->send (broker, caf::atom ("packet"), new_ch->make_chunk_data_packet ().move_data ());
              });
        }
    }
  };
}



chunk*
world::find_chunk (int cx, int cz)
{
  auto key = std::make_pair (cx, cz);
  auto itr = this->chunks.find (key);
  if (itr == this->chunks.end ())
    return nullptr;
  return itr->second.get ();
}
