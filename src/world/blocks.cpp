//
// Created by Jacob Zhitomirsky on 21-Jul-19.
//

#include "world/blocks.hpp"
#include <fstream>
#include <cstdlib>
#include <iostream>


//
// static fields:
//
std::vector<block> block::block_list;
std::unordered_map<std::string, int> block::name_map;


block::block (unsigned short id, const std::string& name, const std::string& friendly_name,
              bool solid, bool transparent)
  : id (id), name (name), friendly_name (friendly_name), solid (solid), transparent (transparent)
{}



void
block::initialize ()
{
  block::block_list.clear ();
  block::name_map.clear ();

  std::ifstream fs ("Blocks.csv");
  if (!fs)
    throw std::runtime_error ("Failed to open Blocks.csv");

  std::string str;

  // skip header line
  std::getline (fs, str, '\n');

  while (fs)
    {
      std::string friendly_name;
      std::string name;
      unsigned short id;
      bool solid, transparent;

      if (!std::getline (fs, friendly_name, ','))
        break;
      std::getline (fs, name, ',');

      std::getline (fs, str, ',');
      id = (unsigned short)std::strtol (str.c_str (), nullptr, 10);

      std::getline (fs, str, ',');
      solid = (std::strtol (str.c_str (), nullptr, 10) == 1);

      std::getline (fs, str, '\n');
      transparent = (std::strtol (str.c_str (), nullptr, 10) == 1);

      // update global lists
      auto index = (int)block::block_list.size ();
      block::block_list.emplace_back (id, name, friendly_name, solid, transparent);
      block::name_map[name] = index;
    }
}

const block&
block::find (const std::string& name)
{
  if (name.find (':') == std::string::npos)
    {
      // no namespace specified, assume "minecraft"
      return block::find ("minecraft:" + name);
    }

  auto itr = block::name_map.find (name);
  if (itr == block::name_map.end ())
    throw block_not_found_error {};
  return block::block_list[itr->second];
}



bool
is_opaque_block (unsigned short id)
{
  return id != 0;
}

bool
is_transparent_block (unsigned short id)
{
  return id == 0;
}
