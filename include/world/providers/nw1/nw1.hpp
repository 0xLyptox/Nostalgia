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

#ifndef NOSTALGIA_WORLD_PROVIDERS_NW1_NW1_HPP
#define NOSTALGIA_WORLD_PROVIDERS_NW1_NW1_HPP

#include "world/provider.hpp"
#include <fstream>
#include <map>
#include <utility>

constexpr size_t default_page_size = 1024;
constexpr size_t known_chunk_entry_size = 12; // x + y + page_idx

static_assert ((default_page_size - 4) % known_chunk_entry_size == 0, "bad page size");


/*!
 * \brief A *very* simple single-file world format provider.
 * TODO: A big flaw this provider currently has is that it does not clean up
 *       dead pages which can accumulate if chunk data gets smaller after it
 *       already been saved to disk.
 */
class nw1_world_provider : public world_provider
{
  struct known_chunk
  {
    unsigned int entry_pos;
    unsigned int page_idx;
  };

  std::fstream fs;
  size_t page_size = default_page_size;
  std::map<std::pair<int, int>, known_chunk> known_chunks;
  std::vector<size_t> known_chunk_pages;

 public:
  void open (const std::string& path) override;

  void close () override;

  bool can_load_chunk (int cx, int cz) override;

  chunk load_chunk (int cx, int cz) override;

  void save_chunk (chunk& ch) override;

 private:
  //! \brief Creates the very first header page.
  void create_header_page ();

  //! \brief Creates and fills new data pages and returns the page index of the first page.
  size_t create_pages (const void *data, size_t len, bool write_size = true);

  //! \brief Overwrites the pages starting at \p start_page_idx with the specified data (of a possibly larger size).
  void overwrite_pages (size_t start_page_idx, const void *data, size_t len);

  //! \brief Inserts a new chunk to the known chunk list.
  void add_known_chunk (int cx, int cz, size_t data_page_idx);

  //! \brief Loads the known chunk table from disk.
  void read_known_chunks ();

  //! \brief Returns the data stored in the data pages starting at \p start_page_idx.
  std::string read_data (size_t start_page_idx);

  void write_zeroes (size_t count);
};

#endif //NOSTALGIA_WORLD_PROVIDERS_NW1_NW1_HPP
