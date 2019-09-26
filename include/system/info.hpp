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

#ifndef NOSTALGIA_INFO_HPP
#define NOSTALGIA_INFO_HPP

#include "util/uuid.hpp"
#include <string>
#include <caf/all.hpp>


enum class actor_type
{
  server,
  client,
  world,
};

struct typed_id
{
  actor_type type;
  unsigned int id;
};

template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, typed_id& id)
{
  return f (caf::meta::type_name ("typed_id"), id.type, id.id);
}


/*!
 * \brief Contains long-term information about a connected client.
 *
 * A global list of these structures is maintained by a server actor.
 * Each client actor stores its own info and keeps the server up to date with
 * any changes.
 */
struct client_info
{
  unsigned int id;
  caf::actor actor;
  uuid_t uuid;
  std::string username;
};


template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, client_info& info)
{
  return f (caf::meta::type_name ("client_info"), info.id, info.actor, info.uuid, info.username);
}


/*!
 * \brief World information.
 *
 * List maintained by server instance.
 */
struct world_info
{
  unsigned int id;
  caf::actor actor;
  std::string name;
};

template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, world_info& info)
{
  return f (caf::meta::type_name ("world_info"), info.id, info.actor, info.name);
}

#endif //NOSTALGIA_INFO_HPP
