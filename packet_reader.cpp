//
// Created by Jacob Zhitomirsky on 08-May-19.
//

#include "packet_reader.hpp"

#define MAX_STRING_SIZE 32767
#define THROW_BAD_DATA \
  throw bad_data_error (this->size (), this->pos)


packet_reader::packet_reader (const std::vector<char>& buf)
  : buf (buf), pos (0)
{
  // nop
}



int8_t
packet_reader::read_byte ()
{
  if (this->pos >= this->buf.size ())
    THROW_BAD_DATA;
  return this->buf[this->pos ++];
}

int16_t
packet_reader::read_short ()
{
  if (this->pos + 2 > this->buf.size ())
    THROW_BAD_DATA;
  this->pos += 2;
  return ((int16_t)this->buf[this->pos - 2] << 8) | (int16_t)this->buf[this->pos - 1];
}

int32_t
packet_reader::read_int ()
{
  if (this->pos + 4 > this->buf.size ())
    THROW_BAD_DATA;
  this->pos += 4;
  return ((int32_t)this->buf[this->pos - 4] << 24)
         | ((int32_t)this->buf[this->pos - 3] << 16)
         | ((int32_t)this->buf[this->pos - 2] << 8)
         | (int32_t)this->buf[this->pos - 1];
}

int64_t
packet_reader::read_long ()
{
  if (this->pos + 8 > this->buf.size ())
    THROW_BAD_DATA;
  this->pos += 8;
  return ((int64_t)this->buf[this->pos - 8] << 56)
         | ((int64_t)this->buf[this->pos - 7] << 48)
         | ((int64_t)this->buf[this->pos - 6] << 40)
         | ((int64_t)this->buf[this->pos - 5] << 32)
         | ((int64_t)this->buf[this->pos - 4] << 24)
         | ((int64_t)this->buf[this->pos - 3] << 16)
         | ((int64_t)this->buf[this->pos - 2] << 8)
         | (int64_t)this->buf[this->pos - 1];
}

int64_t
packet_reader::read_varlong ()
{
  int64_t num = 0;
  unsigned num_bytes = 0;
  while (num_bytes < 10)
    {
      auto byte = this->read_byte ();
      num |= ((byte & 0x7F) << (7 * num_bytes));
      ++ num_bytes;
      if (!(byte & 0x80))
        return num; // done
    }

  throw std::exception ("packet_reader: varlong longer than 10 bytes");
}

std::string
packet_reader::read_string (unsigned max_size)
{
  auto str_size = this->read_varlong ();
  if (str_size > MAX_STRING_SIZE)
    THROW_BAD_DATA;
  if (max_size != 0 && str_size > max_size)
    THROW_BAD_DATA;
  if (this->pos + str_size > this->size ())
    THROW_BAD_DATA;

  std::string str (this->buf.data () + this->pos, str_size);
  str.push_back (0);
  this->pos += (unsigned)str_size;

  return str;
}

