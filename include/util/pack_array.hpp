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

#ifndef NOSTALGIA_PACK_ARRAY_HPP
#define NOSTALGIA_PACK_ARRAY_HPP

#include <cstddef>


/*!
 * Packs the elements into the specified output array in such a way that in
 * the output array every element takes \p bits_per_element bits of space.
 *
 * \tparam T The element type of the input array.
 * \tparam U The element type of the output array.
 * \param elements The array to pack.
 * \param bits_per_element Number bits of space each element will take in the output array.
 * \param out The array to save the results into.
 */
template<typename T, typename U>
void pack_array (const T *in, size_t in_len, U *out, unsigned int bits_per_element)
{
  constexpr auto output_cell_size = sizeof (U) * 8;

  U curr = 0;
  unsigned curr_offset = 0;
  unsigned int len = 0;
  for (size_t i = 0; i < in_len; ++i)
    {
      int val = in[i];

      // determine how many bits to occupy in current output element
      unsigned take = bits_per_element;
      if (output_cell_size - curr_offset < take)
        take = output_cell_size - curr_offset;

      auto head = val >> (bits_per_element - take);
      auto tail = val & ((1 << (bits_per_element - take)) - 1);

      curr <<= take;
      curr |= head;
      curr_offset += take;
      if (curr_offset == output_cell_size)
        {
          // filled an output cell
          out[len++] = curr;

          // move remainder to next cell if there is one
          curr_offset = bits_per_element - take;
          curr = tail;
        }
    }
}

#endif //NOSTALGIA_PACK_ARRAY_HPP
