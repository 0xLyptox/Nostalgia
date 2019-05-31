//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#include "packets.hpp"
#include "consts.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace packets::play {

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
  make_join_game (int entity_id, int gamemode, int dimension, int difficulty, bool reduced_dbg_info)
  {
    //
    // TODO: Figure out how to encode this packet properly because it looks
    //       like it changed in 1.14.
    //
    packet_writer writer;
    writer.write_varlong (OPI_JOIN_GAME);
    writer.write_int (entity_id);
    writer.write_byte ((int8_t)gamemode);
    writer.write_int (dimension);
    writer.write_byte ((int8_t)difficulty);
    writer.write_byte (0);  // max players (unused nowadays)
    writer.write_string ("");  // level type
    writer.write_bool (reduced_dbg_info);
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
