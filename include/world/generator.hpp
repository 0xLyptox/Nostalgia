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

#ifndef NOSTALGIA_GENERATOR_HPP
#define NOSTALGIA_GENERATOR_HPP

#include "world/chunk.hpp"


/*!
 * \class world_generator
 * \brief Abstract base class for world generator implementations.
 */
class world_generator
{
 public:
  //! \brief Returns the name of the generator.
  [[nodiscard]] virtual const char* get_name () const = 0;

  //! \brief Populates the specified chunk.
  virtual void generate_chunk (int cx, int cz, chunk& ch) = 0;
};


#endif //NOSTALGIA_GENERATOR_HPP
