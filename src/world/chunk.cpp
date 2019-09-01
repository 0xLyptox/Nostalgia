//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#include "world/chunk.hpp"
#include "system/consts.hpp"
#include "world/blocks.hpp"
#include "util/nbt.hpp"
#include "util/pack_array.hpp"
#include <set>


chunk_section::chunk_section ()
{
  std::memset (this->ids, 0, sizeof this->ids);
  std::memset (this->block_light, 0, sizeof this->block_light);
  std::memset (this->sky_light, 0, sizeof this->sky_light);
}


static unsigned int
_ilog2 (int n)
{
  unsigned int r = 0;
  while (n > 1)
    {
      n >>= 1;
      ++ r;
    }
  return r;
}

unsigned int
chunk_palette::compute_bits_per_block () const
{
  auto num = this->num_blocks ();
  if (num == 0)
    return 4;

  auto res = _ilog2 (num);
  if ((num & (num - 1)) != 0)
    ++ res; // num_blocks is not a perfect power of two

  if (res < 4)
    res = 4; // there must be at least 4 bits per block

  return res;
}

chunk_palette
chunk_section::generate_palette () const
{
  chunk_palette palette;

  for (unsigned short id : this->ids)
  {
    if (palette.index_map.find (id) == palette.index_map.end ())
      {
        palette.index_map[id] = (unsigned short)palette.ids.size ();
        palette.ids.push_back (id);
      }
  }

  return palette;
}

//! \brief Returns the number of non air blocks present in the section.
int
chunk_section::count_non_air_blocks () const
{
  int count = 0;
  for (unsigned short id : this->ids)
    if (id != 0)
      ++ count;
  return count;
}



chunk::chunk (int x, int z)
  : x (x), z (z), sections (16)
{
  for (int& b : this->biomes)
    b = 1;
}


void
chunk::set_block_id_unsafe (int x, int y, int z, unsigned short id)
{
  this->sections[y >> 4].ids[((y & 0xf) << 8) | (z << 4) | (15 - x)] = id;
  this->section_bitmap |= (1 << (y >> 4));
}

void
chunk::set_block_id (int x, int y, int z, unsigned short id)
{
  if (y < 0 || y >= 256) return;
  if (x < 0 || x >= 16) return;
  if (z < 0 || z >= 16) return;
  this->set_block_id_unsafe (x, y, z, id);
}

unsigned short
chunk::get_block_id_unsafe (int x, int y, int z)
{
  if (this->section_bitmap & (1 << (y >> 4)))
    return this->sections[y >> 4].ids[((y & 0xf) << 8) | (z << 4) | (15 - x)];
  return 0;
}

unsigned short
chunk::get_block_id (int x, int y, int z)
{
  if (y < 0 || y >= 256) return 0;
  if (x < 0 || x >= 16) return 0;
  if (z < 0 || z >= 16) return 0;
  return this->get_block_id_unsafe (x, y, z);
}

void
chunk::set_sky_light_unsafe (int x, int y, int z, unsigned char val)
{
  auto idx = ((y & 0xf) << 8) | (z << 4) | x;
  auto half = idx >> 1;
  auto& section = this->sections[y >> 4];
  if (idx & 1)
    section.sky_light[half] = (section.sky_light[half] & 0x0F) | (val << 4);
  else
    section.sky_light[half] = (section.sky_light[half] & 0xF0) | val;

  this->section_bitmap |= (1 << (y >> 4));
}

void
chunk::set_sky_light (int x, int y, int z, unsigned char val)
{
  if (y < 0 || y >= 256) return;
  if (x < 0 || x >= 16) return;
  if (z < 0 || z >= 16) return;
  this->set_sky_light_unsafe (x, y, z, val);
}

unsigned char
chunk::get_sky_light_unsafe (int x, int y, int z)
{
  if ((this->section_bitmap & (1 << (y >> 4))) == 0)
    return 15;

  auto idx = ((y & 0xf) << 8) | (z << 4) | x;
  auto half = idx >> 1;
  auto& section = this->sections[y >> 4];
  if (idx & 1)
    return section.sky_light[half] >> 4;
  else
    return section.sky_light[half] & 0xf;

}

unsigned char
chunk::get_sky_light (int x, int y, int z)
{
  if (y < 0 || y >= 256) return 0;
  if (x < 0 || x >= 16) return 0;
  if (z < 0 || z >= 16) return 0;
  return this->get_sky_light_unsafe (x, y, z);
}

