/* -*- c++ -*- */
/* 
 * Copyright 2020 gr-howto author.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_HOWTO_MSGPOOL_IMPL_H
#define INCLUDED_HOWTO_MSGPOOL_IMPL_H

#include <howto/msgpool.h>
#include<unordered_map>

namespace gr {
  namespace howto {

    struct Node{
      uint8_t revnum_;
      char* data;
      Node() : revnum_(0) {
        data = new char[243];
      }
      ~Node() {
        delete[] data;
      }
    };

    class msgpool_impl : public msgpool
    {

     public:
      msgpool_impl(int threshold);
      ~msgpool_impl();

      using Nodeptr = std::shared_ptr<Node>;
      using PacketMap = std::unordered_map<uint8_t,Nodeptr>;

      void handle_fun(pmt::pmt_t msg);

      // Where all the action really happens

      private:
      // Nothing to declare in this block.
      int threshold_;
      PacketMap packetmap_;

      int* cur_burst_num;
      

      
    };

  } // namespace howto
} // namespace gr

#endif /* INCLUDED_HOWTO_MSGPOOL_IMPL_H */

