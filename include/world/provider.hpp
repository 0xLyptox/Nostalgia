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

#ifndef NOSTALGIA_WORLD_PROVIDER_HPP
#define NOSTALGIA_WORLD_PROVIDER_HPP

#include <string>
#include <memory>
#include "world/chunk.hpp"


//! \brief Thrown when a provider cannot serve a load request.
class chunk_load_error : public std::exception {};

/*!
 * \brief Base class for providers responsible for reading/writing world data from/to disk.
 */
class world_provider
{
 public:
  virtual ~world_provider () = default;

  //! \brief Sets the world file/directory path and initializes the provider.
  virtual void open (const std::string& path) = 0;

  //! \brief Closes the world's file/directory.
  virtual void close () = 0;

  //! \brief Returns true if the provider can serve a load request for a chunk at the given coordinates.
  virtual bool can_load_chunk (int cx, int cz) = 0;

  /*!
   * \brief Loads a chunk at the specified chunk coordinates.
   * \throws chunk_load_error If the provider cannot serve the request (e.g. chunk doesn't exist).
   */
  virtual chunk load_chunk (int cx, int cz) = 0;

  //! \brief Saves a chunk to disk.
  virtual void save_chunk (chunk& ch) = 0;
};


/*!
 * \brief Creates a new world provider for the specified world format (name).
 */
std::unique_ptr<world_provider> make_world_provider (const std::string& prov_name);

#endif //NOSTALGIA_WORLD_PROVIDER_HPP
