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

#include "world/providers/nw1/nw1.hpp"
#include "world/providers/nw1/compress.hpp"
#include <stdexcept>
#include <cstring>

#include "world/blocks.hpp"  // DEBUG


void
nw1_world_provider::open (const std::string& path)
{
  this->fs.open (path, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  if (this->fs)
    {
      // opened existing world file
      this->read_known_chunks ();
    }
  else
    {
      // world file might not exist, try creating it
      this->fs.open (path, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
      if (!this->fs)
        throw std::runtime_error ("Failed to open world file");

      this->create_header_page ();

      // create first known chunk page
      this->known_chunk_pages.push_back (1);
      this->write_zeroes (this->page_size);
      this->fs.flush ();
    }
}

void
nw1_world_provider::close ()
{
  this->fs.close ();
}

bool
nw1_world_provider::can_load_chunk (int cx, int cz)
{
  return this->known_chunks.find (std::make_pair (cx, cz)) != this->known_chunks.end ();
}





static void
_deserialize_chunk (chunk& ch, const unsigned char *bytes)
{
  auto ptr = bytes;
  unsigned int section_bitmap = *ptr++;
  for (unsigned int y = 0; y < 16; ++y)
    {
      if (section_bitmap & (1U << y))
        {
          auto& section = ch.get_section (y);
          ptr += nw1::decompress_array<unsigned short> (ptr, section.ids, 4096);
          ch.add_section (y);
        }
    }
}

chunk
nw1_world_provider::load_chunk (int cx, int cz)
{
  if (!this->can_load_chunk (cx, cz))
    throw chunk_load_error {};

  chunk ch (cx, cz);
  ch.mark_dirty (false);

  auto kc = this->known_chunks[std::make_pair (cx, cz)];
  auto data = this->read_data (kc.page_idx);

  _deserialize_chunk (ch, reinterpret_cast<const unsigned char *> (data.data ()));

  return ch;
}



static std::string
_serialize_chunk (chunk& ch)
{
  std::string data;

  auto section_bitmap = ch.get_section_bitmap ();
  data.push_back ((char)section_bitmap);

  for (unsigned y = 0; y < 16; ++y)
    {
      if (ch.has_section (y))
        {
          auto& section = ch.get_section (y);
          nw1::compress_array<unsigned short> (section.ids, 4096, data);
        }
    }

  return data;
}

void
nw1_world_provider::save_chunk (chunk& ch)
{
  auto data = _serialize_chunk (ch);

  auto key = std::make_pair (ch.get_x (), ch.get_z ());
  auto itr = this->known_chunks.find (key);
  if (itr == this->known_chunks.end ())
    {
      // chunk does not exist in file
      auto page_idx = this->create_pages (data.data (), data.size ());
      this->add_known_chunk (ch.get_x (), ch.get_z (), page_idx);
    }
  else
    {
      // chunk already exists in file
      this->overwrite_pages (itr->second.page_idx, data.data (), data.size ());
    }
}



void
nw1_world_provider::write_zeroes (size_t count)
{
  char *zeroes = new char[count];
  std::memset (zeroes, 0, count);
  this->fs.write (zeroes, count);
  delete[] zeroes;
}

void
nw1_world_provider::create_header_page ()
{
  this->fs.seekp (0, std::ios_base::beg);
  this->write_zeroes (this->page_size);
  this->fs.flush ();
}

size_t
nw1_world_provider::create_pages (const void *data, size_t len, bool write_size)
{
  this->fs.seekp (0, std::ios_base::end);
  auto start_page_idx = this->fs.tellp () / this->page_size;

  const char *bytes = static_cast<const char *> (data);
  size_t left = len, pos = 0;
  for (int i = 0; left > 0; ++i)
    {
      auto remaining_page_size = this->page_size;

      if (i == 0 && write_size)
        {
          // write data size
          this->fs.write (reinterpret_cast<char *> (&len), 4);
          remaining_page_size -= 4;
        }

      // write page index of next page
      auto next_page_idx = (left > remaining_page_size) ? (start_page_idx + i + 1) : 0;
      this->fs.write (reinterpret_cast<char *> (&next_page_idx), 4);
      remaining_page_size -= 4;

      if (left >= remaining_page_size)
        {
          this->fs.write (bytes + pos, remaining_page_size);
          pos += remaining_page_size;
          left -= remaining_page_size;
        }
      else
        {
          this->fs.write (bytes + pos, left);
          this->write_zeroes (remaining_page_size - left);  // pad remaining space
          pos += left;
          left = 0;
        }
    }

  this->fs.flush ();

  return start_page_idx;
}

void
nw1_world_provider::overwrite_pages (size_t start_page_idx, const void *data, size_t len)
{
  const char *bytes = static_cast<const char *> (data);

  this->fs.seekp (start_page_idx * this->page_size);

  // write data size
  this->fs.write (reinterpret_cast<char *> (&len), 4);

  size_t page_idx = start_page_idx;
  size_t left = len, pos = 0;
  while (page_idx != 0 && left > 0)
    {
      // write head is pointing at next page index right now

      // compute how much data we will write in this page
      auto remaining_page_size = (page_idx == start_page_idx) ? (this->page_size - 8) : (this->page_size - 4);
      auto take = left >= remaining_page_size ? remaining_page_size : left;

      // read next page idx
      size_t next_page_idx = 0;
      this->fs.seekg (this->fs.tellp ());
      this->fs.read (reinterpret_cast<char *> (&next_page_idx), 4);
      if (next_page_idx == 0 && left > take)
        {
          // we will need to start allocating new pages
          // set the next page index of the current page to point to the end of the file,
          // where the leftover pages will be written to.
          size_t prev_pos = this->fs.tellg ();
          this->fs.seekp (0, std::ios_base::end);
          auto new_page_idx = this->fs.tellp () / this->page_size;
          this->fs.seekp (prev_pos - 4);
          this->fs.write (reinterpret_cast<char *> (&new_page_idx), 4);
        }
      else
        {
          // no need to change next page index
          this->fs.seekp (this->fs.tellg ());
        }

      this->fs.write (bytes + pos, take);
      if (take < remaining_page_size)  // pad if necessary
        this->write_zeroes (remaining_page_size - take);

      this->fs.seekp (next_page_idx * this->page_size);
      page_idx = next_page_idx;
      left -= take;
      pos += take;
    }

  if (left > 0)
    {
      // have more data left, but we exhausted all existing pages allocated for this chunk
      // allocate more pages
      this->create_pages (bytes + pos, left, false);
    }

  this->fs.flush ();
}

void
nw1_world_provider::add_known_chunk (int cx, int cz, size_t data_page_idx)
{
  size_t kc_entry_pos = 0;

  // determine known chunk entry position
  auto entries_per_page = ((this->page_size - 4) / known_chunk_entry_size);
  auto known_chunk_entry_page_num = this->known_chunks.size () / entries_per_page;
  auto known_chunk_entry_idx = this->known_chunks.size () % entries_per_page;

  if (known_chunk_entry_page_num < this->known_chunk_pages.size ())
    {
      // matching known chunk page already exists
      auto kc_page_idx = this->known_chunk_pages[known_chunk_entry_page_num];
      kc_entry_pos = kc_page_idx * this->page_size + 4 + known_chunk_entry_idx * known_chunk_entry_size;

      this->fs.seekp (kc_entry_pos);
      this->fs.write (reinterpret_cast<char *> (&cx), 4);
      this->fs.write (reinterpret_cast<char *> (&cz), 4);
      this->fs.write (reinterpret_cast<char *> (&data_page_idx), 4);
    }
  else
    {
      // not enough space in previous known chunk pages, must create new one

      this->fs.seekp (0, std::ios_base::end);
      auto kc_page_idx = this->fs.tellp () / this->page_size;
      kc_entry_pos = kc_page_idx * this->page_size + 4;

      this->write_zeroes (4); // pointer to next page
      this->fs.write (reinterpret_cast<char *> (&cx), 4);
      this->fs.write (reinterpret_cast<char *> (&cz), 4);
      this->fs.write (reinterpret_cast<char *> (&data_page_idx), 4);

      this->write_zeroes (this->page_size - (4 + known_chunk_entry_size)); // pad

      // link previous known chunk page to the new page we created
      this->fs.seekp (this->known_chunk_pages.back () * this->page_size);
      this->fs.write (reinterpret_cast<char *> (&kc_page_idx), 4);

      this->known_chunk_pages.push_back (kc_page_idx);
    }

  this->known_chunks[std::make_pair (cx, cz)] = { (unsigned int)kc_entry_pos, (unsigned int)data_page_idx };
  this->fs.flush ();
}

void
nw1_world_provider::read_known_chunks ()
{
  this->known_chunk_pages.clear ();
  this->known_chunks.clear ();

  size_t page_idx = 1;
  do
    {
      this->known_chunk_pages.push_back (page_idx);
      this->fs.seekg (page_idx * this->page_size);

      // read next known chunk page
      size_t next_page_idx = 0;
      this->fs.read (reinterpret_cast<char *> (&next_page_idx), 4);

      // read entries
      auto entry_pos = (unsigned int)(page_idx * this->page_size + 4);
      auto entries_per_page = ((this->page_size - 4) / known_chunk_entry_size);
      for (size_t i = 0; i < entries_per_page; ++i, entry_pos += known_chunk_entry_size)
        {
          int cx, cz;
          size_t data_page_idx = 0;

          this->fs.read (reinterpret_cast<char *> (&cx), 4);
          this->fs.read (reinterpret_cast<char *> (&cz), 4);
          this->fs.read (reinterpret_cast<char *> (&data_page_idx), 4);

          if (data_page_idx == 0)
            break; // end of entries

          auto kc_entry = known_chunk { entry_pos, (unsigned int)data_page_idx };
          this->known_chunks[std::make_pair (cx, cz)] = kc_entry;
        }

      page_idx = next_page_idx;
    }
  while (page_idx != 0);
}

std::string
nw1_world_provider::read_data (size_t start_page_idx)
{
  this->fs.seekg (start_page_idx * this->page_size);

  // read data size
  unsigned int data_size = 0;
  this->fs.read (reinterpret_cast<char *> (&data_size), 4);

  std::string data (data_size, 0);
  size_t bytes_read = 0, bytes_left = data_size;
  while (bytes_read < data_size)
    {
      // read next page index
      size_t next_page_idx = 0;
      this->fs.read (reinterpret_cast<char *> (&next_page_idx), 4);

      auto remaining_page_size = (bytes_read == 0) ? (this->page_size - 8) : (this->page_size - 4);
      auto take = bytes_left >= remaining_page_size ? remaining_page_size : bytes_left;
      this->fs.read (data.data () + bytes_read, take);

      bytes_read += take;
      bytes_left -= take;
      if (bytes_left > 0)
        {
          // move to next page
          if (next_page_idx == 0)
            throw std::runtime_error ("bad data");

          this->fs.seekg (next_page_idx * this->page_size);
        }
    }

  return data;
}
