//
// Created by Jacob Zhitomirsky on 21-Jul-19.
//

#include "world/blocks.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <iterator>

#define BLOCK_REPORT_PATH "data/blocks.json"


//
// static fields:
//
std::vector<block> block::block_list;
std::unordered_map<std::string, size_t> block::name_map;


block::block (const std::string& name)
  : name (name)
{}



void
block::initialize ()
{
  std::ifstream fs (BLOCK_REPORT_PATH);
  if (!fs)
    throw std::runtime_error ("Failed to open block report file");

  auto j = nlohmann::json::parse (fs);

  for (auto itr = j.begin (); itr != j.end (); ++itr)
    {
      const auto& block_name = itr.key ();
      auto& j_block = itr.value ();

      name_map[block_name] = block::block_list.size ();
      block::block_list.emplace_back (block_name);
      auto& block = block::block_list.back ();

      //
      // Load properties
      //
      if (j_block.find ("properties") != j_block.end ())
        {
          auto& j_props = j_block["properties"];
          for (auto itr2 = j_props.begin (); itr2 != j_props.end (); ++itr2)
            {
              const auto& prop_name = itr2.key ();
              auto& j_values = itr2.value ();
              auto& prop = block.properties[prop_name];
              for (auto& j_value : j_values)
                prop.push_back (j_value.get<std::string> ());
              if (prop.size () >= 0x100)
                throw std::runtime_error ("Too many properties");
            }

          if (block.properties.size () > max_block_properties)
            throw std::runtime_error ("There exists a block with too many properties");
        }

      //
      // Load states
      //
      for (auto& j_state : j_block["states"])
        {
          block.states.emplace_back ();
          auto& state = block.states.back ();

          state.id = j_state["id"].get<block_id> ();

          if (j_state.find ("properties") != j_state.end ())
            {
              auto& j_props = j_state["properties"];
              size_t prop_idx = 0;
              for (auto& p : block.properties)
                {
                  auto& prop_name = p.first;
                  auto& prop_vals = p.second;

                  if (j_props.find (prop_name) == j_props.end ())
                    throw std::runtime_error ("Block report contains an incomplete block");

                  auto val = j_props[prop_name].get<std::string> ();
                  auto val_itr = std::find (prop_vals.begin (), prop_vals.end (), val);
                  if (val_itr == prop_vals.end ())
                    throw std::runtime_error ("Block state holds a previously undefined property");

                  auto val_idx = std::distance (prop_vals.begin (), val_itr);
                  state.properties[prop_idx] = (unsigned char)val_idx;

                  ++ prop_idx;
                }
            }

          // default?
          if (j_state.find ("default") != j_state.end () && j_state["default"].get<bool> ())
            block.default_state_idx = block.states.size () - 1;
        }
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
