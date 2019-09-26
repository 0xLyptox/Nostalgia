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

#include "scripting/scripting.hpp"
#include "scripting/player.hpp"
#include "scripting/events.hpp"
#include "scripting/world.hpp"
#include "network/packets.hpp"
#include "util/position.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <lua.hpp>
#include <system/consts.hpp>

using namespace std::chrono_literals;


//
// Exception types:
//

class script_not_found : public std::exception {};

class lua_exception : public std::runtime_error
{
 public:
  lua_exception (const std::string& str)
    : std::runtime_error (str)
  {}
};

//----

scripting_actor::scripting_actor (caf::actor_config& cfg)
  : caf::blocking_actor (cfg)
{
  lua_State *L = luaL_newstate ();
  luaL_openlibs (L);
  this->vm = L;

  // create table that will hold all thread references
  lua_newtable (L);
  lua_setglobal (L, script::globals::thread_table);

  // create table that will hold all event handlers
  lua_newtable (L);
  lua_setglobal (L, script::globals::player_event_handler_table);

  // create table that will hold all player objects
  lua_newtable (L);
  lua_setglobal (L, script::globals::player_table);

  // create table that will hold all world objects
  lua_newtable (L);
  lua_setglobal (L, script::globals::world_table);
}

scripting_actor::~scripting_actor ()
{
  auto L = static_cast<lua_State *> (this->vm);
  lua_close (L);
}


//! \brief Searches for the script state whose ID matches the specified argument.
script_state*
scripting_actor::find_state (int sid)
{
  // TODO: consider using an associative array for this or something
  auto itr = std::find_if (this->states.begin (), this->states.end (),
      [sid] (const script_state& state) { return state.id == sid; });
  if (itr == this->states.end ())
    return nullptr;
  return &*itr;
}


void
scripting_actor::act ()
{
  bool running = true;
  this->receive_while (running) (
    [&] (stop_atom) {
      running = false;
    },

    [&] (run_script_basic_atom, const std::string& path) {
      this->run_script_basic (path);
    },

    [&] (load_commands_atom, const std::string& path) {
      this->load_commands (path);
    },

    [&] (run_command_atom, const std::string& cmd, const std::string& msg, const client_info& cinfo) {
      this->handle_runcmd (cmd, msg, cinfo);
    },


    [&] (register_player_atom, const client_info& info) {
      this->register_player (info);
    },

    [&] (unregister_player_atom, unsigned int id) {
      this->unregister_player (id);
    },

    [&] (register_world_atom, const world_info& info) {
      this->register_world (info);
    },

    [&] (unregister_world_atom, unsigned int id) {
      this->unregister_world (id);
    },


    // event triggers:

    [&] (s_evt_player_chat_atom, unsigned int client_id, int cont_id, const std::string& msg) {
      this->handle_player_chat_event_trigger (client_id, cont_id, msg);
    },

    [&] (s_evt_player_change_block_atom, unsigned int client_id, int cont_id, block_pos pos, unsigned short block_id) {
      this->handle_player_changeblock_event_trigger (client_id, cont_id, pos, block_id);
    },


    // response messages:

    [&] (s_get_pos_atom, int sid, player_pos pos, player_rot rot) {
      auto state = this->find_state (sid);
      if (!state) return;
      if (state->last_yield_code != script::YC_GET_POSITION) return; // shouldn't happen...

      // push result
      auto L = static_cast<lua_State *> (state->thread);
      lua_createtable (L, 0, 5);
      lua_pushstring (L, "x");
      lua_pushnumber (L, pos.x);
      lua_settable (L, -3);
      lua_pushstring (L, "y");
      lua_pushnumber (L, pos.y);
      lua_settable (L, -3);
      lua_pushstring (L, "z");
      lua_pushnumber (L, pos.z);
      lua_settable (L, -3);
      lua_pushstring (L, "yaw");
      lua_pushnumber (L, rot.yaw);
      lua_settable (L, -3);
      lua_pushstring (L, "pitch");
      lua_pushnumber (L, rot.pitch);
      lua_settable (L, -3);

      this->resume_script (*state, 1);
    },

    [&] (s_get_world_atom , int sid, unsigned int world_id) {
      auto state = this->find_state (sid);
      if (!state) return;
      if (state->last_yield_code != script::YC_GET_WORLD) return; // shouldn't happen...

      // push result
      auto L = static_cast<lua_State *> (state->thread);

      script::get_world_object_from_table (L, world_id);

      this->resume_script (*state, 1);
    },

    [&] (s_get_block_id_atom, int sid, unsigned short id) {
      auto state = this->find_state (sid);
      if (!state) return;
      if (state->last_yield_code != script::YC_GET_BLOCK) return; // shouldn't happen...

      // push result
      auto L = static_cast<lua_State *> (state->thread);
      lua_pushinteger (L, id);

      this->resume_script (*state, 1);
    },


    // timeout:
    caf::after (5ms) >> [&] () {
      this->handle_timeout ();
    }
  );
}

