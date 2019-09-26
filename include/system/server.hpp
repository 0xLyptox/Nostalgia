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

  caf::actor stop_requester;
  bool stopping = false;

  caf::actor world_gen;
  caf::actor script_eng;

  unsigned int next_world_id = 1;

 public:
  explicit server_actor (caf::actor_config& cfg, const caf::actor& script_eng);

  caf::behavior make_behavior () override;

 private:
  void setup ();

  void stop ();
  void handle_stop_response (typed_id id);
};

#endif //NOSTALGIA_SERVER_HPP
