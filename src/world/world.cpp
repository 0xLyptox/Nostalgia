//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#include "world/world.hpp"
#include "world/blocks.hpp"

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define ABS(A) (((A) < 0) ? (-(A)) : (A))


world::world (caf::actor_config& cfg, const std::string& name, caf::actor world_gen)
  : caf::blocking_actor (cfg), name (name), world_gen (world_gen)
{
  // nop
}


void
world::act ()
{
  this->handle_messages ();
}

//! \brief Handles actor messages.
void
world::handle_messages ()
{
  bool running = true;
  this->receive_while (running) (
      [this] (caf::atom_constant<caf::atom ("reqcdata")>, int cx, int cz, caf::actor broker) {
        // find this chunk
        auto ch = this->find_chunk (cx, cz);
        if (ch)
          {
            // chunk already exists, send it.
            this->send (broker, caf::atom ("packet"), ch->make_chunk_data_packet ().move_data ());
          }
        else
          {
            // chunk does not exist, generate it.
            this->request (this->world_gen, caf::infinite, caf::atom ("generate"), chunk_pos (cx, cz), "flatgrass").receive (
                [this, cx, cz, broker] (chunk ch) {
                  auto key = std::make_pair (cx, cz);
                  auto& new_ch = this->chunks[key] = std::make_unique<chunk> (std::move (ch));

                  // send chunk to player.
                  this->send (broker, caf::atom ("packet"), new_ch->make_chunk_data_packet ().move_data ());
                },
                [this] (caf::error& err) {});
          }
      },

      [this] (caf::atom_constant<caf::atom ("setblock")>, block_pos pos, unsigned short id) {
        chunk_pos cpos = pos;
        caf::aout (this) << "World: setblock at (" << pos.x << ", " << pos.y << ", " << pos.z << ") to " << id << std::endl;
        auto ch = this->find_chunk (cpos.x, cpos.z);
        if (ch)
          {
            caf::aout (this) << "Found chunk at: " << cpos.x << ", " << cpos.z << std::endl;
            caf::aout (this) << "Local coordinates: " << (pos.x & 0xf) << ", " << pos.y << ", " << (pos.z & 0xf) << std::endl;
            ch->set_block_id (pos.x & 0xf, pos.y, pos.z & 0xf, id);

            this->lighting_updates.push(lighting_update { pos });
            this->handle_lighting ();
          }
      },

      [&] (caf::atom_constant<caf::atom ("stop")>) {
        running = false;
      });
}

//! \brief Processes queued sky/block lighting updates
void
world::handle_lighting (int max_updates)
{
  for (int i = 0; i < max_updates; ++i)
    {
      if (this->lighting_updates.empty ())
        break;

      auto update = this->lighting_updates.top ();
      this->lighting_updates.pop ();

      auto pos = update.pos;
      auto id = this->get_block_id (pos.x, pos.y, pos.z);

      caf::aout (this) << "LIGHTING: Handling update (" << pos.x << ", " << pos.y << ", " << pos.z << "): " << id << std::endl;

      if (is_opaque_block (id))
        {
          // TODO
        }
      else
        {
          // get neighbour skylight values
          char sl_up = pos.y < 255 ? this->get_sky_light (pos.x, pos.y + 1, pos.z) : (char)15;
          char sl_down = pos.y > 0 ? this->get_sky_light (pos.x, pos.y - 1, pos.z) : (char)15;
          char sl_right = this->get_sky_light (pos.x + 1, pos.y, pos.z);
          char sl_left = this->get_sky_light (pos.x - 1, pos.y, pos.z);
          char sl_front = this->get_sky_light (pos.x, pos.y, pos.z + 1);
          char sl_back = this->get_sky_light (pos.x, pos.y, pos.z - 1);

          caf::aout (this) << "LIGHTING: Neighbour values: " << (int)sl_up << ", " <<(int)sl_down << ", " <<(int)sl_right << ", " <<(int)sl_left << ", " <<(int)sl_front << ", " <<(int)sl_back << std::endl;

          char this_sl = 0;
          if (sl_up == 15)
            // propagate vertical sunlight without change
            this_sl = 15;
          else
            this_sl = MAX(sl_up, MAX(sl_down, MAX(sl_right, MAX(sl_left, MAX(sl_front, sl_back))))) - (char)1;

          // keep value within bounds
          if (this_sl < 0)
            this_sl = 0;

          caf::aout (this) << "LIGHTING: Calculated value: " << (int)this_sl << std::endl;

          this->set_sky_light (pos.x, pos.y, pos.z, (unsigned char)this_sl);

          // enqueue all neighbours if their sky light value differs by more than one
          if (pos.y < 255 && ABS(this_sl - sl_up) > 1 && !is_opaque_block (this->get_block_id (pos.x, pos.y + 1, pos.z)))
            this->lighting_updates.push (lighting_update { block_pos (pos.x, pos.y + 1, pos.z) });
          if (pos.y > 0 && !is_opaque_block (this->get_block_id (pos.x, pos.y - 1, pos.z)))
            {
              if (ABS(this_sl - sl_down) > 1 || (this_sl == 15 && sl_down != 15))
                this->lighting_updates.push (lighting_update { block_pos (pos.x, pos.y - 1, pos.z) });
            }
          if (ABS(this_sl - sl_right) > 1 && !is_opaque_block (this->get_block_id (pos.x + 1, pos.y, pos.z)))
            this->lighting_updates.push (lighting_update { block_pos (pos.x + 1, pos.y, pos.z) });
          if (ABS(this_sl - sl_left) > 1 && !is_opaque_block (this->get_block_id (pos.x - 1, pos.y, pos.z)))
            this->lighting_updates.push (lighting_update { block_pos (pos.x - 1, pos.y, pos.z) });
          if (ABS(this_sl - sl_front) > 1 && !is_opaque_block (this->get_block_id (pos.x, pos.y, pos.z + 1)))
            this->lighting_updates.push (lighting_update { block_pos (pos.x, pos.y, pos.z + 1) });
          if (ABS(this_sl - sl_back) > 1 && !is_opaque_block (this->get_block_id (pos.x, pos.y, pos.z - 1)))
            this->lighting_updates.push (lighting_update { block_pos (pos.x, pos.y, pos.z - 1) });
        }
    }
}



chunk*
world::find_chunk (int cx, int cz)
{
  auto key = std::make_pair (cx, cz);
  auto itr = this->chunks.find (key);
  if (itr == this->chunks.end ())
    return nullptr;
  return itr->second.get ();
}

void
world::set_block_id (int x, int y, int z, unsigned short id)
{
  chunk_pos cp = block_pos (x, y, z);
  auto ch = this->find_chunk (cp.x, cp.z);
  if (ch)
    ch->set_block_id (x & 0xf, y, z & 0xf, id);
}

unsigned short
world::get_block_id (int x, int y, int z)
{
  chunk_pos cp = block_pos (x, y, z);
  auto ch = this->find_chunk (cp.x, cp.z);
  return ch ? ch->get_block_id (x & 0xf, y, z & 0xf) : (unsigned short)0;
}

void
world::set_sky_light (int x, int y, int z, unsigned char val)
{
  chunk_pos cp = block_pos (x, y, z);
  auto ch = this->find_chunk (cp.x, cp.z);
  if (ch)
    ch->set_sky_light (x & 0xf, y, z & 0xf, val);
}

unsigned char
world::get_sky_light (int x, int y, int z)
{
  chunk_pos cp = block_pos (x, y, z);
  auto ch = this->find_chunk (cp.x, cp.z);
  return ch ? ch->get_sky_light (x & 0xf, y, z & 0xf) : (unsigned char)15;
}
