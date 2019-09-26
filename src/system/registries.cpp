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

#include "system/registries.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream> // DEBUG

#define REGISTRIES_PATH "data/registries.json"


int
registry::operator[] (const std::string& name) const
{
  auto itr = this->name_map.find (name);
  if (itr == this->name_map.end ())
    throw entry_not_found {};
  return itr->second;
}

const std::string&
registry::operator[] (int protocol_id) const
{
  auto itr = this->id_map.find (protocol_id);
  if (itr == this->id_map.end ())
    throw entry_not_found {};
  return itr->second;
}


//------------------------------------------------------------------------------

//
// Static variables:
//

std::unordered_map<std::string, registry> registries::entries;



void
registries::initialize ()
{
  std::ifstream fs (REGISTRIES_PATH);
  if (!fs)
    throw std::runtime_error ("Failed to open registries file");

  auto j = nlohmann::json::parse (fs);
  for (auto itr = j.begin (); itr != j.end (); ++itr)
    {
      auto& reg = registries::entries[itr.key ()];
      auto& reg_obj = itr.value ();

      reg.id = reg_obj["protocol_id"].get<int> ();
      if (reg_obj.find ("default") != reg_obj.end ())
        reg.default_entry = reg_obj["default"].get<std::string> ();

      auto& entries_obj = reg_obj["entries"];
      for (auto itr2 = entries_obj.begin (); itr2 != entries_obj.end (); ++itr2)
        {
          int protocol_id = itr2.value ()["protocol_id"].get<int> ();
          reg.id_map[protocol_id] = itr2.key ();
          reg.name_map[itr2.key ()] = protocol_id;
        }
    }
}


registry&
registries::get (const std::string& name)
{
  auto itr = registries::entries.find (name);
  if (itr == registries::entries.end ())
    throw registry_not_found {};
  return itr->second;
}

int
registries::id (const std::string& registry_name, const std::string& entry_name)
{
  return registries::get (registry_name)[entry_name];
}

const std::string&
registries::name (const std::string& registry_name, int id)
{
  return registries::get (registry_name)[id];
}
