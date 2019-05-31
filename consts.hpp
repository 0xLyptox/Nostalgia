//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#ifndef NOSTALGIA_CONSTS_HPP
#define NOSTALGIA_CONSTS_HPP

constexpr const char *current_version_name = "1.14.1";
constexpr int current_procotol_version = 480;

constexpr const char *main_world_name = "Main";


enum class connection_state
{
  handshake,
  play,
  status,
  login
};

enum in_packet_id
{
  // handshake state
  IPI_HANDSHAKE = 0x00,

  // status state
  IPI_REQUEST = 0x00,
  IPI_PING = 0x01,

  // login state
  IPI_LOGIN_START = 0x00,
};

enum out_packet_id
{
  // play state
  OPI_DISCONNECT = 0x1B,
  OPI_JOIN_GAME = 0x25,

  // status state
  OPI_RESPONSE = 0x00,
  OPI_PONG = 0x01,

  // login state
  OPI_DISCONNECT_LOGIN = 0x00,
  OPI_LOGIN_SUCCESS = 0x02,
};

#endif //NOSTALGIA_CONSTS_HPP
