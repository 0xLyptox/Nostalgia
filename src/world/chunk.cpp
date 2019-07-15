//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#include "world/chunk.hpp"
#include "system/consts.hpp"
#include <set>

#include <iostream> // DEBUG


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



chunk::chunk (int x, int z)
  : x (x), z (z), sections (16)
{
  // nop
}


void
chunk::set_block_id_unsafe (int x, int y, int z, unsigned short id)
{
  this->sections[y >> 4].ids[((y & 0xf) << 8) | (z << 4) | x] = id;
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


packet_writer
chunk::make_chunk_data_packet ()
{
  packet_writer writer;

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

        data_size += 1; // bits per block
        if (bits_per_block <= 8)
          {
            data_size += varlong_size (palette.num_blocks ());
            for (unsigned short id : palette.ids)
              data_size += varlong_size (id);
          }

        data_size += varlong_size (bits_per_block * 512); // data array length
        data_size += bits_per_block * 512; // data array
        data_size += 4096; // block light + sky light arrays
      }

  writer.write_varlong (OPI_CHUNK_DATA);
  writer.write_int (this->x);
  writer.write_int (this->z);
  writer.write_bool (true);
  writer.write_varlong (bitmask);
  writer.write_varlong (data_size);

  // data
  for (int y = 0; y < 16; ++y)
    {
      if (!(this->section_bitmap & (1 << y)))
        continue;
      const auto& section = this->sections[y];

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
      uint64_t curr = 0;
      unsigned curr_offset = 0;
      unsigned int len = 0;
      for (unsigned short id : section.ids)
        {
          // this is the value we encode
          int val = direct ? id : palette.index_map[id];

          // how many bits to occupy in current qword
          unsigned take = bits_per_block;
          if (64 - curr_offset < take)
            take = 64 - curr_offset;

          auto head = val >> (bits_per_block - take);
          auto tail = val & ((1 << (bits_per_block - take)) - 1);

          curr <<= take;
          curr |= head;
          curr_offset += take;
          if (curr_offset == 64)
            {
              // filled a qword
              writer.write_long (curr);
              len += 8;

              // move remainder to next qword if there is one
              curr_offset = bits_per_block - take;
              curr = tail;
            }
        }

      writer.write_bytes (section.block_light, 2048);
      writer.write_bytes (section.sky_light, 2048);
    }

  // biomes
  for (int b : this->biomes)
    writer.write_int (b);

  writer.write_varlong (0); // number of block entities

  return writer;
}


