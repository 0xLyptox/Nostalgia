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

#ifndef NOSTALGIA_SCRIPTING_COMMON_HPP
#define NOSTALGIA_SCRIPTING_COMMON_HPP

#include <lua.hpp>


// forward decs:
class scripting_actor;

namespace script {

constexpr unsigned int player_object_magic = 0x13370001;
constexpr unsigned int world_object_magic = 0x13370002;

namespace globals {
  constexpr const char *thread_table = "_THREADS";
  constexpr const char *player_event_handler_table = "_EVENT_HANDLERS";
  constexpr const char *player_table = "_PLAYERS";
  constexpr const char *world_table = "_WORLDS";
}

/*!
 * \brief Script yield codes
 * Method calls that require waiting for a response from an actor can yield
 * and thus pause the execution of a script until an answer is retrieved.
 */
enum yield_code
{
  YC_NONE,
  YC_GET_POSITION,
  YC_GET_WORLD,
  YC_GET_BLOCK,
};


//! \brief Returns the integer in the given field from the table at the specified index.
long long get_int_from_table (lua_State *L, int idx, const char *field);



//! \brief Returns the scripting actor pointer stored in objects like player object, world objects, etc...
scripting_actor *get_scripting_actor_from_object (lua_State *L, int idx);

//! \brief Checks whether the _magic field in a object is equal to the specified value.
bool check_object_magic (lua_State *L, int idx, unsigned int expected);

}



#endif //NOSTALGIA_SCRIPTING_COMMON_HPP
