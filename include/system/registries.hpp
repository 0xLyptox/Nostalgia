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

#ifndef NOSTALGIA_REGISTRIES_HPP
#define NOSTALGIA_REGISTRIES_HPP

#include <string>
#include <unordered_map>

//
// Standard registry names:
//
#define ITEM_REGISTRY "minecraft:item"


class registry_not_found : public std::exception {};
class entry_not_found : public std::exception {};

/*!
 * \brief A mapping between names and network IDs.
 */
struct registry
{
  int id;
  std::string default_entry;
  std::unordered_map<std::string, int> name_map;
  std::unordered_map<int, std::string> id_map;

  /*!
   * \brief Returns the network ID mapped to the specified name.
   * \throws entry_not_found if the name is not mapped to anything.
   */
  int operator[] (const std::string& name) const;

  /*!
   * \brief Returns the namespaced ID mapped to the specified network ID.
   * \throws entry_not_found if the name is not mapped to anything.
   */
  const std::string& operator[] (int protocol_id) const;
};


/*!
 * \brief Manages all registries structures.
 */
class registries
{
  static std::unordered_map<std::string, registry> entries;

 public:
  /*!
   * \brief Loads the registries from disk (must be called at server start).
   */
  static void initialize ();

  //! \brief Returns the registry that has the specified name.
  static registry& get (const std::string& name);

  //! \brief Shorthand for registries::get(registry_name)[entry_name]
  static int id (const std::string& registry_name, const std::string& entry_name);

  //! \brief Shorthand for registries::get(registry_name)[id]
  static const std::string& name (const std::string& registry_name, int id);
};

#endif //NOSTALGIA_REGISTRIES_HPP
