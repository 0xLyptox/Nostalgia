//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#include "player/client.hpp"
#include "system/atoms.hpp"
#include "network/packet_reader.hpp"
#include "network/packet_writer.hpp"
#include "system/consts.hpp"
#include "network/packets.hpp"
#include "system/registries.hpp"
#include "world/blocks.hpp"
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cctype>


client_actor::client_actor (caf::actor_config& cfg, const caf::actor& srv,
                            caf::actor script_eng, unsigned int client_id)
  : caf::event_based_actor (cfg), srv (srv), script_eng (script_eng), curr_state (connection_state::handshake),
    inv (window_spec::player_inventory)
{
  this->info.id = client_id;
}



caf::behavior
client_actor::make_behavior ()
{
  return {
      [this] (broker_atom, caf::actor broker) {
        this->broker = broker;
      },

      //
      // Whole packets received from the client's associated broker.
      //
      [this] (packet_in_atom, std::vector<char> buf) {
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
      },

      [this] (packet_out_atom, const std::vector<char>& buf) {
        return this->delegate (this->broker, packet_out_atom::value, buf);
      },

      // messages from scripting engine:

      [this] (s_get_pos_atom, int sid) {
        this->send (this->script_eng, s_get_pos_atom::value, sid, this->pos, this->rot);
      },
  };
}


void
client_actor::send_packet (packet_writer& writer)
{
  this->send (this->broker, packet_out_atom::value, writer.move_data ());
}

void
client_actor::send_packet (packet_writer&& writer)
{
  this->send (this->broker, packet_out_atom::value, writer.move_data ());
}


