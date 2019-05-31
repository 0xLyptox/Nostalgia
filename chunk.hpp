//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#ifndef NOSTALGIA_CHUNK_HPP
#define NOSTALGIA_CHUNK_HPP

#include <vector>
#include <memory>


struct chunk_section
{
  unsigned short ids[4096];
  unsigned char block_light[2048];
  unsigned char sky_light[2048];
};

class chunk
{
  int x, y;
  std::vector<std::unique_ptr<chunk_section>> sections;

 public:
  chunk (int x, int y);
};

#endif //NOSTALGIA_CHUNK_HPP
