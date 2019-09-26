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

#include "scripting/common.hpp"


namespace script {


long long
get_int_from_table (lua_State *L, int idx, const char *field)
{
  lua_pushstring (L, field);
  lua_gettable (L, idx - 1);
  auto val = lua_tointeger (L, -1);
  lua_pop (L, 1);
  return val;
}


scripting_actor*
get_scripting_actor_from_object (lua_State *L, int idx)
{
  lua_pushstring (L, "_engine");
  lua_gettable (L, idx - 1);
  auto engine = *static_cast<scripting_actor **> (lua_touserdata (L, -1));
  lua_pop (L, 1);
  return engine;
}

bool
check_object_magic (lua_State *L, int idx, unsigned int expected)
{
  if (!lua_istable (L, idx))
    return false;

  lua_pushstring (L, "_magic");
  lua_gettable (L, idx - 1);
  auto value = (unsigned int)lua_tointeger (L,-1);
  lua_pop (L, 1);

  return value == expected;
}

}
