//
// Created by Jacob Zhitomirsky on 06-Jul-19.
//

#ifndef NOSTALGIA_GENERATOR_HPP
#define NOSTALGIA_GENERATOR_HPP

#include <caf/all.hpp>


/*!
 * \class world_generator
 * \brief A blocking actor that is responsible for generating chunks in its
 *        own separate thread.
 */
class world_generator : public caf::blocking_actor
{
 public:
  world_generator (caf::actor_config& cfg);

  virtual void act () override;
};

#endif //NOSTALGIA_GENERATOR_HPP
