//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#include "client.hpp"
#include "packet_reader.hpp"
#include "packet_writer.hpp"
#include "consts.hpp"
#include "packets.hpp"


client::client (caf::actor_config& cfg, const caf::actor& srv,
                            unsigned int client_id)
  : event_based_actor (cfg), srv (srv), curr_state (connection_state::handshake)
{
  this->info.id = client_id;
}



caf::behavior
client::make_behavior ()
{
  return {
      [this] (caf::atom_constant<caf::atom ("broker")>, const caf::actor& broker) {
        this->broker = broker;
      },

      //
      // Whole packets received from the client's associated broker.
      //
      [this] (caf::atom_constant<caf::atom ("packet")>, std::vector<char> buf) {
        try
          {
            packet_reader reader (buf);
            this->handle_packet (reader);
          }
        catch (const bad_data_error&)
          {
            caf::aout (this) << "WARNING: Got bad packet data" << std::endl;
            this->quit (caf::exit_reason::user_shutdown);
          }
        catch (const disconnect& err)
          {
            caf::aout (this) << "DISCONNECT: " << err.message () << std::endl;
            if (this->curr_state == connection_state::login)
              this->send_packet (packets::login::make_disconnect (err.message ()));
            else if (this->curr_state == connection_state::play)
              this->send_packet (packets::play::make_disconnect (err.message ()));
            else
              this->quit (caf::exit_reason::user_shutdown);
          }
      }
  };
}


void
client::send_packet (packet_writer& writer)
{
  this->send (this->broker, caf::atom ("packet"), writer.move_data ());
}

void
client::send_packet (packet_writer&& writer)
{
  this->send (this->broker, caf::atom ("packet"), writer.move_data ());
}


void
client::handle_packet (packet_reader& reader)
{
  caf::aout (this) << "Received packet of size " << reader.size () << std::endl;

  switch (this->curr_state)
    {
    case connection_state::handshake:
      this->handle_handshake_state_packet (reader);
      break;

    case connection_state::play:
      this->handle_play_state_packet (reader);
        break;

    case connection_state::status:
      this->handle_status_state_packet (reader);
        break;

    case connection_state::login:
      this->handle_login_state_packet (reader);
        break;

      default:
        // TODO
        break;
    }
}


void
client::handle_handshake_state_packet (packet_reader& reader)
{
  auto id = reader.read_varlong ();
  switch (id)
    {
    case IPI_HANDSHAKE:
      this->handle_handshake_packet (reader);
      break;

    default:
      // TODO
      break;
    }
}

void
client::handle_play_state_packet (packet_reader& reader)
{
  auto id = reader.read_varlong ();
  switch (id)
    {
    default:
      caf::aout (this) << "WARNING: Unknown PLAY state packet (ID " << id << ")" << std::endl;
      break;
    }
}

void
client::handle_status_state_packet (packet_reader& reader)
{
  auto id = reader.read_varlong ();
  switch (id)
    {
    case IPI_REQUEST:
      caf::aout (this) << "Got status request packet" << std::endl;
      this->send_packet (packets::status::make_response (
          current_version_name, current_procotol_version, 12, 0, "Nostalgia"));
      break;

    case IPI_PING:
      this->send_packet (packets::status::make_pong (reader.read_long ()));
      break;

    default:
      // TODO
      break;
    }
}

void
client::handle_login_state_packet (packet_reader& reader)
{
  auto id = reader.read_varlong ();
  switch (id)
    {
    case IPI_LOGIN_START:
      this->handle_login_start_packet (reader);
      break;

    default:
      // TODO
      break;
    }
}



void
client::handle_handshake_packet (packet_reader& reader)
{
  caf::aout (this) << "Got HANDSHAKE packet" << std::endl;

  auto proto_version = reader.read_varlong ();
  if (proto_version != current_procotol_version)
    {
      caf::aout (this) << "WARNING: Client connected with wrong protocol version: " << proto_version << std::endl;
      throw disconnect ("wrong protocol version");
    }

  reader.read_string (255);  // skip server address
  reader.read_unsigned_short ();  // skip server port

  auto next_state = reader.read_varlong ();
  if (next_state == 1)
    this->curr_state = connection_state::status;
  else if (next_state == 2)
    this->curr_state = connection_state::login;
  else
    throw disconnect ("invalid next state");
}

void
client::handle_login_start_packet (packet_reader& reader)
{
  caf::aout (this) << "Got LOGINSTART packet" << std::endl;

  auto username = reader.read_string (16);
  caf::aout (this) << "Username: " << username << std::endl;

  // get info from server
  this->request (this->srv, caf::infinite, caf::atom ("getclient"), this->info.id).await (
      [=] (const client_info& info) {
        caf::aout (this) << "Got some info." << std::endl;
        // update information
        this->info = info;
        this->info.username = username;

        // update server
        this->send (this->srv, caf::atom ("setclient"), this->info.id, this->info);

        // transition into PLAY state.
        this->curr_state = connection_state::play;
        this->send_packet (packets::login::make_login_success (this->info.uuid, username));
        this->send_packet (packets::play::make_join_game (1, 1, 0, 0, false));

        this->join_world (main_world_name);
      });
}



void
client::join_world (const std::string& world_name)
{
  caf::aout (this) << "Joining world: " << world_name << std::endl;

  this->request (this->srv, caf::infinite, caf::atom ("getworld"), world_name).await (
      [=] (const world_info& info) {
        caf::aout (this) << "Got world information of \"" << info.name << "\" from server." << std::endl;
        this->curr_world = info;
      });
}
