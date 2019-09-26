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

#ifndef NOSTALGIA_SCRIPTING_WORLD_HPP
#define NOSTALGIA_SCRIPTING_WORLD_HPP

#include "scripting/common.hpp"
#include "system/info.hpp"
#include <lua.hpp>


// forward decs:
class scripting_actor;

namespace script {

//! \brief Creates a world object and stores it into the global world table.
void create_world_object (lua_State *L, scripting_actor *engine, const world_info& info);

//! \brief Pushes a world object onto the stack from the global world table.
void get_world_object_from_table (lua_State *L, unsigned int id);

//! \brief Checks whether the stack item at the specified index is a world object.
bool is_world_object (lua_State *L, int idx);


//
// Player object methods:
//

//! \brief world:set_block(x, y, z, block_id)
int world_set_block (lua_State *L);

//! \brief world:get_block(x, y, z)
int world_get_block (lua_State *L);

//! \brief world:save()
int world_save (lua_State *L);

}

#endif //NOSTALGIA_SCRIPTING_WORLD_HPP
