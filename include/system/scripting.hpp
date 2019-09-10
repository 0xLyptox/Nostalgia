//
// Created by Jacob Zhitomirsky on 07-Sep-19.
//

#ifndef NOSTALGIA_SCRIPTING_HPP
#define NOSTALGIA_SCRIPTING_HPP

#include "system/atoms.hpp"
#include "util/position.hpp"
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
                      const caf::actor& client);

  void handle_timeout ();


  /*!
   * \brief Loads a command script.
   * The script is loaded and run (but the docommand function is not invoked).
   * \param cmd_name The name of the command to load.
   * \throw command_not_found if a command script does not exist.
   * \return The newly created script state.
   */
  void load_command (const std::string& cmd_path);

  //! \brief Loads all command scripts from the specified directory.
  void load_commands (const std::string& path);

  /*!
   * \brief Runs the command script associated with the specified state structure.
   */
  void run_command (script_state& state);


  //! \brief Handles requests made by yields.
  void process_yield (script_state& state);

  //! \brief Resumes a script after the request of its yield has been answered to.
  void resume_script (script_state& state, int num_args);


  //! \brief Removes the specified script state from the state list, and performs some cleanup.
  void delete_state (script_state& state);
};

#endif //NOSTALGIA_SCRIPTING_HPP
