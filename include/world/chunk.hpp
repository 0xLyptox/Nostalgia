//
// Created by Jacob Zhitomirsky on 10-May-19.
//

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

  chunk_palette generate_palette () const;

  //! \brief Returns the number of non air blocks present in the section.
  int count_non_air_blocks () const;

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

 public:
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
