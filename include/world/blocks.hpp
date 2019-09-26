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

#ifndef NOSTALGIA_BLOCKS_HPP
#define NOSTALGIA_BLOCKS_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <exception>

typedef unsigned short block_id;

//! \brief This defines the maximum amount of properties a block can have at any one time.
constexpr int max_block_properties = 8;

/*!
 * \brief Maps between property names and lists of possible values
 *
 * NOTE: It's very important that map is used (as opposed to unordered_map) since
 *       we require that iteration over property names will be sorted in ascending
 *       order (which is guaranteed by the standard for std::map).
 */
using property_map = std::map<std::string, std::vector<std::string>>;

class block_not_found_error : public std::exception {};

struct block_state
{
  /*!
   * The i'th element corresponds to the i'th property acquired from iterating
   * the block's property map sorted in ascending order. The value it holds is
   * the index of the property value held by this state in the vector
   * corresponding to the matching property (in property_map).
   * I hope what I wrote isn't too hard to understand...
   */
  unsigned char properties[max_block_properties];
  block_id id; // numeric id
};

class block
{
  //
  // member fields:
  //
  std::string name; // namespaced ID
  property_map properties;
  std::vector<block_state> states;
  size_t default_state_idx = 0; // index to default state
  bool solid = true;
  bool transparent = false;

  //
  // static fields:
  //
  static std::vector<block> block_list;
  static std::unordered_map<std::string, size_t> name_map;

 public:
  //! \brief Returns the numeric ID of this block's default state.
  [[nodiscard]] inline block_id get_id () const { return this->states[this->default_state_idx].id; }

  [[nodiscard]] inline const std::string& get_name () const { return this->name; }
  [[nodiscard]] inline bool is_solid () const { return this->solid; }
  [[nodiscard]] inline bool is_transparent () const { return this->transparent; }

  block (const std::string& name);

 public:
  /*!
   * \brief Loads block descriptions from disk.
   */
  static void initialize ();

  /*!
   * \brief Searches for a block using its namespaced ID.
   * \throw block_not_found Thrown if the specified namespaced ID does not match any block.
   */
  static const block& find (const std::string& name);
};



//! \brief Returns true if sky light cannot pass through the specified block.
bool is_opaque_block (unsigned short id);

/*!
 * \brief Returns true if the specified block is completely transparent.
 * NOTE: This is not the opposite of is_opaque_block, as water is not opaque
 *       and not transparent at the same time.
 */
bool is_transparent_block (unsigned short id);

#endif //NOSTALGIA_BLOCKS_HPP
