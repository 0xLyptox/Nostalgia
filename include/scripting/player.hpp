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

#ifndef NOSTALGIA_SCRIPTING_PLAYER_HPP
#define NOSTALGIA_SCRIPTING_PLAYER_HPP

#include "scripting/common.hpp"
#include "system/info.hpp"


// forward decs:
class scripting_actor;

namespace script {

/*!
 * \brief Pushes a player object onto the specified thread's lua stack.
 * \param L The lua thread to operate on.
 * \param cinfo Client information.
 */
void create_player_object (lua_State *L, scripting_actor *engine, const client_info& cinfo);

//! \brief Pushes a player object onto the stack from the global _PLAYERS table.
void get_player_object_from_table (lua_State *L, unsigned int id);

//! \brief Checks whether the stack item at the specified index is a player object.
bool is_player_object (lua_State *L, int idx);


//
// Player object methods:
//

//! \brief player:message(args...)
int player_message (lua_State *L);

//! \brief player:get_position()
int player_get_position (lua_State *L);

//! \brief player:add_event_listener(event_name, callback)
int player_add_event_listener (lua_State *L);

//! \brief player:get_world()
int player_get_world (lua_State *L);
}

#endif //NOSTALGIA_SCRIPTING_PLAYER_HPP
