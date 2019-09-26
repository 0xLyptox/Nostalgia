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

#ifndef NOSTALGIA_WORLD_PROVIDERS_NW1_COMPRESS_HPP
#define NOSTALGIA_WORLD_PROVIDERS_NW1_COMPRESS_HPP

#include <string>
#include <iostream> // DEBUG


namespace nw1 {

  template<typename T>
  void
  write_varint (std::string& out, T x)
  {
    if (x == 0)
      out.push_back (0);

    while (x != 0)
      {
        auto part = (unsigned char)(x & 0x7FU);
        x >>= 7U;
        if (x != 0)
          part |= 0x80U;
        out.push_back (part);
      }
  }

  template<>
  void
  write_varint<unsigned char> (std::string& out, unsigned char x)
  {
    out.push_back (x);
  }

  template<>
  void
  write_varint<char> (std::string& out, char x)
  {
    out.push_back (x);
  }

  /*!
   * \brief Very simple run-length encoding compression of an array of elements into a bytstream.
   * \tparam T The array element type.
   * \param out The string to append the compression result to.
   * \param data The array of elements to compress
   * \param count The size of the input array.
   */
  template<typename T>
  void
  compress_array (const T *data, size_t count, std::string& out)
  {
    unsigned int run_length = 0;
    T ref = 0;
    for (size_t i = 0; i < count; ++i)
      {
        if (run_length == 0 || data[i] != ref)
          {
            if (run_length > 0)
              {
                write_varint (out, run_length);
                write_varint (out, ref);
              }

            ref = data[i];
            run_length = 1;
          }
        else
          ++ run_length;
      }

    if (run_length > 0)
      {
        write_varint (out, run_length);
        write_varint (out, ref);
      }
  }


  template<typename T>
  int
  read_varint (const unsigned char *data, T& result)
  {
    result = 0;

    auto ptr = data;
    if (*ptr == 0)
      return 1;

    size_t width = 0;
    do
      {
        result |= (*ptr & 0x7F) << width;
        width += 7;
      }
    while ((*ptr++ & 0x80) != 0);

    return (int)(ptr - data);
  }

  template<>
  int
  read_varint<unsigned char> (const unsigned char *data, unsigned char& result)
  {
    result = *data;
    return 1;
  }

  template<>
  int
  read_varint<char> (const unsigned char *data, char& result)
  {
    result = (char)*data;
    return 1;
  }

  /*!
   * \brief Decompresses an array compressed by compress_array until the output array is full.
   * \tparam T The original array element type.
   * \param data The compressed byte stream to decompress.
   * \param out The array of elements to fill with the decompressed result.
   * \param out_len Size of the output array.
   * \return Amount of input bytes processed.
   */
  template<typename T>
  int
  decompress_array (const unsigned char *data, T *out, size_t out_len)
  {
    auto ptr = data;
    size_t out_pos = 0;
    unsigned int run_length;
    T val;

    while (out_pos < out_len)
      {
        ptr += read_varint<unsigned int> (ptr, run_length);
        ptr += read_varint<T> (ptr, val);

        for (unsigned int i = 0; i < run_length; ++i)
          out[out_pos + i] = val;
        out_pos += run_length;
      }

    return (int)(ptr - data);
  }
}

#endif //NOSTALGIA_WORLD_PROVIDERS_NW1_COMPRESS_HPP
