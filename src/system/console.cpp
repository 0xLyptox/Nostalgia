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

#include "system/console.hpp"
#include "system/atoms.hpp"
#include "system/info.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <Windows.h>

static caf::actor global_server_actor;
static caf::actor_system *global_system = nullptr;
static bool can_terminate = false;


static void
_server_stop_actor (caf::blocking_actor *self)
{
  self->send (global_server_actor, stop_atom::value, self);
  self->receive (
      [] (stop_response_atom, typed_id id) { });

  std::cout << "Server stopped" << std::endl;
  can_terminate = true;
}

static void
_stop_server ()
{
  static bool server_stopping = false;
  if (server_stopping)
    return;
  server_stopping = true;

  std::cout << "Stopping server..." << std::endl;

  global_system->spawn (_server_stop_actor);

  while (!can_terminate)
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

  std::exit (0);
}

static BOOL WINAPI
_control_handler (DWORD ctrl_type)
{
  _stop_server ();
  return FALSE;
}

static void
_console_thread_function (const caf::actor& srv)
{
  global_server_actor = srv;

  for (;;)
    {
      char line[1024];
      std::cin.getline (line, sizeof line);
      if (!std::strcmp (line, "stop"))
        break;
    }

  _stop_server ();
}

void
start_console_thread (caf::actor_system& sys, const caf::actor& srv)
{
  global_system = &sys;

  if (!SetConsoleCtrlHandler (_control_handler, TRUE))
    {
      std::cout << "ERROR: Failed to setup control handler!" << std::endl;
    }

  std::thread th (_console_thread_function, srv);
  th.detach ();
}
