//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#ifndef NOSTALGIA_INFO_HPP
#define NOSTALGIA_INFO_HPP

#include "uuid.hpp"
#include <string>
#include <caf/all.hpp>


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
  caf::actor actor;
  std::string name;
};

template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, world_info& info)
{
  return f (caf::meta::type_name ("world_info"), info.actor, info.name);
}

#endif //NOSTALGIA_INFO_HPP
