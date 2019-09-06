//
// Created by Jacob Zhitomirsky on 06-Sep-19.
//

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
