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

#include "window/window.hpp"

//
// Standard window specifications:
//

window_spec window_spec::player_inventory {
  46,
  {
      { "output", 0 },
      { "input", { 1, 5 } },
      { "armor", { 5, 9 } },
      { "main", { 9, 36 } },
      { "hotbar", { 36, 45 } },
      { "offhand", 45 },

      { "head", 5 },
      { "chest", 6 },
      { "legs", 7 },
      { "feet", 8 }
  }
};

//------------------------------------------------------------------------------

window::window (const window_spec& spec)
  : spec (spec), slots (spec.num_slots)
{
}


const window_range&
window::range (const std::string& name) const
{
  auto itr = this->spec.range_map.find (name);
  if (itr == this->spec.range_map.end ())
    throw window_range_not_found {};
  return itr->second;
}

void
window::set_slot (int idx, std::unique_ptr<slot>&& item)
{
  if (idx < 0 || idx >= (int)this->slots.size ())
    return;
  this->slots[idx] = std::move (item);
}

void
window::clear_slot (int idx)
{
  if (idx < 0 || idx >= (int)this->slots.size ())
    return;
  this->slots[idx].reset ();
}

slot*
window::get_slot (int idx)
{
  if (idx < 0 || idx >= (int)this->slots.size ())
    return nullptr;
  return this->slots[idx].get ();
}

slot*
window::get_slot (const std::string& range_name, int offset)
{
  auto& rng = this->range (range_name);
  int idx = rng.start + offset;
  if (idx < 0 || idx >= (int)this->slots.size () || idx >= rng.end)
    return nullptr;
  return this->slots[idx].get ();
}
