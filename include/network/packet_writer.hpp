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

#ifndef NOSTALGIA_PACKET_WRITER_HPP
#define NOSTALGIA_PACKET_WRITER_HPP

#include "util/uuid.hpp"
#include "util/position.hpp"
#include <vector>
#include <cstdint>


// forward decs:
class nbt_writer;

class packet_writer
{
  std::vector<char> buf;
  unsigned int pos = 0;

 public:
  inline unsigned int position () const { return this->pos; }

  inline const char* data () const { return this->buf.data (); }
  inline std::vector<char>&& move_data () { return std::move (buf); }

  void write_bool (bool val);
  void write_byte (uint8_t val);
  void write_short (uint16_t val);
  void write_int (uint32_t val);
  void write_long (uint64_t val);
  void write_varlong (uint64_t val);
  void write_float (float val);
  void write_double (double val);
  void write_bytes (const void *data, unsigned int len);
  void write_string (const std::string& str);
  void write_uuid_string (const uuid_t& uuid);
  void write_position (block_pos pos);
  void write_nbt (const nbt_writer& writer);
};

//! \brief Returns the size in bytes of a specified varlong.
unsigned int varlong_size (uint64_t val);

#endif //NOSTALGIA_PACKET_WRITER_HPP