void
scripting_actor::handle_runcmd (const std::string& cmd, const std::string& msg, const client_info& cinfo)
{
  auto& state = this->create_state (script_type::command, true);
  state.target = cinfo.actor;

  this->run_command (state, cmd, msg, cinfo);
}

void
scripting_actor::handle_timeout ()
{

}



client_info*
scripting_actor::get_client_info (unsigned int player_id)
{
  auto itr = this->client_info_table.find (player_id);
  if (itr == this->client_info_table.end ())
    return nullptr;
  return &itr->second;
}

world_info*
scripting_actor::get_world_info (unsigned int world_id)
{
  auto itr = this->world_info_table.find (world_id);
  if (itr == this->world_info_table.end ())
    return nullptr;
  return &itr->second;
}


script_state&
scripting_actor::create_state (script_type type, bool new_thread)
{
  auto L = static_cast<lua_State *> (this->vm);

  // create new state
  this->states.emplace_back ();
  auto& state = this->states.back ();
  state.vm = L;
  state.type = type;
  state.id = this->next_state_id ++;
  state.self = this;
  state.last_yield_code = script::YC_NONE;
  state.itr = this->states.end ();
  -- state.itr;

  // create new thread if needed (if so, store a reference to it in the global thread table).
  if (new_thread)
    {
      std::ostringstream thread_name;
      thread_name << "thread_" << state.id;
      lua_getglobal (L, script::globals::thread_table);
      lua_pushstring (L, thread_name.str ().c_str ());
      state.thread = lua_newthread (L);
      lua_settable (L, -3);
      lua_pop (L, 1);
    }
  else
    state.thread = L;

  return state;
}

void
scripting_actor::delete_state (script_state& state)
{
  if (state.thread != state.vm)
    {
      // remove thread from global thread table so that it could be garbage collected.
      auto L = static_cast<lua_State *> (state.vm);
      std::ostringstream thread_name;
      thread_name << "thread_" << state.id;
      lua_getglobal (L, script::globals::thread_table);
      lua_pushstring (L, thread_name.str ().c_str ());
      lua_pushnil (L);
      lua_settable (L, -3);
      lua_pop (L, 1);
    }
  this->states.erase (state.itr);
}


void
scripting_actor::run_script_basic (const std::string& script_path)
{
  // open script file
  std::ifstream fs (script_path);
  if (!fs)
    throw script_not_found {};

  // read contents into string
  std::stringstream ss;
  ss << fs.rdbuf ();
  auto script = ss.str ();

  // load script in context of main VM
  auto L = static_cast<lua_State *> (this->vm);
  int err = luaL_loadstring (L, script.c_str ()) || lua_pcall (L, 0, 0, 0);
  if (err)
    {
      std::string err_msg = lua_tostring (L, -1);
      this->states.pop_back ();
      throw lua_exception (err_msg);
    }
}

