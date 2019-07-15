//
// Created by Jacob Zhitomirsky on 08-May-19.
//

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

  inline uint8_t read_unsigned_byte () { return (uint8_t)this->read_byte (); }
  inline uint16_t read_unsigned_short () { return (uint16_t)this->read_short (); }
  inline uint32_t read_unsigned_int () { return (uint32_t)this->read_int (); }
  inline uint64_t read_unsigned_long () { return (uint64_t)this->read_long (); }
};

#endif //NOSTALGIA_PACKET_READER_HPP
