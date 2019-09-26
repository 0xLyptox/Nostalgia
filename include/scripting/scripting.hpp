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

#ifndef NOSTALGIA_SCRIPTING_HPP
#define NOSTALGIA_SCRIPTING_HPP

#include "system/atoms.hpp"
#include "util/position.hpp"
#include "system/info.hpp"
#include <stdexcept>
#include <list>
#include <map>
#include <caf/all.hpp>


// forward decs:
class scripting_actor;

enum class script_type
{
  command,
  event_handler,
};

/*!
 * \brief Holds the state of a running script.
 */
struct script_state
{
  std::list<script_state>::iterator itr;
  script_type type;
  int id;
  void *vm, *thread; // lua_State*
  caf::actor target;
  scripting_actor *self;
  int last_yield_code;
  int cont_id; // continuation ID for event handlers
};

/*!
 * \brief This actor manages requests to run Lua scripts.
 */
class scripting_actor : public caf::blocking_actor
{
  void *vm; // lua_State*
  std::list<script_state> states;
  int next_state_id = 1;

  std::map<unsigned int, client_info> client_info_table;
  std::map<unsigned int, world_info> world_info_table;

 public:
  scripting_actor (caf::actor_config& cfg);
  ~scripting_actor ();

  void act () override;

  //! \brief Searches for the script state whose ID matches the specified argument.
  script_state *find_state (int sid);

  //! \brief Returns the client info structure associated with the specified player/client ID.
  client_info *get_client_info (unsigned int player_id);

  //! \brief Returns the world info structure associated with the specified world ID.
  world_info *get_world_info (unsigned int world_id);

 private:
  void handle_runcmd (const std::string& cmd, const std::string& msg,
                      const client_info& cinfo);

  void handle_timeout ();


  //! \brief Creates a player object and inserts it into the global player table.
  void register_player (const client_info& cinfo);

  //! \brief Removes a player from the global player table.
  void unregister_player (unsigned int player_id);


  //! \brief Creates a world object and inserts it into the global world table.
  void register_world (const world_info& info);

  //! \brief Removes a world from the global world table.
  void unregister_world (unsigned int world_id);


  /*!
   * \brief Runs the Lua script at the specified path but does not handle yields!
   */
  void run_script_basic (const std::string& script_path);


  //! \brief Loads all command scripts from the specified directory.
  void load_commands (const std::string& path);

  /*!
   * \brief Runs the command script associated with the specified state structure.
   */
  void run_command (script_state& state, const std::string& cmd, const std::string& msg,
                    const client_info& cinfo);


  //! \brief Handles requests made by yields.
  void process_yield (script_state& state);

  //! \brief Handles a state after it has finished executing its script.
  void process_completion (script_state& state);

  //! \brief Handles errors thrown in resume_script.
  void process_error (script_state& state);

  //! \brief Resumes a script after the request of its yield has been answered to.
  void resume_script (script_state& state, int num_args);


  //! \brief Creates a new script state of the given type, possibly creating a new thread for it.
  script_state& create_state (script_type type, bool new_thread);

  //! \brief Removes the specified script state from the state list, and performs some cleanup.
  void delete_state (script_state& state);


  //
  // event trigger handlers:
  //

  void handle_player_chat_event_trigger (unsigned int client_id, int cont_id, const std::string& msg);
  void handle_player_changeblock_event_trigger (unsigned int client_id, int cont_id, block_pos pos, unsigned short block_id);

  /*!
   * \brief Creates a script state and a rudimentary event object for a player event invocation.
   * \param event_name The name of the event that is being triggered
   * \param client_id Client ID
   * \param cont_id Event handler continuation ID.
   * \return Returns a pointer to a script state if there is a handler available, otherwise null.
   */
  script_state *start_player_event_trigger (const char *event_name, unsigned int client_id, int cont_id);

  void run_event_trigger (script_state& state);
};

#endif //NOSTALGIA_SCRIPTING_HPP
