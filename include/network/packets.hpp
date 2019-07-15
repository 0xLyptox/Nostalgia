//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#ifndef NOSTALGIA_PACKETS_HPP
#define NOSTALGIA_PACKETS_HPP

#include "util/uuid.hpp"
#include "network/packet_writer.hpp"
#include <string>


namespace packets::play {

  packet_writer make_disconnect (const std::string& msg);

  packet_writer make_join_game (int entity_id, int gamemode, int dimension, int difficulty, bool reduced_dbg_info);

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
