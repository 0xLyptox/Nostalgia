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

#ifndef NOSTALGIA_CLIENT_HPP
#define NOSTALGIA_CLIENT_HPP

#include "system/server.hpp"
#include "system/consts.hpp"
#include "system/info.hpp"
#include "util/position.hpp"
#include "caf/all.hpp"
#include "window/window.hpp"
#include <functional>
#include <set>
#include <utility>
#include <vector>

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


class client_actor : public caf::event_based_actor
{
  caf::actor srv;
  caf::actor script_eng;
  connection_state curr_state;
  caf::actor broker;
  client_info info;

  world_info curr_world;
  player_pos pos;
  player_rot rot;
  bool ground = true;
  chunk_pos last_cpos = { -0x13371337, 0x13371337 };

  window inv;
  int hand_slot_idx = 0; // the slot which the player has selected (0-8)

  double last_tick_time = 0.0;
  bool first_tick = true;
  uint64_t elapsed_ticks = 0;
  double time_since_keep_alive = 0;
  uint32_t keep_alive_id = (uint32_t)-1;

  std::map<int, std::function<void ()>> handler_continuations;
  std::map<int, bool> immediate_continuations;
  int next_ev_cont_id = 1;

 public:
  [[nodiscard]] inline typed_id get_typed_id () const { return { actor_type::client, this->info.id }; }

  explicit client_actor (caf::actor_config& cfg, const caf::actor& srv,
                         const caf::actor& script_eng, unsigned int client_id);

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
  void handle_close_window_packet (packet_reader& reader);
  void handle_keep_alive_packet (packet_reader& reader);
  void handle_player_packet (packet_reader& reader);
  void handle_player_position_packet (packet_reader& reader);
  void handle_player_position_and_look_packet (packet_reader& reader);
  void handle_player_look_packet (packet_reader& reader);
  void handle_player_digging_packet (packet_reader& reader);
  void handle_held_item_change (packet_reader& reader);
  void handle_creative_inventory_action_packet (packet_reader& reader);
  void handle_player_block_placement (packet_reader& reader);


  void handle_command (const std::string& msg);

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

  //! \brief Returns the slot item currently held by the player.
  slot* held_item ();

  //! \brief Invoked when the actor exits.
  void exit_handler ();

 private:
  friend struct event_trigger_continuation;
  struct event_trigger_continuation
  {
    client_actor& cl;
    int id;

   public:
    explicit event_trigger_continuation (client_actor& cl, int cont_id);
    void set_default_handler (std::function<void ()>&& f);
  };

  template<typename Atom, typename... Args>
  event_trigger_continuation
  fire_event (Atom atom, Args&&... args)
  {
    int cont_id = this->next_ev_cont_id ++;
    this->send (this->script_eng, atom, this->info.id, cont_id, std::forward<Args> (args)...);
    return event_trigger_continuation { *this, cont_id };
  }
};

#endif //NOSTALGIA_CLIENT_HPP
