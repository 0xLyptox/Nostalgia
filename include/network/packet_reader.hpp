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

#ifndef NOSTALGIA_PACKET_READER_HPP
#define NOSTALGIA_PACKET_READER_HPP

#include <vector>
#include <cstdint>
#include <exception>
#include "util/position.hpp"


/*!
 * \class bad_data_error
 * \brief Thrown by packet_reader when trying to parse ill-formed data.
 */
class bad_data_error : public std::exception
{
  unsigned sz;
  unsigned pos;

 public:
  bad_data_error (unsigned sz, unsigned pos)
    : sz (sz), pos (pos)
  {}

  inline unsigned size () const { return this->sz; }
  inline unsigned position () const { return this->pos; }
};


class packet_reader
{
  const std::vector<char>& buf;
  const unsigned char *data;
  unsigned int pos;

 public:
  explicit packet_reader (const std::vector<char>& buf);

  inline unsigned int position () const { return this->pos; }
  inline unsigned int size () const { return (unsigned int)this->buf.size (); }

  bool read_bool ();
  int8_t read_byte ();
  int16_t read_short ();
  int32_t read_int ();
  int64_t read_long ();
  int64_t read_varlong ();
  float read_float ();
  double read_double ();
  std::string read_string (unsigned max_size = 0);
  block_pos read_position ();
  void read_bytes (void *out, unsigned int len);

  inline uint8_t read_unsigned_byte () { return (uint8_t)this->read_byte (); }
  inline uint16_t read_unsigned_short () { return (uint16_t)this->read_short (); }
  inline uint32_t read_unsigned_int () { return (uint32_t)this->read_int (); }
  inline uint64_t read_unsigned_long () { return (uint64_t)this->read_long (); }
};

#endif //NOSTALGIA_PACKET_READER_HPP
