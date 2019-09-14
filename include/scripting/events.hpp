//
// Created by Jacob Zhitomirsky on 14-Sep-19.
//

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
