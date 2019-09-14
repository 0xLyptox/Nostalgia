//
// Created by Jacob Zhitomirsky on 10-May-19.
//

#include "world/world.hpp"
#include "world/blocks.hpp"
#include "system/atoms.hpp"
#include "network/packets.hpp"

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define ABS(A) (((A) < 0) ? (-(A)) : (A))


world::world (caf::actor_config& cfg, unsigned int id, const std::string& name,
    const caf::actor& srv, const caf::actor& script_eng, const caf::actor& world_gen)
  : caf::blocking_actor (cfg), srv (srv), script_eng (script_eng), world_gen (world_gen)
{
  this->info.id = id;
  this->info.actor = this;
  this->info.name = name;
}


void
world::act ()
{
  // register with scripting engine
  this->send (this->script_eng, register_world_atom::value, this->info);

  this->handle_messages ();
}

//! \brief Handles actor messages.
void
world::handle_messages ()
{
  bool running = true;
  this->receive_while (running) (
      [this] (request_chunk_data_atom, int cx, int cz, const caf::actor& broker) {
        // find this chunk
        auto ch = this->find_chunk (cx, cz);
        if (ch)
          {
            // chunk already exists, send it.
            this->send (broker, packet_out_atom::value, ch->make_chunk_data_packet ().move_data ());
          }
        else
          {
            // chunk does not exist, generate it.
            this->request (this->world_gen, caf::infinite, generate_atom::value, chunk_pos (cx, cz), "flatgrass").receive (
                [this, cx, cz, broker] (chunk ch) {
                  auto key = std::make_pair (cx, cz);
                  auto& new_ch = this->chunks[key] = std::make_unique<chunk> (std::move (ch));

                  // send chunk to player.
                  this->send (broker, packet_out_atom::value, new_ch->make_chunk_data_packet ().move_data ());
                },
                [this] (caf::error& err) {});
          }
      },

      [this] (set_block_atom, block_pos pos, unsigned short id) {
        chunk_pos cpos = pos;
//        caf::aout (this) << "World: setblock at (" << pos.x << ", " << pos.y << ", " << pos.z << ") to " << id << std::endl;
        auto ch = this->find_chunk (cpos.x, cpos.z);
        if (ch)
          {
            ch->set_block_id (pos.x & 0xf, pos.y, pos.z & 0xf, id);

            // update players
            // TODO: Send this update only to players that are in range!!!
            this->send (this->srv, broadcast_packet_atom::value, packets::play::make_block_change (pos, id).move_data ());

            this->lighting_updates.push(lighting_update { pos });
            this->handle_lighting ();
          }
      },

      [&] (stop_atom) {
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

//      caf::aout (this) << "LIGHTING: Handling update (" << pos.x << ", " << pos.y << ", " << pos.z << "): " << id << std::endl;

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

//          caf::aout (this) << "LIGHTING: Neighbour values: " << (int)sl_up << ", " <<(int)sl_down << ", " <<(int)sl_right << ", " <<(int)sl_left << ", " <<(int)sl_front << ", " <<(int)sl_back << std::endl;

          char this_sl = 0;
          if (sl_up == 15)
            // propagate vertical sunlight without change
            this_sl = 15;
          else
            this_sl = MAX(sl_up, MAX(sl_down, MAX(sl_right, MAX(sl_left, MAX(sl_front, sl_back))))) - (char)1;

          // keep value within bounds
          if (this_sl < 0)
            this_sl = 0;

//          caf::aout (this) << "LIGHTING: Calculated value: " << (int)this_sl << std::endl;

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
