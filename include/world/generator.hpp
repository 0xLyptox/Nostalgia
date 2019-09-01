//
// Created by Jacob Zhitomirsky on 06-Aug-19.
//

#ifndef NOSTALGIA_GENERATOR_HPP
#define NOSTALGIA_GENERATOR_HPP

#include "world/chunk.hpp"


/*!
 * \class world_generator
 * \brief Abstract base class for world generator implementations.
 */
class world_generator
{
 public:
  //! \brief Returns the name of the generator.
  [[nodiscard]] virtual const char* get_name () const = 0;

  //! \brief Populates the specified chunk.
  virtual void generate_chunk (int cx, int cz, chunk& ch) = 0;
};


#endif //NOSTALGIA_GENERATOR_HPP
