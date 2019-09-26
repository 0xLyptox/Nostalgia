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

#ifndef NOSTALGIA_NBT_HPP
#define NOSTALGIA_NBT_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <stack>


enum nbt_tag
{
  TAG_END        = 0,
  TAG_BYTE       = 1,
  TAG_SHORT      = 2,
  TAG_INT        = 3,
  TAG_LONG       = 4,
  TAG_FLOAT      = 5,
  TAG_DOUBLE     = 6,
  TAG_BYTE_ARRAY = 7,
  TAG_STRING     = 8,
  TAG_LIST       = 9,
  TAG_COMPOUND   = 10,
  TAG_INT_ARRAY  = 11,
  TAG_LONG_ARRAY = 12,
};

/*!
 * \class nbt_writer
 * \brief Manages an NBT-formatted buffer with an interface that allows updating it.
 */
class nbt_writer
{
  std::vector<unsigned char> buf;

  std::stack<nbt_tag> hierarchy;
  std::stack<unsigned int> list_size_stack;
  std::stack<unsigned int> list_size_offset_stack;

 public:
  [[nodiscard]] const auto& buffer () const { return this->buf; }
  [[nodiscard]] auto size () const { return this->buf.size (); }

  void push_byte (uint8_t val, const char *name = nullptr);
  void push_short (uint16_t val, const char *name = nullptr);
  void push_int (uint32_t val, const char *name = nullptr);
  void push_long (uint64_t val, const char *name = nullptr);
  void push_float (float val, const char *name = nullptr);
  void push_double (double val, const char *name = nullptr);
  void push_string (const char *str, const char *name = nullptr);
  void push_string (const std::string& str, const char *name = nullptr);
  void push_byte_array (const void *data, unsigned int len, const char *name = nullptr);
  void push_int_array (const void *data, unsigned int len, const char *name = nullptr);
  void push_long_array (const void *data, unsigned int len, const char *name = nullptr);

  void start_compound (const char *name = nullptr);
  void end_compound ();

  void start_list (nbt_tag element_type, const char *name = nullptr);
  void end_list ();

 private:
  void _emit_tag_header (nbt_tag type, const char *name);

  void _emit_short (uint16_t val);
  void _emit_int (uint32_t val);
  void _emit_long (uint64_t val);
  void _emit_float (float val);
  void _emit_double (double val);
  void _emit_string (const char *str);

  void _push_array_type (nbt_tag tag, const void *data, unsigned int len, const char *name, int unsigned len_multiplier);

  void _push_list_element ();
};

#endif //NOSTALGIA_NBT_HPP
