//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#ifndef NOSTALGIA_PACKET_WRITER_HPP
#define NOSTALGIA_PACKET_WRITER_HPP

#include "uuid.hpp"
#include <vector>
#include <cstdint>


class packet_writer
{
  std::vector<char> buf;
  unsigned int pos;

 public:
  packet_writer ();

  inline unsigned int position () const { return this->pos; }

  inline const char* data () const { return this->buf.data (); }
  inline std::vector<char>&& move_data () { return std::move (buf); }

  void write_bool (bool val);
  void write_byte (int8_t val);
  void write_short (int16_t val);
  void write_int (int32_t val);
  void write_long (int64_t val);
  void write_varlong (int64_t val);
  void write_bytes (const void *data, unsigned int len);
  void write_string (const std::string& str);
  void write_uuid_string (const uuid_t& uuid);
};

#endif //NOSTALGIA_PACKET_WRITER_HPP
