//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#ifndef NOSTALGIA_CLIENT_HPP
#define NOSTALGIA_CLIENT_HPP

#include "system/consts.hpp"
#include "system/info.hpp"
#include "util/position.hpp"
#include "caf/all.hpp"
#include <set>
#include <utility>

// forward decs:
class packet_reader;
class packet_writer;


/*!
 * \brief Thrown by functions called by handle_packet to force the client
 *        to be disconnected with a message.
 */
class disconnect : public std::exception
{
  std::string msg;

 public:
  explicit disconnect (const std::string& msg)
    : msg (msg)
  {}

  inline const std::string& message () const { return this->msg; }
};


class client : public caf::event_based_actor
{
  caf::actor srv;
  connection_state curr_state;
  caf::actor broker;
  client_info info;

  world_info curr_world;
  player_pos pos;
  player_rot rot;
  bool ground = true;
  chunk_pos last_cpos = { -0x13371337, 0x13371337 };

  double last_tick_time = 0.0;
  bool first_tick = true;
  uint64_t elapsed_ticks = 0;
  double time_since_keep_alive = 0;
  uint32_t keep_alive_id = (uint32_t)-1;

 public:
  explicit client (caf::actor_config& cfg, const caf::actor& srv,
                   unsigned int client_id);

  caf::behavior make_behavior () override;

 private:
  void handle_packet (packet_reader& reader);

  void handle_handshake_state_packet (packet_reader& reader);
  void handle_play_state_packet (packet_reader& reader);
  void handle_status_state_packet (packet_reader& reader);
  void handle_login_state_packet (packet_reader& reader);

  void handle_handshake_packet (packet_reader& reader);

  void handle_login_start_packet (packet_reader& reader);

  void handle_chat_message_packet (packet_reader& reader);
  void handle_client_settings_packet (packet_reader& reader);
  void handle_keep_alive_packet (packet_reader& reader);
  void handle_player_packet (packet_reader& reader);
  void handle_player_position_packet (packet_reader& reader);
  void handle_player_position_and_look_packet (packet_reader& reader);
  void handle_player_look_packet (packet_reader& reader);
  void handle_player_digging_packet (packet_reader& reader);


  /*!
   * \brief Sends the contents of the specified packet writer.
   * \param writer The packet writer that contains the packet to send.
   *
   * NOTE: This invalidates the packet writer as it moves its buffer.
   */
  void send_packet (packet_writer& writer);
  void send_packet (packet_writer&& writer);

  void join_world (const std::string& world_name);

  void update_position (player_pos pos, player_rot rot, bool ground);

  /*!
   * \brief Tick function that is called every second.
   */
  void tick ();

  void call_tick ();

  /*!
   * \brief Loads or unloads new/old chunks based on the player's position.
   */
  void update_chunks ();
};

#endif //NOSTALGIA_CLIENT_HPP