void
scripting_actor::run_command (script_state& state, const std::string& cmd, const std::string& msg,
                              const client_info& cinfo)
{
  auto L = static_cast<lua_State *> (state.thread);
  lua_getglobal (L, "cmd");
  lua_pushstring (L, ("do_" + cmd).c_str ());
  lua_gettable (L, -2);
  bool cmd_exists = lua_isfunction (L, -1);
  if (!cmd_exists)
    {
      this->send (cinfo.actor, message_atom::value, "No such command: " + cmd);
      this->delete_state (state);
      return;
    }

  script::get_player_object_from_table (L, cinfo.id);
  lua_pushstring (L, msg.c_str ());

  this->resume_script (state, 2);
}

void
scripting_actor::load_commands (const std::string& path)
{
  caf::aout (this) << "Loading commands..." << std::endl;
  for (auto& entry : std::filesystem::directory_iterator (path))
    {
      auto& p = entry.path ();
      if (!entry.is_regular_file ())
        continue;
      if (p.extension () != ".lua")
        continue;

      try
        {
          caf::aout (this) << "    Loading command: " << p.filename ().string () << std::endl;
          this->run_script_basic (p.string ());
        }
//      catch (const script_not_found&)
//        {
//          caf::aout (this) << "Command not found" << std::endl;
//        }
      catch (const lua_exception& ex)
        {
          // TODO:
          caf::aout (this) << "Lua error during load: " << ex.what () << std::endl;
        }
    }

  caf::aout (this) << "    Done" << std::endl;
}


void
scripting_actor::process_yield (script_state& state)
{
  auto L = static_cast<lua_State *> (state.thread);
  auto params = lua_gettop (L);
  if (params == 0)
    {
      this->delete_state (state);
      return;
    }

  auto yield_code = lua_tointeger (L, -params);
  state.last_yield_code = (int)yield_code;
  switch (yield_code)
    {
    case script::YC_GET_POSITION:
      this->send (state.target, s_get_pos_atom::value, state.id);
      break;

    case script::YC_GET_WORLD:
      this->send (state.target, s_get_world_atom::value, state.id);
      break;

    case script::YC_GET_BLOCK:
      {
        auto world_id = (unsigned int)lua_tointeger (L, -4);
        auto x = (int)lua_tointeger (L, -3);
        auto y = (int)lua_tointeger (L, -2);
        auto z = (int)lua_tointeger (L, -1);
        auto itr = this->world_info_table.find (world_id);
        if (itr != this->world_info_table.end ())
          this->send (itr->second.actor, s_get_block_id_atom::value, state.id, x, y, z);

      }
      break;

    default: ;
    }

  // clear stack
  lua_pop (L, params);
}

void
scripting_actor::process_completion (script_state& state)
{
  // done
  if (state.type == script_type::event_handler)
    {
      // an event handler has finished
      // check its event object (which should be the topmost item on the stack)
      // and see if we need to suppress the default handler
      auto L = static_cast<lua_State *> (state.thread);
      lua_pushstring (L, "suppress");
      lua_gettable (L, -2);
      bool suppress = lua_toboolean (L, -1);
      lua_pop (L, 2); // pop both the bool and the event object

      this->send (state.target, event_complete_atom::value, state.cont_id, suppress);
    }

  this->delete_state (state);
}

void
scripting_actor::process_error (script_state& state)
{
  if (state.target && state.type == script_type::command)
    {
      auto L = static_cast<lua_State *> (state.thread);
      std::ostringstream err;
      err << color_escape << "4" << "Error" << color_escape << "c" << ": " << lua_tostring (L, -1);
      this->send (state.target, message_atom::value, err.str ());
    }

  this->delete_state (state);
}

void
scripting_actor::resume_script (script_state& state, int num_args)
{
  auto L = static_cast<lua_State *> (state.thread);

  int status = lua_resume (L, NULL, num_args);
  if (status == LUA_OK)
    {
      this->process_completion (state);
    }
  else if (status == LUA_YIELD)
    {
      // script yielded
      this->process_yield (state);
    }
  else
    {
      this->process_error (state);
    }
}



