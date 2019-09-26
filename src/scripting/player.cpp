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

#include "scripting/player.hpp"
#include "scripting/scripting.hpp"
#include "scripting/events.hpp"
#include "system/consts.hpp"


namespace script {

void
create_player_object (lua_State *L, scripting_actor *engine, const client_info& cinfo)
{
  lua_createtable (L, 0, 0);

  // player._magic
  lua_pushstring (L, "_magic");
  lua_pushinteger (L, script::player_object_magic);
  lua_settable (L, -3);

  // player._engine (pointer to scripting_actor)
  lua_pushstring (L, "_engine");
  auto ctx = static_cast<scripting_actor **> (lua_newuserdata (L, sizeof (scripting_actor *)));
  *ctx = engine;
  lua_settable (L, -3);

  // player.id
  lua_pushstring (L, "id");
  lua_pushinteger (L, cinfo.id);
  lua_settable (L, -3);

  // player.name
  lua_pushstring (L, "name");
  lua_pushstring (L, cinfo.username.c_str ());
  lua_settable (L, -3);

  // player.uuid
  lua_pushstring (L, "uuid");
  lua_pushstring (L, cinfo.uuid.str ().c_str ());
  lua_settable (L, -3);

  // player.add_event_listener()
  lua_pushstring (L, "add_event_listener");
  lua_pushcfunction (L, script::player_add_event_listener);
  lua_settable (L, -3);

  // player.message()
  lua_pushstring (L, "message");
  lua_pushcfunction (L, script::player_message);
  lua_settable (L, -3);

  // player.get_position()
  lua_pushstring (L, "get_position");
  lua_pushcfunction (L, script::player_get_position);
  lua_settable (L, -3);

  // player.get_world()
  lua_pushstring (L, "get_world");
  lua_pushcfunction (L, script::player_get_world);
  lua_settable (L, -3);
}

void
get_player_object_from_table (lua_State *L, unsigned int id)
{
  lua_getglobal (L, globals::player_table);
  lua_pushinteger (L, id);
  lua_gettable (L, -2);
  lua_remove (L, -2);
}

bool
is_player_object (lua_State *L, int idx)
{
  return check_object_magic (L, idx, player_object_magic);
}





//
// Player object methods:
//

int
player_message (lua_State *L)
{
  int num_params = lua_gettop (L);
  if (num_params < 2)
    {
      // TODO: invalid argument count
      return 0;
    }

  // verify arguments
  if (!script::is_player_object (L, -num_params))
    {
      // TODO: invalid arguments
      return 0;
    }

  auto engine = script::get_scripting_actor_from_object (L, -num_params);
  auto id = (unsigned int)script::get_int_from_table (L, -num_params, "id");
  auto cinfo = engine->get_client_info (id);
  if (!cinfo)
    {
      std::cout << "Failed to get client info!" << std::endl;
      return 0;
    }

  // construct message from arguments
  std::string msg = color_escape;
  msg += "e"; // start with a yellow color
  for (int i = num_params - 1; i >= 1; --i)
    {
      lua_getglobal (L, "tostring");
      lua_pushvalue (L, -i - 1);
      lua_call (L, 1, 1);
      const char *part = lua_tostring (L, -1);
      if (part)
        msg += part;
      lua_pop (L, 1);
    }

  // send message packet to player
  engine->send (cinfo->actor, message_atom::value, msg);

  return 0;
}


static int
_player_get_position_cont (lua_State *L, int status, lua_KContext kctx)
{
  return 1;
}

int
player_get_position (lua_State *L)
{
  if (lua_gettop (L) != 1)
    {
      // TODO: invalid argument count
      return 0;
    }

  // verify arguments
  if (!script::is_player_object (L, -1))
    {
      // TODO: invalid arguments
      return 0;
    }

  lua_pushinteger (L, YC_GET_POSITION);
  lua_yieldk (L, 1, NULL, _player_get_position_cont);
  return 0;
}

int
player_add_event_listener (lua_State *L)
{
  int num_params = lua_gettop (L);
  if (num_params != 3 || !script::is_player_object (L, -3) || !lua_isstring (L, -2) || !lua_isfunction (L, -1))
    {
      // TODO: invalid arguments
      return 0;
    }

  auto id = (unsigned int)script::get_int_from_table (L, -num_params, "id");
  script::store_player_event_handler (L, id, lua_tostring (L, -2), -1);

  return 0;
}

static int
_player_get_world_cont (lua_State *L, int status, lua_KContext kctx)
{
  return 1;
}

int
player_get_world (lua_State *L)
{
  int num_params = lua_gettop (L);
  if (num_params != 1 || !script::is_player_object (L, -1))
    {
      // TODO: invalid arguments
      return 0;
    }

  lua_pushinteger (L, YC_GET_WORLD);
  lua_yieldk (L, 1, NULL, _player_get_world_cont);

  return 0;
}

}



void
scripting_actor::register_player (const client_info& cinfo)
{
  auto L = static_cast<lua_State *> (this->vm);
  lua_getglobal (L, script::globals::player_table);
  lua_pushinteger (L, cinfo.id);
  script::create_player_object (L, this, cinfo);
  lua_settable (L, -3);
  lua_pop (L, 1); // pop table

  this->client_info_table[cinfo.id] = cinfo;
}

void
scripting_actor::unregister_player (unsigned int player_id)
{
  auto itr = this->client_info_table.find (player_id);
  if (itr == this->client_info_table.end ())
    return;

  this->client_info_table.erase (itr);

  auto L = static_cast<lua_State *> (this->vm);
  lua_getglobal (L, script::globals::player_table);
  lua_pushinteger (L, player_id);
  lua_pushnil (L);
  lua_settable (L, -3);
  lua_pop (L, 1);
}
