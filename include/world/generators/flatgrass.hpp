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

#ifndef NOSTALGIA_FLATGRASS_HPP
#define NOSTALGIA_FLATGRASS_HPP

#include "world/generator.hpp"
#include <memory>


/*!
 * \brief Classic flatgrass world generator.
 */
class flatgrass_world_generator : public world_generator
{
  struct palette_t; // forward dec
  std::unique_ptr<palette_t> palette;

 public:
  flatgrass_world_generator ();

  [[nodiscard]] const char* get_name () const override { return "flatgrass"; }

  void generate_chunk (int cx, int cz, chunk& ch) override;
};

#endif //NOSTALGIA_FLATGRASS_HPP
