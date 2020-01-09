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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "msgpool_impl.h"

namespace gr {
  namespace howto {

    msgpool::sptr
    msgpool::make(int threshold,uint8_t netnum)
    {
      return gnuradio::get_initial_sptr
        (new msgpool_impl(threshold,netnum));
    }

    /*
     * The private constructor
     */
    msgpool_impl::msgpool_impl(int threshold,uint8_t netnum)
      : gr::block("msgpool",
              gr::io_signature::make(0,0,0),
              gr::io_signature::make(0,0,0)),
              threshold_(threshold),netnum_(netnum)
    {
      message_port_register_in(pmt::mp("in"));
      message_port_register_out(pmt::mp("out"));

      set_msg_handler(pmt::mp("in"),boost::bind(&msgpool_impl::handle_fun,this,_1));

      cur_burst_num = new int[255];
      memset(cur_burst_num,0,255);

      netnum_ <<= 5;
    }

    /*
     * Our virtual destructor.
     */
    msgpool_impl::~msgpool_impl()
    {
      delete[] cur_burst_num;
    }

    void 
    msgpool_impl::handle_fun(pmt::pmt_t msg) {

      uint8_t netnum;
      uint8_t burstnum;
      uint8_t packetnum;
      uint8_t recv;
      uint8_t net_get = 224;
      uint8_t burst_get = 31;
      std::string str = pmt::symbol_to_string(msg);
      memcpy(&recv,&str[1],1);
      netnum = recv & net_get;
     // std::cout << netnum*1 << std::endl;
      if(netnum != netnum_) return;  //如果不是发给本机，丢弃

      burstnum = recv & burst_get;
      cur_burst_num[packetnum] = burstnum;

      memcpy(&packetnum,&str[0],1);
      if(burstnum < cur_burst_num[packetnum]) {
        packetmap_[packetnum]->revnum_ = 0;
      }                                          //如果接收到新的包，就更新revnum,继续处理。
      cur_burst_num[packetnum] = burstnum;
      auto it = packetmap_.find(packetnum);
      if(it == packetmap_.end()) {
        Nodeptr tmp = std::make_shared<Node>();
        tmp->revnum_ = 1;
        memcpy(tmp->data+burstnum*9,&str[2],9);
        packetmap_.insert({packetnum,tmp});
      }
      else {
        if(((*it).second)->revnum_ < 0) return; //如果受到一个已经发送的包的burst,就丢弃不理.

        memcpy(((*it).second)->data+burstnum*9,&str[2],9);
        ((*it).second)->revnum_++;

        if(((*it).second)->revnum_ >= 0)

        if(((*it).second)->revnum_ >= threshold_) {
          std::string str(((*it).second)->data,((*it).second)->data+243);
          pmt::pmt_t ss = pmt::string_to_symbol(str);
          message_port_pub(pmt::mp("out"),ss);
          ((*it).second)->revnum_ = -1;
        }
      }
    }

  } /* namespace howto */
} /* namespace gr */

