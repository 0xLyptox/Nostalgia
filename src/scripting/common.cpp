//
// Created by Jacob Zhitomirsky on 14-Sep-19.
//

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
