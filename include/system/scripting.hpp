//
// Created by Jacob Zhitomirsky on 07-Sep-19.
//

#ifndef NOSTALGIA_SCRIPTING_HPP
#define NOSTALGIA_SCRIPTING_HPP

#include "system/atoms.hpp"
#include "util/position.hpp"
#include "system/info.hpp"
#include <stdexcept>
#include <list>
#include <caf/all.hpp>


// forward decs:
class scripting_actor;

enum class script_type
{
  command,
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
  caf::actor client;
  scripting_actor *self;
  int last_yield_code;
};

/*!
 * \brief This actor manages requests to run Lua scripts.
 */
class scripting_actor : public caf::blocking_actor
{
  void *vm; // lua_State*
  std::list<script_state> states;
  int next_state_id = 1;

 public:
  scripting_actor (caf::actor_config& cfg);
  ~scripting_actor ();

  void act () override;

  //! \brief Searches for the script state whose ID matches the specified argument.
  script_state *find_state (int sid);

 private:
  void handle_runcmd (const std::string& cmd, const std::string& msg,
                      const client_info& cinfo);

  void handle_timeout ();


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

  //! \brief Resumes a script after the request of its yield has been answered to.
  void resume_script (script_state& state, int num_args);


  //! \brief Creates a new script state of the given type, possibly creating a new thread for it.
  script_state& create_state (script_type type, bool new_thread);

  //! \brief Removes the specified script state from the state list, and performs some cleanup.
  void delete_state (script_state& state);
};

#endif //NOSTALGIA_SCRIPTING_HPP
