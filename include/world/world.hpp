//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#ifndef NOSTALGIA_WORLD_HPP
#define NOSTALGIA_WORLD_HPP

#include "world/chunk.hpp"
#include <string>
#include <map>
#include <utility>
#include <caf/all.hpp>


class world : public caf::event_based_actor
{
  std::string name;
  std::map<std::pair<int, int>, std::unique_ptr<chunk>> chunks;

  caf::actor world_gen;

 public:
  inline const std::string get_name () const { return this->name; }

  world (caf::actor_config& cfg, const std::string& name, caf::actor world_gen);

  caf::behavior make_behavior () override;

  chunk* find_chunk (int cx, int cz);
};

#endif //NOSTALGIA_WORLD_HPP
