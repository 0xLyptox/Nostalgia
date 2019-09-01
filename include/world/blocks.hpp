//
// Created by Jacob Zhitomirsky on 21-Jul-19.
//

#ifndef NOSTALGIA_BLOCKS_HPP
#define NOSTALGIA_BLOCKS_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <exception>


typedef unsigned short block_id;

class block_not_found_error : public std::exception {};

class block
{
  //
  // member fields:
  //
  block_id id; // numeric id
  std::string name;  // namespaced id
  std::string friendly_name;
  bool solid;
  bool transparent;

  //
  // static fields:
  //
  static std::vector<block> block_list;
  static std::unordered_map<std::string, int> name_map;

 public:
  [[nodiscard]] inline block_id get_id () const { return this->id; }
  [[nodiscard]] inline const std::string& get_name () const { return this->name; }
  [[nodiscard]] inline const std::string& get_friendly_name () const { return this->friendly_name; }
  [[nodiscard]] inline bool is_solid () const { return this->solid; }
  [[nodiscard]] inline bool is_transparent () const { return this->transparent; }

  block (unsigned short id, const std::string& name, const std::string& friendly_name, bool solid, bool transparent);

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

//namespace block {
//
//  enum BLOCK_ID : unsigned short
//  {
//    STONE = 1,
//    GRANITE,
//    POLISHED_GRANITE,
//    DIORITE,
//    POLISHED_DIORITE,
//    ANDESITE,
//    POLISHED_ANDESITE,
//    SNOWY_GRASS_BLOCK,
//    GRASS_BLOCK,
//    DIRT,
//    COARSE_DIRT,
//    SNOWY_PODZOL,
//    PODZOL,
//    COBBLESTONE,
//    OAK_PLANKS,
//    SPRUCE_PLANKS,
//    BIRCH_PLANKS,
//    JUNGLE_PLANKS,
//    ACACIA_PLANKS,
//    DARK_OAK_PLANKS,
//    OAK_SAPLING_STAGE_0,
//    OAK_SAPLING_STAGE_1,
//    SPRUCE_SAPLING_STAGE_0,
//    SPRUCE_SAPLING_STAGE_1,
//    BIRCH_SAPLING_STAGE_0,
//    BIRCH_SAPLING_STAGE_1,
//    JUNGLE_SAPLING_STAGE_0,
//    JUNGLE_SAPLING_STAGE_1,
//    ACACIA_SAPLING_STAGE_0,
//    ACACIA_SAPLING_STAGE_1,
//    DARK_OAK_SAPLING_STAGE_0,
//    DARK_OAK_SAPLING_STAGE_1,
//    BEDROCK,
//    WATER,
//    FLOWING_WATER_7,
//    FLOWING_WATER_6,
//    FLOWING_WATER_5,
//    FLOWING_WATER_4,
//    FLOWING_WATER_3,
//    FLOWING_WATER_2,
//    FLOWING_WATER_1,
//    FALLING_FLOWING_WATER_8,
//    FALLING_FLOWING_WATER_7,
//    FALLING_FLOWING_WATER_6,
//    FALLING_FLOWING_WATER_5,
//    FALLING_FLOWING_WATER_4,
//    FALLING_FLOWING_WATER_3,
//    FALLING_FLOWING_WATER_2,
//    FALLING_FLOWING_WATER_1,
//  };
//}

//! \brief Returns true if sky light cannot pass through the specified block.
bool is_opaque_block (unsigned short id);

/*!
 * \brief Returns true if the specified block is completely transparent.
 * NOTE: This is not the opposite of is_opaque_block, as water is not opaque
 *       and not transparent at the same time.
 */
bool is_transparent_block (unsigned short id);

#endif //NOSTALGIA_BLOCKS_HPP
