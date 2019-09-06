//
// Created by Jacob Zhitomirsky on 06-Sep-19.
//

#ifndef NOSTALGIA_SLOT_HPP
#define NOSTALGIA_SLOT_HPP


/*!
 * \class slot
 * \brief Represents a window slot which stores an item and its associated data.
 */
class slot
{
  int item_id;
  unsigned char item_count;

 public:
  [[nodiscard]] inline auto id () const { return this->item_id; }
  [[nodiscard]] inline auto count () const { return this->item_count; }

  inline void set_id (int val) { this->item_id = val; }
  inline void set_count (unsigned char count) { this->item_count = count; }

  explicit slot (int item_id, unsigned char item_count = 1)
    : item_id (item_id), item_count (item_count)
  { }
};

#endif //NOSTALGIA_SLOT_HPP
