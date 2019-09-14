//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#include "network/packets.hpp"
#include "system/consts.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <cstring>

using json = nlohmann::json;


static std::vector<std::string> _color_names {
    "black",
    "dark_blue",
    "dark_green",
    "dark_aqua",
    "dark_red",
    "dark_purple",
    "gold",
    "gray",
    "dark_gray",
    "blue",
    "green",
    "aqua",
    "red",
    "light_purple",
    "yellow",
    "white"
};

namespace packets::play {

  packet_writer
  make_block_change (block_pos pos, unsigned short block_id)
  {
    packet_writer writer;
    writer.write_varlong (OPI_BLOCK_CHANGE);
    writer.write_position (pos);
    writer.write_varlong (block_id);

    return writer;
  }

  packet_writer
  make_chat_message_simple (const std::string& msg, char position)
  {
    packet_writer writer;
    writer.write_varlong (OPI_CHAT_MESSAGE);

    json j = {
        { "text", "" },
        { "extra", nlohmann::json::array () }
    };

    int col = 0xf;
    size_t pos = 0, end;
    std::string part;
    while (pos < msg.length ())
      {
        end = msg.find (color_escape, pos);
        if (end == std::string::npos)
          part = msg.substr (pos);
        else
          part = msg.substr (pos, end - pos);
        j["extra"].push_back ({
            { "text", part },
            { "color", _color_names[col] }
        });

        if (end == std::string::npos)
          break;

        pos = end + std::strlen (color_escape);
        if (pos >= msg.length ())
          break;
        col = msg[pos];
        if (col >= '0' && col <= '9')
          col = col - '0';
        else if (col >= 'a' && col <= 'f')
          col = col - 'a' + 10;
        else if (col >= 'A' && col <= 'F')
          col = col - 'A' + 10;
        ++ pos;
      }


    writer.write_string (j.dump ());
    writer.write_byte ((unsigned char)position);

    return writer;
  }

  packet_writer
  make_disconnect (const std::string& msg)
  {
    packet_writer writer;
    writer.write_varlong (OPI_DISCONNECT);

    json j = {
        {"text", msg}
    };
    writer.write_string (j.dump ());

    return writer;
  }

  packet_writer
  make_unload_chunk (int x, int z)
  {
    packet_writer writer;
    writer.write_varlong (OPI_UNLOAD_CHUNK);
    writer.write_int (x);
    writer.write_int (z);
    return writer;
  }

  packet_writer
  make_keep_alive (uint64_t id)
  {
    packet_writer writer;
    writer.write_varlong (OPI_KEEP_ALIVE);
    writer.write_long (id);
    return writer;
  }

  packet_writer
  make_join_game (int entity_id, int gamemode, int dimension, bool reduced_dbg_info, int view_distance)
  {
    packet_writer writer;
    writer.write_varlong (OPI_JOIN_GAME);
    writer.write_int (entity_id);
    writer.write_byte ((int8_t)gamemode);
    writer.write_int (dimension);
    writer.write_byte (0);  // max players (unused nowadays)
    writer.write_string ("");  // level type
    writer.write_varlong (view_distance);
    writer.write_bool (reduced_dbg_info);
    return writer;
  }

  packet_writer
  make_spawn_position (block_pos pos)
  {
    packet_writer writer;
    writer.write_varlong (OPI_SPAWN_POSITION);
    writer.write_position (pos);
    return writer;
  }

  packet_writer make_player_position_and_look (player_pos pos, player_rot rot,
                                               unsigned char flags, int teleport_id)
  {
    packet_writer writer;
    writer.write_varlong (OPI_PLAYER_POSITION_AND_LOOK);
    writer.write_double (pos.x);
    writer.write_double (pos.y);
    writer.write_double (pos.z);
    writer.write_float (rot.yaw);
    writer.write_float (rot.pitch);
    writer.write_byte (flags);
    writer.write_varlong (teleport_id);
    return writer;
  }
}

namespace packets::status {

  packet_writer
  make_response (const std::string& version, int protocol, int max_players,
                        int num_online, const std::string& description)
  {
    packet_writer writer;
    writer.write_varlong (OPI_RESPONSE);

    json j = {
        {"version", {
            {"name", version},
            {"protocol", protocol}
        }},
        {"players", {
            {"max", max_players},
            {"online", num_online},
            {"sample", json::array ()}
        }},
        {"description", {
            {"text", description}
        }}
    };
    writer.write_string (j.dump ());

    return writer;
  }

  packet_writer
  make_pong (int64_t num)
  {
    packet_writer writer;
    writer.write_varlong (OPI_PONG);
    writer.write_long (num);
    return writer;
  }
}

namespace packets::login {

  packet_writer
  make_disconnect (const std::string& msg)
  {
    packet_writer writer;
    writer.write_varlong (OPI_DISCONNECT_LOGIN);

    json j = {
        {"text", msg}
    };
    writer.write_string (j.dump ());

    return writer;
  }

  packet_writer
  make_login_success (const uuid_t& uuid, const std::string& username)
  {
    packet_writer writer;
    writer.write_varlong (OPI_LOGIN_SUCCESS);
    writer.write_uuid_string (uuid);
    writer.write_string (username);
    return writer;
  }
}
