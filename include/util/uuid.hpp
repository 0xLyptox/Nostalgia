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

#ifndef NOSTALGIA_UUID_HPP
#define NOSTALGIA_UUID_HPP

#include <caf/all.hpp>
#include <cstdint>
#include <random>
#include <cstring>
#include <sstream>
#include <iomanip>


struct uuid_t
{
  uint8_t data[16];

  bool
  operator== (const uuid_t& other) const
  { return std::memcmp (this->data, other.data, 16) == 0; }

  bool
  operator!= (const uuid_t& other) const
  { return !this->operator== (other);}


  template<typename R>
  static uuid_t
  random (R& rnd)
  {
    std::uniform_int_distribution<> dis (0x00, 0xFF);
    uuid_t res;
    for (int i = 0; i < 16; ++i)
      res.data[i] = (uint8_t)dis (rnd);
    return res;
  }

  std::string
  str () const
  {
    std::ostringstream ss;
    ss << std::hex << std::setfill ('0');
    for (int i = 0; i < 4; ++i)
      ss << std::setw (2) << (int)this->data[i];
    for (int i = 0; i < 3; ++i)
      {
        ss << '-';
        for (int j = 0; j < 2; ++j)
          ss << std::setw (2) << (int)this->data[4 + i * 2 + j];
      }
    ss << '-';
    for (int i = 0; i < 6; ++i)
      ss << std::setw (2) << (int)this->data[10 + i];
    return ss.str ();
  }
};


template<typename Inspector>
typename Inspector::result_type
inspect (Inspector& f, uuid_t& uuid)
{
  return f (caf::meta::type_name ("uuid_t"), uuid.data);
}

#endif //NOSTALGIA_UUID_HPP
