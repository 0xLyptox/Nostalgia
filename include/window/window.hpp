//
// Created by Jacob Zhitomirsky on 06-Sep-19.
//

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