void
client_actor::handle_packet (packet_reader& reader)
{
  //caf::aout (this) << "Received packet of size " << reader.size () << std::endl;

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
client_actor::handle_handshake_state_packet (packet_reader& reader)
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
client_actor::handle_play_state_packet (packet_reader& reader)
{
  auto id = reader.read_varlong ();
  //caf::aout (this) << "PLAY packet: " << id << std::endl;
  switch (id)
    {
    case IPI_CHAT_MESSAGE:
      this->handle_chat_message_packet (reader);
      break;

    case IPI_CLIENT_SETTINGS:
      this->handle_client_settings_packet (reader);
      break;

    case IPI_CLOSE_WINDOW:
      this->handle_close_window_packet (reader);
      break;

    case IPI_KEEP_ALIVE:
      this->handle_keep_alive_packet (reader);
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

    case IPI_PLAYER_DIGGING:
      this->handle_player_digging_packet (reader);
      break;

    case IPI_HELD_ITEM_CHANGE:
      this->handle_held_item_change (reader);
      break;

    case IPI_CREATIVE_INVENTORY_ACTION:
      this->handle_creative_inventory_action_packet (reader);
      break;

    case IPI_PLAYER_BLOCK_PLACEMENT:
      this->handle_player_block_placement (reader);
      break;

    default:
      caf::aout (this) << "WARNING: Unknown PLAY state packet (ID " << id << ")" << std::endl;
      break;
    }
}

void
client_actor::handle_status_state_packet (packet_reader& reader)
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
client_actor::handle_login_state_packet (packet_reader& reader)
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
client_actor::handle_handshake_packet (packet_reader& reader)
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
client_actor::handle_login_start_packet (packet_reader& reader)
{
  caf::aout (this) << "Got LOGINSTART packet" << std::endl;

  auto username = reader.read_string (16);
  caf::aout (this) << "Username: " << username << std::endl;

  // get info from server
  this->request (this->srv, caf::infinite, get_client_atom::value, this->info.id).await (
      [=] (const client_info& info) {
        caf::aout (this) << "Got some info." << std::endl;
        // update information
        this->info = info;
        this->info.username = username;

        // update server
        this->send (this->srv, set_client_atom::value, this->info.id, this->info);

        // transition into PLAY state.
        this->curr_state = connection_state::play;
        this->send_packet (packets::login::make_login_success (this->info.uuid, username));
        this->send_packet (packets::play::make_join_game (1, 1, 0, false, 2));

        this->join_world (main_world_name);
      });
}


void
client_actor::handle_chat_message_packet (packet_reader& reader)
{
  auto msg = reader.read_string (256);

  // handle commands
  if (msg.find ('/') == 0)
    {
      this->handle_command (std::move (msg));
      return;
    }

  auto out_msg = this->info.username + ": " + msg;

  this->send (this->srv, global_message_atom::value, out_msg);
}

void
client_actor::handle_client_settings_packet (packet_reader& reader)
{
  auto locale = reader.read_string (16);
  int view_distance = reader.read_byte ();
  int chat_mode = (int)reader.read_varlong ();
  bool chat_colors = reader.read_bool ();
  unsigned skin_parts = reader.read_unsigned_byte ();
  int main_hand = (int)reader.read_varlong ();

}

void
client_actor::handle_close_window_packet (packet_reader& reader)
{

}

void
client_actor::handle_keep_alive_packet (packet_reader& reader)
{
  auto id = (uint64_t)reader.read_long ();
  if (id != this->keep_alive_id)
    throw disconnect ("Keep alive ID mismatch");

  this->keep_alive_id = (uint32_t)-1;
  this->time_since_keep_alive = 0;
}

void
client_actor::handle_player_packet (packet_reader& reader)
{
  bool ground = reader.read_bool ();
  this->update_position (this->pos, this->rot, ground);
}

void
client_actor::handle_player_position_packet (packet_reader& reader)
{
  double x = reader.read_double ();
  double y = reader.read_double ();
  double z = reader.read_double ();
  bool ground = reader.read_bool ();
  this->update_position ({ x, y, z }, this->rot, ground);
}

void
client_actor::handle_player_position_and_look_packet (packet_reader& reader)
{
  double x = reader.read_double ();
  double y = reader.read_double ();
  double z = reader.read_double ();
  float yaw = reader.read_float ();
  float pitch = reader.read_float ();
  bool ground = reader.read_bool ();
  this->update_position ({ x, y, z }, { yaw, pitch }, ground);
}

void
client_actor::handle_player_look_packet (packet_reader& reader)
{
  float yaw = reader.read_float ();
  float pitch = reader.read_float ();
  bool ground = reader.read_bool ();
  this->update_position (this->pos, { yaw, pitch }, ground);
}

void
client_actor::handle_player_digging_packet (packet_reader& reader)
{
  auto status = (int)reader.read_varlong ();
  block_pos pos = reader.read_position ();
  char face = reader.read_byte ();

  caf::aout (this) << "Digging at (" << pos.x << ", " << pos.y << ", " << pos.z << ") [Status: " << status << ", Face: " << (int)face << "]" << std::endl;

  this->send (this->curr_world.actor, set_block_atom::value, pos, (unsigned short)0);
}

void
client_actor::handle_held_item_change (packet_reader& reader)
{
  int idx = (int)reader.read_short ();
  if (idx < 0 || idx > 8)
    return;

  this->hand_slot_idx = idx;
}

void
client_actor::handle_creative_inventory_action_packet (packet_reader& reader)
{
  int idx = reader.read_short ();
  bool has_slot = reader.read_bool ();

  // allow changes only to the hotbar
  if (!this->inv.range ("hotbar").contains (idx))
    return;

  if (has_slot)
    {
      // continue reading slot information (we ignore NBT data for now...)
      auto item_id = (int)reader.read_varlong ();
      auto item_count = (unsigned char)reader.read_byte ();
      auto item = std::make_unique<slot> (item_id, item_count);
      this->inv.set_slot (idx, std::move (item));
    }
  else
    {
      // empty slot
      this->inv.clear_slot (idx);
    }
}

void
client_actor::handle_player_block_placement (packet_reader& reader)
{
  auto hand = reader.read_varlong ();
  auto location = reader.read_position ();
  auto face = reader.read_varlong ();
  auto cursor_x = reader.read_float ();
  auto cursor_y = reader.read_float ();
  auto cursor_z = reader.read_float ();
  auto inside_block = reader.read_bool ();

  auto item = this->held_item ();
  if (!item)
    return; // not holding anything


  // use the face value to calculate the position of the block we are going to place
  auto place_pos = location;
  switch (face)
    {
    case 0: --place_pos.y; break;
    case 1: ++place_pos.y; break;
    case 2: --place_pos.z; break;
    case 3: ++place_pos.z; break;
    case 4: --place_pos.x; break;
    case 5: ++place_pos.x; break;
    default: ;
    }

  auto& item_name = registries::name (ITEM_REGISTRY, item->id ());
  auto block_id = (unsigned short)block::find (item_name).get_id ();

  this->send (this->curr_world.actor, set_block_atom::value, place_pos, block_id);
}


void
client_actor::handle_command (std::string&& msg)
{
  std::istringstream ss (msg);
  std::string cmd_name;
  ss >> cmd_name;

  cmd_name.erase (0, 1); // remove /
  std::transform (cmd_name.begin (), cmd_name.end (), cmd_name.begin (),
      [] (char c) { return std::tolower ((unsigned char)c); }); // convert to lower case

  // send command to scripting engine
  this->send (this->script_eng, run_command_atom::value, cmd_name, msg, this);
}


void
client_actor::join_world (const std::string& world_name)
{
  caf::aout (this) << "Joining world: " << world_name << std::endl;

  this->request (this->srv, caf::infinite, get_world_atom::value, world_name).await (
      [=] (const world_info& info) {
        caf::aout (this) << "Got world information of \"" << info.name << "\" from server." << std::endl;
        this->curr_world = info;

        this->pos = player_pos (0, 66.0, 0);

        // send initial chunks
        this->update_chunks ();

        // spawn player
        this->send_packet (packets::play::make_spawn_position (this->pos));
        this->send_packet (packets::play::make_player_position_and_look (
            this->pos, this->rot, 0, 1));
      });
}

void
client_actor::update_position (player_pos pos, player_rot rot, bool ground)
{
  this->pos = pos;
  this->rot = rot;
  this->ground = ground;

  this->call_tick ();
  this->update_chunks ();
}

void
client_actor::call_tick ()
{
  double this_time = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::high_resolution_clock::now ().time_since_epoch ()).count () / 1000.0;
  if (this->first_tick)
    {
      this->last_tick_time = this_time;
      this->first_tick = false;
      return;
    }

  double elapsed = this_time - this->last_tick_time;
  if (elapsed >= 1.0)
    {
      this->last_tick_time = this_time;
      this->tick ();
    }
}

