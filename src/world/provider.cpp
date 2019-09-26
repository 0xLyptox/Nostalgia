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

#include "world/provider.hpp"

// providers:
#include "world/providers/nw1/nw1.hpp"


std::unique_ptr<world_provider>
make_world_provider (const std::string& prov_name)
{
  if (prov_name == "nw1")
    return std::unique_ptr<world_provider> (new nw1_world_provider ());

  throw std::runtime_error ("Unknown world provider name");
}
