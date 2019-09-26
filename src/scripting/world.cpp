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

#include "scripting/world.hpp"
#include "scripting/scripting.hpp"


namespace script {

void
create_world_object (lua_State *L, scripting_actor *engine, const world_info& info)
{
  lua_createtable (L, 0, 0);

  // world._magic
  lua_pushstring (L, "_magic");
  lua_pushinteger (L, script::world_object_magic);
  lua_settable (L, -3);

  // world._engine (pointer to scripting_actor)
  lua_pushstring (L, "_engine");
  auto ctx = static_cast<scripting_actor **> (lua_newuserdata (L, sizeof (scripting_actor *)));
  *ctx = engine;
  lua_settable (L, -3);

  // world.id
  lua_pushstring (L, "id");
  lua_pushinteger (L, info.id);
  lua_settable (L, -3);

  // world.name
  lua_pushstring (L, "name");
  lua_pushstring (L, info.name.c_str ());
  lua_settable (L, -3);

  // world:set_block()
  lua_pushstring (L, "set_block");
  lua_pushcfunction (L, script::world_set_block);
  lua_settable (L, -3);

  // world:set_block()
  lua_pushstring (L, "get_block");
  lua_pushcfunction (L, script::world_get_block);
  lua_settable (L, -3);

  // world:save()
  lua_pushstring (L, "save");
  lua_pushcfunction (L, script::world_save);
  lua_settable (L, -3);
}

void
get_world_object_from_table (lua_State *L, unsigned int id)
{
  lua_getglobal (L, globals::world_table);
  lua_pushinteger (L, id);
  lua_gettable (L, -2);
  lua_remove (L, -2);
}

bool
is_world_object (lua_State *L, int idx)
{
  return check_object_magic (L, idx, world_object_magic);
}


//
// Player object methods:
//

int
world_set_block (lua_State *L)
{
  int num_params = lua_gettop (L);
  if (num_params != 5)
    {
      // TODO: invalid argument count
      return 0;
    }

  // verify arguments
  if (!script::is_world_object (L, -5) || !lua_isinteger (L, -4)
      || !lua_isinteger (L, -3) || !lua_isinteger (L, -2) || !lua_isinteger (L, -1))
    {
      // TODO: invalid arguments
      return 0;
    }

  auto engine = script::get_scripting_actor_from_object (L, -num_params);
  auto id = (unsigned int)script::get_int_from_table (L, -num_params, "id");
  auto info = engine->get_world_info (id);
  if (!info)
    {
      std::cout << "Failed to get world info!" << std::endl;
      return 0;
    }

  block_pos pos = {
      (int)lua_tointeger (L, -4), // x
      (int)lua_tointeger (L, -3), // y
      (int)lua_tointeger (L, -2), // z
  };

  auto block_id = (unsigned short)lua_tointeger (L, -1);
  engine->send (info->actor, set_block_atom::value, pos, block_id);

  return 0;
}

static int
_world_get_block_cont (lua_State *L, int status, lua_KContext kctx)
{
  return 1;
}

int
world_get_block (lua_State *L)
{
  int num_params = lua_gettop (L);
  if (num_params != 4)
    {
      // TODO: invalid argument count
      return 0;
    }

  // verify arguments
  if (!script::is_world_object (L, -4) || !lua_isinteger (L, -3)
      || !lua_isinteger (L, -2) || !lua_isinteger (L, -1))
    {
      // TODO: invalid arguments
      return 0;
    }

  auto engine = script::get_scripting_actor_from_object (L, -num_params);
  auto id = (unsigned int)script::get_int_from_table (L, -num_params, "id");

  lua_pushinteger (L, YC_GET_BLOCK);
  lua_pushinteger (L, id);
  lua_pushvalue (L, -5); // x
  lua_pushvalue (L, -5); // y
  lua_pushvalue (L, -5); // z
  lua_yieldk (L, 5, NULL, _world_get_block_cont);
  return 0;
}

int
world_save (lua_State *L)
{
  int num_params = lua_gettop (L);
  if (num_params != 1 || !script::is_world_object (L, -1))
    {
      // TODO: invalid arguments
      return 0;
    }

  auto engine = script::get_scripting_actor_from_object (L, -num_params);
  auto id = (unsigned int)script::get_int_from_table (L, -num_params, "id");
  auto info = engine->get_world_info (id);
  if (!info)
    return 0;

  engine->send (info->actor, save_atom::value);
  return 0;
}

}


void
scripting_actor::register_world (const world_info& info)
{
  caf::aout (this) << "Registering world \"" << info.name << "\" (ID: " << info.id << ")" << std::endl;

  auto L = static_cast<lua_State *> (this->vm);
  lua_getglobal (L, script::globals::world_table);
  lua_pushinteger (L, info.id);
  script::create_world_object (L, this, info);
  lua_settable (L, -3);
  lua_pop (L, 1); // pop table

  this->world_info_table[info.id] = info;
}

void
scripting_actor::unregister_world (unsigned int world_id)
{
  auto itr = this->world_info_table.find (world_id);
  if (itr == this->world_info_table.end ())
    return;

  this->world_info_table.erase (itr);

  auto L = static_cast<lua_State *> (this->vm);
  lua_getglobal (L, script::globals::world_table);
  lua_pushinteger (L, world_id);
  lua_pushnil (L);
  lua_settable (L, -3);
  lua_pop (L, 1);
}
