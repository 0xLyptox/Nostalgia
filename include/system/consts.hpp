//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#ifndef NOSTALGIA_CONSTS_HPP
#define NOSTALGIA_CONSTS_HPP

constexpr const char *current_version_name = "1.13.2";
constexpr int current_procotol_version = 404;

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

  // play state
  IPI_CLIENT_SETTINGS = 0x04,
  IPI_PLAYER = 0x0F,
  IPI_PLAYER_POSITION = 0x10,
  IPI_PLAYER_POSITION_AND_LOOK = 0x11,
  IPI_PLAYER_LOOK = 0x12,

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
  OPI_CHUNK_DATA = 0x22,
  OPI_JOIN_GAME = 0x25,
  OPI_PLAYER_POSITION_AND_LOOK = 0x32,
  OPI_SPAWN_POSITION = 0x49,

  // status state
  OPI_RESPONSE = 0x00,
  OPI_PONG = 0x01,

  // login state
  OPI_DISCONNECT_LOGIN = 0x00,
  OPI_LOGIN_SUCCESS = 0x02,
};

#endif //NOSTALGIA_CONSTS_HPP
