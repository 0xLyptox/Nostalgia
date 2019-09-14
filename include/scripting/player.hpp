//
// Created by Jacob Zhitomirsky on 14-Sep-19.
//

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
