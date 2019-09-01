//
// Created by Jacob Zhitomirsky on 10-Aug-19.
//

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
