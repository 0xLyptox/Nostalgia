//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#ifndef NOSTALGIA_CONSTS_HPP
#define NOSTALGIA_CONSTS_HPP

constexpr const char *current_version_name = "1.14.4";
constexpr int current_procotol_version = 498;

constexpr const char *main_world_name = "Main";

constexpr int chunk_radius = 4;
constexpr int max_lighting_updates = 1024;


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
  IPI_CHAT_MESSAGE = 0x03,
  IPI_CLIENT_SETTINGS = 0x05,
  IPI_CLOSE_WINDOW = 0x0A,
  IPI_KEEP_ALIVE = 0x0F,
  IPI_PLAYER = 0x10,
  IPI_PLAYER_POSITION = 0x11,
  IPI_PLAYER_POSITION_AND_LOOK = 0x12,
  IPI_PLAYER_LOOK = 0x13,
  IPI_PLAYER_DIGGING = 0x1A,
  IPI_HELD_ITEM_CHANGE = 0x23,
  IPI_CREATIVE_INVENTORY_ACTION = 0x26,
  IPI_PLAYER_BLOCK_PLACEMENT = 0x2C,

  // status state
  IPI_REQUEST = 0x00,
  IPI_PING = 0x01,

  // login state
  IPI_LOGIN_START = 0x00,
};

enum out_packet_id
{
  // play state
  OPI_CHAT_MESSAGE = 0x0E,
  OPI_DISCONNECT = 0x1A,
  OPI_UNLOAD_CHUNK = 0x1D,
  OPI_KEEP_ALIVE = 0x20,
  OPI_CHUNK_DATA = 0x21,
  OPI_JOIN_GAME = 0x25,
  OPI_PLAYER_POSITION_AND_LOOK = 0x35,
  OPI_SPAWN_POSITION = 0x4D,

  // status state
  OPI_RESPONSE = 0x00,
  OPI_PONG = 0x01,

  // login state
  OPI_DISCONNECT_LOGIN = 0x00,
  OPI_LOGIN_SUCCESS = 0x02,
};

#endif //NOSTALGIA_CONSTS_HPP
