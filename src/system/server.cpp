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

#include "system/server.hpp"
#include "system/atoms.hpp"
#include "world/world.hpp"
#include "system/consts.hpp"
#include "world/generator_actor.hpp"
#include "network/packets.hpp"
#include "world/blocks.hpp"
#include "system/registries.hpp"
#include "scripting/scripting.hpp"
#include <filesystem>


server_actor::server_actor (caf::actor_config& cfg, const caf::actor& script_eng)
    : caf::event_based_actor (cfg), script_eng (script_eng)
{
  // nop
}


void
server_actor::setup ()
{
  // create directories
  std::filesystem::create_directory ("worlds");

  // load registries
  registries::initialize ();

  // load blocks
  block::initialize ();

  // run init script
  this->send (this->script_eng, run_script_basic_atom::value, "scripts/init.lua");

  // load commands
  this->send (this->script_eng, load_commands_atom::value, "scripts/commands");

  // spawn world generator actor
  this->world_gen = this->system ().spawn<world_generator_actor> ();

  // spawn main world
  auto main_world = this->system ().spawn<world> (this->next_world_id, main_world_name, this, this->script_eng, this->world_gen);
  world_info info = { this->next_world_id, main_world, main_world_name };
  this->worlds[main_world_name] = info;
  ++ this->next_world_id;
}

void
server_actor::stop ()
{
  if (this->stopping)
    return;
  this->stopping = true;

  // disconnect all clients
  caf::aout (this) << "Disconnecting all connected clients" << std::endl;
  for (auto& p : this->connected_clients)
    this->send (p.second.actor, stop_atom::value, this);

  // save and stop worlds
  caf::aout (this) << "Stopping and saving worlds" << std::endl;
  for (auto& p : this->worlds)
    this->send (p.second.actor, stop_atom::value, this);
}

void
server_actor::handle_stop_response (typed_id id)
{
  if (id.type == actor_type::world)
    {
      auto itr = std::find_if (this->worlds.begin (), this->worlds.end (),
          [id] (auto& p) { return p.second.id == id.id; });
      if (itr != this->worlds.end ())
        this->worlds.erase (itr);
    }
  else if (id.type == actor_type::client)
    {
      caf::aout (this) << "Got response from client" << std::endl;
      auto itr = this->connected_clients.find (id.id);
      if (itr != this->connected_clients.end ())
        this->connected_clients.erase (itr);
    }

  if (this->worlds.empty () && this->connected_clients.empty ())
    {
      // server can terminate now
      if (this->stop_requester)
        this->send (this->stop_requester, stop_response_atom::value, typed_id { actor_type::server, 0 });
    }
}


caf::behavior
server_actor::make_behavior ()
{
  return {
      [=] (init_atom) {
        this->setup ();
        return true;
      },

      [=] (stop_atom, const caf::actor& requester) {
        this->stop_requester = requester;
        this->stop ();
      },

      [=] (stop_response_atom, typed_id id) {
        this->handle_stop_response (id);
      },

      [=] (add_client_atom, const caf::actor& client, unsigned int client_id) {
        if (this->stopping) return;
        this->connected_clients[client_id] = client_info ();
        client_info& info = this->connected_clients[client_id];
        info.actor = client;
        info.id = client_id;
        info.uuid = uuid_t::random (this->rnd);
      },

      [=] (del_client_atom, unsigned int client_id) {
        this->connected_clients.erase (client_id);
      },

      [=] (get_client_atom, unsigned int client_id) {
        auto itr = this->connected_clients.find (client_id);
        if (itr == this->connected_clients.end ())
          return client_info {};
        return itr->second;
      },

      [=] (set_client_atom, unsigned int client_id,
              const client_info& info) {
        if (this->stopping) return;
        auto itr = this->connected_clients.find (client_id);
        if (itr != this->connected_clients.end ())
          {
            itr->second = info;
          }
      },

      [=] (get_world_atom, const std::string& world_name) {
        auto itr = this->worlds.find (world_name);
        if (itr == this->worlds.end ())
          return world_info {};
        return itr->second;
      },


      [=] (broadcast_packet_atom, const std::vector<char>& buf) {
        for (auto& p : this->connected_clients)
          {
            auto& cl = p.second.actor;
            this->send (cl, packet_out_atom::value, buf);
          }
      },

      [=] (global_message_atom, const std::string& msg) {
        auto packet = packets::play::make_chat_message_simple (msg, 0);
        std::vector<char> packet_buf = packet.move_data ();
        for (auto& p : this->connected_clients)
          {
            auto& cl = p.second.actor;
            this->send (cl, packet_out_atom::value, packet_buf);
          }
      }
  };
}

