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

#ifndef NOSTALGIA_WINDOW_HPP
#define NOSTALGIA_WINDOW_HPP

#include "window/slot.hpp"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

struct window_range
{
  int start, end; // end is exclusive!

  window_range (unsigned int start, unsigned int end = -1)
    : start (start), end (end < 0 ? start + 1 : end)
  {}

  [[nodiscard]] inline bool contains (int idx) const { return start <= idx && idx < end; }
};

using window_range_map = std::unordered_map<std::string, window_range>;

struct window_spec
{
  unsigned int num_slots;
  window_range_map range_map;

  //
  // Standard window specifications:
  //
  static window_spec player_inventory;
};


class window_range_not_found : public std::exception {};

/*!
 * \class window
 * \brief Represents a slot window.
 */
class window
{
  std::vector<std::unique_ptr<slot>> slots;
  const window_spec& spec;

 public:
  //! \brief Constructs a window from the specified specification structure.
  window (const window_spec& spec);

  //! \brief Returns the window range corresponding to the specified name.
  const window_range& range (const std::string& name) const;

  //! \brief Sets the slot at the specified index to the given item.
  void set_slot (int idx, std::unique_ptr<slot>&& item);

  //! \brief Empties the slot at the specified index.
  void clear_slot (int idx);

  //! \brief Returns the slot at the specified index, or null if the slot is empty.
  slot *get_slot (int idx);
  slot *get_slot (const std::string& range_name, int offset = 0);
};

#endif //NOSTALGIA_WINDOW_HPP
