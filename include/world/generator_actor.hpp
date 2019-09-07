//
// Created by Jacob Zhitomirsky on 06-Jul-19.
//

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
