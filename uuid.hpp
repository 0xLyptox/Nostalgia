//
// Created by Jacob Zhitomirsky on 09-May-19.
//

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
