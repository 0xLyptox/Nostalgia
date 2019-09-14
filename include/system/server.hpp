//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#ifndef NOSTALGIA_SERVER_HPP
#define NOSTALGIA_SERVER_HPP

#include "system/info.hpp"
#include <caf/all.hpp>
#include <vector>
#include <map>
#include <random>


class server_actor : public caf::event_based_actor
{
  std::mt19937 rnd;
  std::map<unsigned int, client_info> connected_clients;
  std::map<std::string, world_info> worlds;

  caf::actor world_gen;
  caf::actor script_eng;

  unsigned int next_world_id = 1;

 public:
  explicit server_actor (caf::actor_config& cfg, const caf::actor& script_eng);

  caf::behavior make_behavior () override;

 private:
  void setup ();
};

#endif //NOSTALGIA_SERVER_HPP
