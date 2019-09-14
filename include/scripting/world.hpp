//
// Created by Jacob Zhitomirsky on 14-Sep-19.
//

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

}

#endif //NOSTALGIA_SCRIPTING_WORLD_HPP