void
chunk::set_block_light_unsafe (int x, int y, int z, unsigned char val)
{
  auto idx = ((y & 0xf) << 8) | (z << 4) | x;
  auto half = idx >> 1;
  auto& section = this->sections[y >> 4];
  if (idx & 1)
    section.block_light[half] = (section.block_light[half] & 0x0F) | (val << 4);
  else
    section.block_light[half] = (section.block_light[half] & 0xF0) | val;

  this->section_bitmap |= (1 << (y >> 4));
}

void
chunk::set_block_light (int x, int y, int z, unsigned char val)
{
  if (y < 0 || y >= 256) return;
  if (x < 0 || x >= 16) return;
  if (z < 0 || z >= 16) return;
  this->set_block_light_unsafe (x, y, z, val);
}


packet_writer
chunk::make_chunk_data_packet ()
{
  packet_writer writer;

  // generate NBT structure holding height map
  int height_map[256];
  this->compute_height_map (height_map);
  nbt_writer heightmap_nbt;
  heightmap_nbt.start_compound ("");

  uint64_t height_map_packed[36];
  pack_array (height_map, 256, height_map_packed, 9);
  heightmap_nbt.push_long_array (height_map_packed, 36, "MOTION_BLOCKING");

  heightmap_nbt.end_compound ();

  // preprocess sections
  int bitmask = 0;
  int data_size = 1024; // account for biome array
  std::vector<chunk_palette> palettes (16);
  for (int y = 0; y < 16; ++y)
    if (this->section_bitmap & (1 << y))
      {
        auto& section = this->sections[y];
        bitmask |= 1 << y;
        palettes[y] = section.generate_palette ();

        auto& palette = palettes[y];
        auto bits_per_block = palette.compute_bits_per_block ();

        data_size += 2; // number of non-air blocks
        data_size += 1; // bits per block
        if (bits_per_block <= 8)
          {
            data_size += varlong_size (palette.num_blocks ());
            for (unsigned short id : palette.ids)
              data_size += varlong_size (id);
          }

        data_size += varlong_size (bits_per_block * 512); // data array length
        data_size += bits_per_block * 512; // data array
      }

  writer.write_varlong (OPI_CHUNK_DATA);
  writer.write_int (this->x);
  writer.write_int (this->z);
  writer.write_bool (true);
  writer.write_varlong (bitmask);
  writer.write_nbt (heightmap_nbt);
  writer.write_varlong (data_size);

  // data
  for (int y = 0; y < 16; ++y)
    {
      if (!(this->section_bitmap & (1 << y)))
        continue;
      const auto& section = this->sections[y];

      writer.write_short (section.count_non_air_blocks ());

      // generate palette
      auto& palette = palettes[y];
      auto bits_per_block = palette.compute_bits_per_block ();
      writer.write_byte ((char)bits_per_block);

      bool direct = bits_per_block > 8;
      if (!direct)
        {
          // indirect mode
          writer.write_varlong (palette.num_blocks ());
          for (auto id : palette.ids)
            writer.write_varlong (id);
        }
      else
        {
          // direct mode - no palette
        }

      writer.write_varlong (bits_per_block * 64); // data array length

      // prepare unpacked array for packing
      unsigned short raw[4096];
      if (direct)
        std::memcpy (raw, section.ids, 4096);
      else
        {
          for (int i = 0; i < 4096; ++i)
            raw[i] = palette.index_map[section.ids[i]];
        }

      // pack
      std::vector<uint64_t> packed (bits_per_block * 64, 0);
      pack_array<unsigned short, uint64_t> (raw, 4096, packed.data (), bits_per_block);
      for (uint64_t v : packed)
        writer.write_long (v);
    }

  // biomes
  for (int b : this->biomes)
    writer.write_int (b);

  writer.write_varlong (0); // number of block entities

  return writer;
}



//! \brief Computes sky/block lighting for the blocks in chunk.
void
chunk::compute_initial_lighting ()
{
  int height_map[256];
  this->compute_height_map (height_map);

  // all transparent blocks with direct vertical contact with sunlight
  // get skylight value of 15.
  for (int x = 0; x < 16; ++x)
    for (int z = 0; z < 16; ++z)
      {
        for (int y = 255; y > height_map[x * 16 + z]; --y)
          {
            if (this->section_bitmap & (1 << (y >> 4)))
              {
                this->set_sky_light_unsafe (x, y, z, 15);
              }
          }
      }

}

//! \brief Fills the specified height map array with the correct values.
void
chunk::compute_height_map (int height_map[256])
{
  for (int x = 0; x < 16; ++x)
    for (int z = 0; z < 16; ++z)
      {
        height_map[x * 16 + z] = -1;
        for (int y = 255; y >= 0; --y)
          {
            if (!is_transparent_block (this->get_block_id_unsafe (x, y, z)))
              {
                height_map[x * 16 + z] = y;
                break;
              }
          }
      }
}
