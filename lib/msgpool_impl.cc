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
    msgpool::make(int threshold)
    {
      return gnuradio::get_initial_sptr
        (new msgpool_impl(threshold));
    }

    /*
     * The private constructor
     */
    msgpool_impl::msgpool_impl(int threshold)
      : gr::block("msgpool",
              gr::io_signature::make(0,0,0),
              gr::io_signature::make(0,0,0)),
              threshold_(threshold)
    {
      message_port_register_in(pmt::mp("in"));
      message_port_register_out(pmt::mp("out"));

      set_msg_handler(pmt::mp("in"),boost::bind(&msgpool_impl::handle_fun,this,_1));

      msgnum = new int[8];
      for(int i = 0;i < 8;i++) {
        msgnum[i] = 0;
      }

      for(int i = 0;i < 8;i++) {
        Nodeptr tmp = std::make_shared<Node>(0);
        MsgList listtmp;
        listtmp.push_back(tmp);
        msgpool_.push_back(listtmp);
      }
    }

    /*
     * Our virtual destructor.
     */
    msgpool_impl::~msgpool_impl()
    {
    }

    void 
    msgpool_impl::handle_fun(pmt::pmt_t msg) {
      uint8_t netnum;
      uint8_t burstnum;
      Nodeptr tmp = std::make_shared<Node>(0);
      std::string str = pmt::symbol_to_string(msg);

      memcpy(&burstnum,&str[0],1);
      memcpy(&netnum,&str[1],1);
      tmp->key_ = burstnum;
      memcpy(&(tmp->data),&str[2],9);

      insert_node(netnum,tmp);

    }

    void 
    msgpool_impl::insert_node(uint8_t index,Nodeptr node) {
      auto it = msgpool_[index].begin();
      it++;
      while((*(*it)).key_ < node->key_ && it != msgpool_[index].end()) {
        it++;
      }
      msgpool_[index].insert(it,node);
      msgnum[index]++;

      if(msgnum[index] >= threshold_) {
        sendmsg(msgpool_[index]);
        msgpool_[index].clear();
        msgnum[index] = 0;
      }
    }

    //组合以及发送数据
    void msgpool_impl::sendmsg(MsgList msglist) {
      std::string msgtmp(243,'a');
      auto it = msglist.begin();
      it++;
      for(uint8_t i = 0;i < 27;i++) {
        if(i == (*(*it)).key_) {
          memcpy(&msgtmp[i*9],&((*(*it)).data),9);
        }
      }

      pmt::pmt_t ss = pmt::string_to_symbol(msgtmp);
      message_port_pub(pmt::mp("out"),ss);
      
    }



  } /* namespace howto */
} /* namespace gr */

