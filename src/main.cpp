#include <iostream>
#include <vector>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "player/client.hpp"
#include "system/server.hpp"
#include "scripting/scripting.hpp"
#include "network/packet_writer.hpp"


class nostalgia_config : public caf::actor_system_config
{
 public:
};



struct client_broker_state
{
  caf::actor srv;
  caf::actor cl;
  unsigned int client_id;

  unsigned int packet_size = 0;
  unsigned int num_size_bytes_received = 0;
  bool waiting_for_packet_data = false;
};

caf::behavior
client_broker_impl (caf::io::broker *self, caf::io::connection_handle hdl,
                    client_broker_state* state)
{
  self->configure_read (hdl, caf::io::receive_policy::exactly (1));

  // announce self to client and link together
  self->send (state->cl, broker_atom::value, caf::actor_cast<caf::actor> (self));
  self->link_to (state->cl);

  self->set_exit_handler ([=] (caf::exit_msg& msg) {
    caf::aout (self) << "Client actor stopped." << std::endl;
    self->send (self, caf::io::connection_closed_msg { hdl });
  });

  return {
    [=] (const caf::io::connection_closed_msg& msg) {
      caf::aout (self) << "Connection closed." << std::endl;
      self->close (hdl);

      // remove client from server
      self->send (state->srv, del_client_atom::value, state->client_id);

      self->quit (caf::exit_reason::user_shutdown);
    },

    [=] (const caf::io::new_data_msg& msg) {
      if (state->waiting_for_packet_data)
        {
          // relay packet contents to associated client actor.
          self->send (state->cl, packet_in_atom::value, msg.buf);

          // reset state for next packet
          self->configure_read (hdl, caf::io::receive_policy::exactly (1));
          state->waiting_for_packet_data = false;
          state->num_size_bytes_received = 0;
          state->packet_size = 0;
        }
      else
        {
          unsigned int byte = msg.buf[0];
          state->packet_size |= (byte & 0x7F) << (7 * state->num_size_bytes_received);
          ++ state->num_size_bytes_received;
          if (!(byte & 0x80))
            {
              // this was the last byte belonging to packet length field.
              state->waiting_for_packet_data = true;
              self->configure_read (hdl, caf::io::receive_policy::exactly (state->packet_size));
            }
        }
    },

    //
    // Handles packet send requests from associated client actor.
    //
    [=] (packet_out_atom, const std::vector<char>& buf) {
      // write the size of the packet
      packet_writer writer;
      writer.write_varlong (buf.size ());
      self->write (hdl, writer.position (), writer.data ());

      self->write (hdl, buf.size (), buf.data ());
      self->flush (hdl);
    }
  };
}



struct server_broker_state
{
  std::vector<std::unique_ptr<client_broker_state>> states;
  unsigned int next_client_id = 1;
};

caf::behavior
server_broker_impl (caf::io::broker *self, server_broker_state *state,
                    const caf::actor& srv, const caf::actor& script_eng)
{
  return {
    [=] (const caf::io::new_connection_msg& msg) {
      caf::aout (self) << "Accepted new connection!" << std::endl;
      auto client_id = state->next_client_id++;
      auto cl = self->system ().spawn<client_actor> (srv, script_eng, client_id);

      state->states.emplace_back (new client_broker_state ());
      auto client_broker_state = state->states.back ().get ();
      client_broker_state->srv = srv;
      client_broker_state->cl = cl;
      client_broker_state->client_id = client_id;

      auto client_broker = self->fork (client_broker_impl, msg.handle,
          state->states.back ().get ());

      self->send (srv, add_client_atom::value, cl, client_id);
    }
  };
}


void
caf_main (caf::actor_system& system, const nostalgia_config& cfg)
{
  auto script_eng = system.spawn<scripting_actor> ();
  auto srv = system.spawn<server_actor> (script_eng);

  // initialize server
  {
    caf::scoped_actor self (system);
    std::cout << "Initializing server..." << std::endl;
    self->request (srv, caf::infinite, init_atom::value).receive (
      [&] (bool res) {
        std::cout << "Server initialized." << std::endl;
      },
      [&] (const caf::error& err) {
        // TODO
      });
  }

  auto broker_state = new server_broker_state ();
  auto server_broker_actor = system.middleman ().spawn_server (server_broker_impl, 25565, broker_state, srv, script_eng);
  if (!server_broker_actor)
    {
      std::cout << "Failed to spawn server actor: " << system.render (server_broker_actor.error ()) << std::endl;
      return;
    }
}

CAF_MAIN(caf::io::middleman)
