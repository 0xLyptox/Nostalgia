//
// Created by Jacob Zhitomirsky on 14-Sep-19.
//

#include "scripting/scripting.hpp"
#include "scripting/player.hpp"
#include <lua.hpp>



namespace script {

void
store_player_event_handler (lua_State *L, unsigned int player_id, const char *event_name, int func_idx)
{
  lua_getglobal (L, script::globals::player_event_handler_table);
  lua_pushinteger (L, player_id);
  lua_gettable (L, -2);

  if (lua_isnil (L, -1))
    {
      // player event handler table does not exist, create it.
      lua_pop(L, 1);
      lua_pushinteger (L, player_id);
      lua_newtable (L);
      lua_settable (L, -3);

      // retrieve it
      lua_pushinteger (L, player_id);
      lua_gettable (L, -2);
    }

  lua_pushstring (L, event_name);
  lua_pushvalue (L, func_idx - 3);
  lua_settable (L, -3);

  lua_pop (L, 2); // pop player event table and global event table
}

void
load_player_event_handler (lua_State *L, unsigned int player_id, const char *event_name)
{
  lua_getglobal (L, script::globals::player_event_handler_table);
  lua_pushinteger (L, player_id);
  lua_gettable (L, -2);

  if (lua_isnil (L, -1))
    {
      // no table for player
      lua_remove (L, -2); // remove table and return nil
      return;
    }

  lua_pushstring (L, event_name);
  lua_gettable (L, -2);
  lua_remove (L, -2); // remove player event handler table
  lua_remove (L, -2); // remove global event handler table
}

bool
check_player_event_handler (lua_State *L, unsigned int player_id, const char *event_name)
{
  load_player_event_handler (L, player_id, event_name);
  auto exists = !lua_isnil (L, -1);
  lua_pop (L, 1);
  return exists;
}

void
create_event_object (lua_State *L, const char *event_name)
{
  lua_createtable (L, 0, 0);

  // ev.event_name
  lua_pushstring (L, "event_name");
  lua_pushstring (L, event_name);
  lua_settable (L, -3);

  // ev.suppress
  lua_pushstring (L, "suppress");
  lua_pushboolean (L, false);
  lua_settable (L, -3);
}

void
create_player_event_object (lua_State *L, const char *event_name, unsigned int player_id)
{
  create_event_object (L, event_name);

  // ev.target
  lua_pushstring (L, "target");
  script::get_player_object_from_table (L, player_id);
  lua_settable (L, -3);
}

}



script_state *
scripting_actor::start_player_event_trigger (const char *event_name, unsigned int client_id, int cont_id)
{
  auto cinfo = this->get_client_info (client_id);
  if (!cinfo)
    return nullptr;

  // check if a handler exists first on main thread
  if (!script::check_player_event_handler (static_cast<lua_State *> (this->vm), client_id, event_name))
    {
      // not event handler found, signal event completion immediately.
      this->send (cinfo->actor, event_complete_atom::value, cont_id, false);
      return nullptr;
    }

  auto& state = this->create_state (script_type::event_handler, true);
  state.cinfo = *cinfo;
  state.cont_id = cont_id;

  auto L = static_cast<lua_State *> (state.thread);

  // create and push event object onto stack
  script::create_player_event_object (L, event_name, cinfo->id);

  // push player event handler onto stack
  script::load_player_event_handler (L, client_id, event_name);

  // push another copy of the event object so that we can call the event handler
  // and after the call still have a reference to the event object.
  lua_pushvalue (L, -2);

  return &state;
}

void
scripting_actor::run_event_trigger (script_state& state)
{
  auto L = static_cast<lua_State *> (state.thread);
  this->resume_script (state, 1);
}

void
scripting_actor::handle_player_chat_event_trigger (unsigned int client_id, int cont_id, const std::string& msg)
{
  if (auto state = this->start_player_event_trigger ("chat", client_id, cont_id))
    {
      auto L = static_cast<lua_State *> (state->thread);

      // ev.msg
      lua_pushstring (L, "msg");
      lua_pushstring (L, msg.c_str ());
      lua_settable (L, -3);

      this->run_event_trigger (*state);
    }
}

void
scripting_actor::handle_player_changeblock_event_trigger (unsigned int client_id, int cont_id, block_pos pos,
                                                          unsigned short block_id)
{
  if (auto state = this->start_player_event_trigger ("changeblock", client_id, cont_id))
    {
      auto L = static_cast<lua_State *> (state->thread);

      // ev.block_pos
      lua_pushstring (L, "block_pos");
      lua_createtable (L, 0, 3);
      lua_pushstring (L, "x");
      lua_pushinteger (L, pos.x);
      lua_settable (L, -3);
      lua_pushstring (L, "y");
      lua_pushinteger (L, pos.y);
      lua_settable (L, -3);
      lua_pushstring (L, "z");
      lua_pushinteger (L, pos.z);
      lua_settable (L, -3);
      lua_settable (L, -3);

      // ev.block_id
      lua_pushstring (L, "block_id");
      lua_pushinteger (L, block_id);
      lua_settable (L, -3);

      this->run_event_trigger (*state);
    }
}
