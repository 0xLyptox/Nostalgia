//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#ifndef NOSTALGIA_WORLD_HPP
#define NOSTALGIA_WORLD_HPP

#include "system/consts.hpp"
#include "world/chunk.hpp"
#include <string>
#include <map>
#include <utility>
#include <stack>
#include <caf/all.hpp>


struct lighting_update
{
  block_pos pos;
};

class world : public caf::blocking_actor
{
  std::string name;
  std::map<std::pair<int, int>, std::unique_ptr<chunk>> chunks;

  caf::actor world_gen;

  std::stack<lighting_update> lighting_updates;

 public:
  inline const std::string get_name () const { return this->name; }

  world (caf::actor_config& cfg, const std::string& name, caf::actor world_gen);

  virtual void act () override;

 private:
  //! \brief Handles actor messages.
  void handle_messages ();

  //! \brief Processes queued sky/block lighting updates
  void handle_lighting (int max_updates=max_lighting_updates);


  chunk* find_chunk (int cx, int cz);

  void set_block_id (int x, int y, int z, unsigned short id);
  unsigned short get_block_id (int x, int y, int z);

  void set_sky_light (int x, int y, int z, unsigned char val);
  unsigned char get_sky_light (int x, int y, int z);
};

#endif //NOSTALGIA_WORLD_HPP
