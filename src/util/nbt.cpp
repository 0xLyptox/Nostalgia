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

#include "util/nbt.hpp"
#include <cstring>
#include <iostream>


void
nbt_writer::push_byte (uint8_t val, const char *name)
{
  this->_emit_tag_header (TAG_BYTE, name);
  this->buf.push_back (val);
}

void
nbt_writer::push_short (uint16_t val, const char *name)
{
  this->_emit_tag_header (TAG_SHORT, name);
  this->_emit_short (val);
}

void
nbt_writer::push_int (uint32_t val, const char *name)
{
  this->_emit_tag_header (TAG_INT, name);
  this->_emit_int (val);
}

void
nbt_writer::push_long (uint64_t val, const char *name)
{
  this->_emit_tag_header (TAG_LONG, name);
  this->_emit_long (val);
}

void
nbt_writer::push_float (float val, const char *name)
{
  this->_emit_tag_header (TAG_FLOAT, name);
  this->_emit_float (val);
}

void
nbt_writer::push_double (double val, const char *name)
{
  this->_emit_tag_header (TAG_DOUBLE, name);
  this->_emit_double (val);
}

void
nbt_writer::_push_array_type (nbt_tag tag, const void *data, unsigned int len, const char *name, int unsigned len_multiplier)
{
  this->_emit_tag_header (tag, name);
  this->_emit_int (len);

  auto effective_len = len * len_multiplier;
  auto index = this->buf.size ();
  this->buf.resize (index + effective_len);
  std::memcpy (this->buf.data () + index, data, effective_len);
}

void
nbt_writer::push_byte_array (const void *data, unsigned int len, const char *name)
{
  this->_push_array_type (TAG_BYTE_ARRAY, data, len, name, 1);
}

void
nbt_writer::push_int_array (const void *data, unsigned int len, const char *name)
{
  this->_push_array_type (TAG_INT_ARRAY, data, len, name, 4);
}

void
nbt_writer::push_long_array (const void *data, unsigned int len, const char *name)
{
  this->_push_array_type (TAG_LONG_ARRAY, data, len, name, 8);
}

void
nbt_writer::push_string (const char *str, const char *name)
{
  this->_emit_tag_header (TAG_STRING, name);
  this->_emit_string (str);
}

void
nbt_writer::push_string (const std::string& str, const char *name)
{
  this->_emit_tag_header (TAG_STRING, name);
  this->_emit_short ((uint16_t)str.length ());
  for (char c : str)
    this->buf.push_back (c);
}

void
nbt_writer::start_compound (const char *name)
{
  this->_emit_tag_header (TAG_COMPOUND, name);
  this->hierarchy.push (TAG_COMPOUND);
}

void
nbt_writer::end_compound ()
{
  this->buf.push_back (TAG_END);
}

void
nbt_writer::start_list (nbt_tag element_type, const char *name)
{
  this->_emit_tag_header (TAG_LIST, name);
  this->buf.push_back (element_type);

  this->hierarchy.push (TAG_LIST);
  this->list_size_stack.push (0);
  this->list_size_offset_stack.push ((unsigned int)this->buf.size ()); // remember offset of list size for later
  this->_emit_int (0); // placeholder for size
}

void
nbt_writer::end_list ()
{
  this->hierarchy.pop ();

  auto list_size = this->list_size_stack.top ();
  this->list_size_stack.pop ();

  auto size_offset = this->list_size_offset_stack.top ();
  this->list_size_offset_stack.pop ();

  // update list size field
  this->buf[size_offset + 0] = (unsigned char)(list_size >> 24U);
  this->buf[size_offset + 1] = (unsigned char)((list_size >> 16U) & 0xFFU);
  this->buf[size_offset + 2] = (unsigned char)((list_size >> 8U) & 0xFFU);
  this->buf[size_offset + 3] = (unsigned char)(list_size & 0xFFU);
}



void
nbt_writer::_emit_tag_header (nbt_tag type, const char *name)
{
  if (this->hierarchy.empty () || this->hierarchy.top () != TAG_LIST)
    {
      this->buf.push_back (type);
      this->_emit_string (name);
    }
  else
    {
      // we are inside a list, so don't emit tag id and name.
    }

  this->_push_list_element ();
}


void
nbt_writer::_emit_short (unsigned short val)
{
  this->buf.push_back ((unsigned char)(val >> 8U));
  this->buf.push_back ((unsigned char)(val & 0xFFU));
}

void
nbt_writer::_emit_int (uint32_t val)
{
  this->buf.push_back ((unsigned char)(val >> 24U));
  this->buf.push_back ((unsigned char)((val >> 16U) & 0xFFU));
  this->buf.push_back ((unsigned char)((val >> 8U) & 0xFFU));
  this->buf.push_back ((unsigned char)(val & 0xFFU));
}

void
nbt_writer::_emit_long (uint64_t val)
{
  this->buf.push_back ((unsigned char)(val >> 56U));
  this->buf.push_back ((unsigned char)((val >> 48U) & 0xFFU));
  this->buf.push_back ((unsigned char)((val >> 40U) & 0xFFU));
  this->buf.push_back ((unsigned char)((val >> 32U) & 0xFFU));
  this->buf.push_back ((unsigned char)((val >> 24U) & 0xFFU));
  this->buf.push_back ((unsigned char)((val >> 16U) & 0xFFU));
  this->buf.push_back ((unsigned char)((val >> 8U) & 0xFFU));
  this->buf.push_back ((unsigned char)(val & 0xFFU));
}

void
nbt_writer::_emit_float (float val)
{
  this->_emit_int (*reinterpret_cast<uint32_t *> (&val));
}

void
nbt_writer::_emit_double (double val)
{
  this->_emit_long (*reinterpret_cast<uint64_t *> (&val));
}

void
nbt_writer::_emit_string (const char *str)
{
  if (!str)
    return;

  this->_emit_short ((uint16_t)std::strlen (str));
  while (*str)
    this->buf.push_back (*str++);
}


void
nbt_writer::_push_list_element ()
{
  // called from push methods.
  // updates the list statistics if we are a direct descendant of a list.
  if (!this->hierarchy.empty ())
    {
      auto parent = this->hierarchy.top ();
      if (parent == TAG_LIST)
        {
          // we are in a list, increase its size by one.
          auto list_size = this->list_size_stack.top ();
          this->list_size_stack.pop ();
          this->list_size_stack.push (list_size + 1);
        }
    }
}
