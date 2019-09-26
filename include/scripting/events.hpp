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

#ifndef NOSTALGIA_SCRIPTING_EVENTS_HPP
#define NOSTALGIA_SCRIPTING_EVENTS_HPP

#include "scripting/common.hpp"


namespace script {

//! \brief stores an event handler for a given event in the specified player's event handler table.
void store_player_event_handler (lua_State *L, unsigned int player_id, const char *event_name, int func_idx);

//! \brief Loads onto the stack the event handler for the specified event for a given player.
void load_player_event_handler (lua_State *L, unsigned int player_id, const char *event_name);

//! \brief Checks whether an even handler exists for the specified player and event.
bool check_player_event_handler (lua_State *L, unsigned int player_id, const char *event_name);

//! \brief Creates and pushes onto the stack an generic event object.
void create_event_object (lua_State *L, const char *event_name);

//! \brief Creates and pushes onto the stack a player event object.
void create_player_event_object (lua_State *L, const char *event_name, unsigned int player_id);
}

#endif //NOSTALGIA_SCRIPTING_EVENTS_HPP
