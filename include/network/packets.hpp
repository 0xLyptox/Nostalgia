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

#ifndef NOSTALGIA_PACKETS_HPP
#define NOSTALGIA_PACKETS_HPP

#include "util/uuid.hpp"
#include "network/packet_writer.hpp"
#include <string>


namespace packets::play {

  packet_writer make_block_change (block_pos pos, unsigned short block_id);

  packet_writer make_chat_message_simple (const std::string& msg, char position);

  packet_writer make_disconnect (const std::string& msg);

  packet_writer make_unload_chunk (int x, int z);

  packet_writer make_keep_alive (uint64_t id);

  packet_writer make_join_game (int entity_id, int gamemode, int dimension, bool reduced_dbg_info, int view_distance);

  packet_writer make_spawn_position (block_pos pos);

  packet_writer make_player_position_and_look (player_pos pos, player_rot rot,
      unsigned char flags, int teleport_id);
}

namespace packets::status {

  packet_writer make_response (const std::string& version, int protocol, int max_players,
                               int num_online, const std::string& description);

  packet_writer make_pong (int64_t num);
}

namespace packets::login {

  packet_writer make_disconnect (const std::string& msg);

  packet_writer make_login_success (const uuid_t& uuid, const std::string& username);
}

#endif //NOSTALGIA_PACKETS_HPP