/*!
 * \brief Tick function that is called every second.
 */
void
client_actor::tick ()
{
  ++ this->elapsed_ticks;

  this->time_since_keep_alive += 1.0;
  if (this->time_since_keep_alive >= 5.0)
    {
      if (this->keep_alive_id == (uint32_t)-1)
        {
          // send keep alive
          this->keep_alive_id = (uint32_t)this->elapsed_ticks;
          this->time_since_keep_alive = 0.0;
          this->send_packet (packets::play::make_keep_alive (this->keep_alive_id));
        }
      else
      {
        // did not get response to keep alive: disconnect client
        throw disconnect ("Timed out");
      }
    }
}


/*!
 * \brief Loads or unloads new/old chunks based on the player's position.
 */
void
client_actor::update_chunks ()
{
  chunk_pos pos = this->pos;
  if (pos == this->last_cpos)
    return;

  // unload prev chunks
  for (int x = this->last_cpos.x - chunk_radius; x <= this->last_cpos.x + chunk_radius; ++x)
  for (int z = this->last_cpos.z - chunk_radius; z <= this->last_cpos.z + chunk_radius; ++z)
    {
      if (x < pos.x - chunk_radius || x > pos.x + chunk_radius ||
          z < pos.z - chunk_radius || z > pos.z + chunk_radius)
        {
          // chunk outside chunk radius
          //caf::aout (this) << "Unloading chunk: " << x << "," << z << std::endl;
          this->send_packet (packets::play::make_unload_chunk (x, z));
        }
    }

  // load new chunks
  for (int x = pos.x - chunk_radius; x <= pos.x + chunk_radius; ++x)
  for (int z = pos.z - chunk_radius; z <= pos.z + chunk_radius; ++z)
    {
      auto key = std::make_pair (x, z);
      if (x < this->last_cpos.x - chunk_radius || x > this->last_cpos.x + chunk_radius ||
          z < this->last_cpos.z - chunk_radius || z > this->last_cpos.z + chunk_radius)
        {
          //caf::aout (this) << "Sending chunk " << x << "," << z << std::endl;
          this->send (this->curr_world.actor, request_chunk_data_atom::value, x, z, this->broker);
        }
    }

  this->last_cpos = pos;
}



//! \brief Returns the slot item currently held by the player.
slot*
client_actor::held_item ()
{
  return this->inv.get_slot ("hotbar", this->hand_slot_idx);
}
