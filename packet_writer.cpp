//
// Created by Jacob Zhitomirsky on 09-May-19.
//

#include "packet_writer.hpp"
#include <iomanip>


packet_writer::packet_writer ()
  : pos (0)
{
}


void
packet_writer::write_bool (bool val)
{
  this->write_byte ((int8_t)val);
}

void
packet_writer::write_byte (int8_t val)
{
  if (this->pos == this->buf.size ())
    {
      this->buf.push_back (val);
      ++ this->pos;
    }
  else
    this->buf[this->pos ++] = val;
}

void
packet_writer::write_short (int16_t val)
{
  this->write_byte ((int8_t)(val >> 8));
  this->write_byte ((int8_t)(val & 0xFF));
}

void
packet_writer::write_int (int32_t val)
{
  this->write_byte ((int8_t)(val >> 24));
  this->write_byte ((int8_t)((val >> 16) & 0xFF));
  this->write_byte ((int8_t)((val >> 8) & 0xFF));
  this->write_byte ((int8_t)(val & 0xFF));
}

void
packet_writer::write_long (int64_t val)
{
  this->write_byte ((int8_t)(val >> 56));
  this->write_byte ((int8_t)((val >> 48) & 0xFF));
  this->write_byte ((int8_t)((val >> 40) & 0xFF));
  this->write_byte ((int8_t)((val >> 32) & 0xFF));
  this->write_byte ((int8_t)((val >> 24) & 0xFF));
  this->write_byte ((int8_t)((val >> 16) & 0xFF));
  this->write_byte ((int8_t)((val >> 8) & 0xFF));
  this->write_byte ((int8_t)(val & 0xFF));
}

void
packet_writer::write_varlong (int64_t val)
{
  while (val > 0x7F)
    {
      this->write_byte ((int8_t)(0x80 | (val & 0x7F)));
      val >>= 7;
    }

  this->write_byte ((int8_t)val);
}

void
packet_writer::write_bytes (const void *data, unsigned int len)
{
  if (this->pos + len > this->buf.size ())
    this->buf.resize (this->pos + len);
  std::memcpy (this->buf.data () + this->pos, data, len);
  this->pos += len;
}

void
packet_writer::write_string (const std::string& str)
{
  this->write_varlong (str.size ());
  this->write_bytes (str.c_str (), (unsigned)str.size ());
}

void
packet_writer::write_uuid_string (const uuid_t& uuid)
{
  this->write_string (uuid.str ());
}