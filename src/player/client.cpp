//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#include "player/client.hpp"
#include "network/packet_reader.hpp"
#include "network/packet_writer.hpp"
#include "system/consts.hpp"
#include "network/packets.hpp"


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
    case IPI_CLIENT_SETTINGS:
      this->handle_client_settings_packet (reader);
      break;

    case IPI_PLAYER:
      this->handle_player_packet (reader);
      break;

    case IPI_PLAYER_POSITION:
      this->handle_player_position_packet (reader);
      break;

    case IPI_PLAYER_POSITION_AND_LOOK:
      this->handle_player_position_and_look_packet (reader);
      break;

    case IPI_PLAYER_LOOK:
      this->handle_player_look_packet (reader);
      break;

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
client::handle_client_settings_packet (packet_reader& reader)
{
  auto locale = reader.read_string (16);
  int view_distance = reader.read_byte ();
  int chat_mode = (int)reader.read_varlong ();
  bool chat_colors = reader.read_bool ();
  unsigned skin_parts = reader.read_unsigned_byte ();
  int main_hand = (int)reader.read_varlong ();

}

void client::handle_player_packet (packet_reader& reader)
{
  bool ground = reader.read_bool ();
  this->update_position (this->pos, this->rot, ground);
}

inline void
swap (unsigned char& a, unsigned char& b)
{
  unsigned char t = a;
  a = b;
  b = t;
}

void client::handle_player_position_packet (packet_reader& reader)
{
  double x = reader.read_double ();
  caf::aout (this) << "x: " << x << std::endl;

//  double x = reader.read_double ();
//  double y = reader.read_double ();
//  double z = reader.read_double ();
//  bool ground = reader.read_bool ();
//  this->update_position ({ x, y, z }, this->rot, ground);
}

void client::handle_player_position_and_look_packet (packet_reader& reader)
{
  double x = reader.read_double ();
  double y = reader.read_double ();
  double z = reader.read_double ();
  float yaw = reader.read_float ();
  float pitch = reader.read_float ();
  bool ground = reader.read_bool ();
  this->update_position ({ x, y, z }, { yaw, pitch }, ground);
}

void client::handle_player_look_packet (packet_reader& reader)
{
  float yaw = reader.read_float ();
  float pitch = reader.read_float ();
  bool ground = reader.read_bool ();
  this->update_position (this->pos, { yaw, pitch }, ground);
}




void
client::join_world (const std::string& world_name)
{
  caf::aout (this) << "Joining world: " << world_name << std::endl;

  this->request (this->srv, caf::infinite, caf::atom ("getworld"), world_name).await (
      [=] (const world_info& info) {
        caf::aout (this) << "Got world information of \"" << info.name << "\" from server." << std::endl;
        this->curr_world = info;

        // send initial chunks
        chunk_pos cpos = this->pos;
        for (int cx = cpos.x; cx <= cpos.x; ++cx)
          for (int cz = cpos.z; cz <= cpos.z; ++cz)
            {
              // request chunk data from world
              this->send (this->curr_world.actor, caf::atom ("reqcdata"), cx, cz, this->broker);
            }

        // spawn player
        this->pos = player_pos (0, 130.0, 0);
        this->send_packet (packets::play::make_spawn_position (this->pos));
        this->send_packet (packets::play::make_player_position_and_look (
            this->pos, this->rot, 0, 1));
      });
}

void
client::update_position (player_pos pos, player_rot rot, bool ground)
{
  this->pos = pos;
  this->rot = rot;
  this->ground = ground;

  caf::aout (this) << "Position: " << pos.x << ", " << pos.y << ", " << pos.z << " : " << rot.yaw << ", " << rot.pitch << std::endl;

  caf::aout (this) << " - " << *(uint64_t *)&pos.y << std::endl;
}
