//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#include "world.hpp"


world::world (caf::actor_config& cfg, const std::string& name)
  : caf::event_based_actor (cfg), name (name)
{
  // nop
}


caf::behavior
world::make_behavior ()
{
  return {
    [this] (caf::atom_constant<caf::atom ("getcdata")>, int cx, int cy) {

    }
  };
}



chunk*
world::find_chunk (int cx, int cy)
{
  auto key = std::make_pair (cx, cy);
  auto itr = this->chunks.find (key);
  if (itr == this->chunks.end ())
    return nullptr;
  return itr->second.get ();
}
