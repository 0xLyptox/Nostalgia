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

#ifndef NOSTALGIA_CHUNK_HPP
#define NOSTALGIA_CHUNK_HPP

#include <vector>
#include <map>
#include <memory>
#include "network/packet_writer.hpp"


struct chunk_palette
{
  std::vector<unsigned short> ids;
  std::map<unsigned short, unsigned short> index_map;

  unsigned int num_blocks () const { return (unsigned)ids.size (); }
  unsigned int compute_bits_per_block () const;
};

struct chunk_section
{
  unsigned short ids[4096];
  unsigned char block_light[2048];
  unsigned char sky_light[2048];

  chunk_section ();

  [[nodiscard]] chunk_palette generate_palette () const;

  //! \brief Returns the number of non air blocks present in the section.
  [[nodiscard]] int count_non_air_blocks () const;

  template<typename Inspector>
  friend typename Inspector::result_type
  inspect (Inspector& f, chunk_section& cs)
  {
    return f (caf::meta::type_name ("chunk_section"), cs.ids, cs.block_light, cs.sky_light);
  }
};

class chunk
{
  int x, z;
  int biomes[256] = { 0 };
  std::vector<chunk_section> sections;
  unsigned int section_bitmap = 0;
  bool dirty = true; // tracks whether changes have been made to this chunk

 public:
  [[nodiscard]] inline int get_x () const { return this->x; }
  [[nodiscard]] inline int get_z () const { return this->z; }
  [[nodiscard]] inline bool has_section (unsigned idx) const { return this->section_bitmap & (1U << idx); }
  [[nodiscard]] inline const auto& get_section (unsigned idx) const { return this->sections[idx]; }
  [[nodiscard]] inline auto& get_section (unsigned idx) { return this->sections[idx]; }
  [[nodiscard]] inline auto get_section_bitmap () const { return this->section_bitmap; }
  inline void add_section (unsigned idx) { this->section_bitmap |= (1U << idx); }

  [[nodiscard]] inline bool is_dirty () const { return this->dirty; }
  inline void mark_dirty (bool value = true) { this->dirty = value; }

  chunk (int x, int z);

  void set_block_id_unsafe (int x, int y, int z, unsigned short id);
  void set_block_id (int x, int y, int z, unsigned short id);
  unsigned short get_block_id_unsafe (int x, int y, int z);
  unsigned short get_block_id (int x, int y, int z);

  void set_sky_light_unsafe (int x, int y, int z, unsigned char val);
  void set_sky_light (int x, int y, int z, unsigned char val);
  unsigned char get_sky_light_unsafe (int x, int y, int z);
  unsigned char get_sky_light (int x, int y, int z);

  void set_block_light_unsafe (int x, int y, int z, unsigned char val);
  void set_block_light (int x, int y, int z, unsigned char val);

  //! \brief Computes sky/block lighting for the blocks in chunk.
  void compute_initial_lighting ();

  //! \brief Fills the specified height map array with the correct values.
  void compute_height_map (int height_map[256]);

  /*!
   * \brief Creates a CHUNK DATA packet to send to a client.
   */
  packet_writer make_chunk_data_packet ();

  template<typename Inspector>
  friend typename Inspector::result_type
  inspect (Inspector& f, chunk& ch)
  {
    return f (caf::meta::type_name ("chunk"), ch.x, ch.z, ch.sections, ch.biomes);
  }
};

#endif //NOSTALGIA_CHUNK_HPP
