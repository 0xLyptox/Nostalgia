//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#ifndef NOSTALGIA_SERVER_HPP
#define NOSTALGIA_SERVER_HPP

#include "info.hpp"
#include <caf/all.hpp>
#include <vector>
#include <map>
#include <random>


class server : public caf::event_based_actor
{
  std::mt19937 rnd;
  std::map<unsigned int, client_info> connected_clients;
  std::map<std::string, world_info> worlds;

 public:
  explicit server (caf::actor_config& cfg);

  caf::behavior make_behavior () override;

 private:
  void setup ();
};

#endif //NOSTALGIA_SERVER_HPP
