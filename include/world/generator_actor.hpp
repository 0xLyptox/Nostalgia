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

#ifndef NOSTALGIA_GENERATOR_ACTOR_HPP
#define NOSTALGIA_GENERATOR_ACTOR_HPP

#include <caf/all.hpp>
#include <vector>
#include <memory>


// forward decs:
class world_generator;

/*!
 * \class world_generator_actor
 * \brief A blocking actor that is responsible for generating chunks in its
 *        own separate thread.
 */
class world_generator_actor : public caf::blocking_actor
{
  std::vector<std::unique_ptr<world_generator>> generators;

 public:
  world_generator_actor (caf::actor_config& cfg);

  void act () override;
};

#endif //NOSTALGIA_GENERATOR_HPP
