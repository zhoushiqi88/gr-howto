/* -*- c++ -*- */
/* 
 * Copyright 2019 gr-howto author.
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
#include "timehop_impl.h"

namespace gr {
  namespace howto {

    timehop::sptr
    timehop::make(uint8_t netnum)
    {
      return gnuradio::get_initial_sptr
        (new timehop_impl(netnum));
    }

    /*
     * The private constructor
     */
    timehop_impl::timehop_impl(uint8_t netnum)
      : gr::block("timehop",
              gr::io_signature::make(0,0,0),
              gr::io_signature::make(0,0,0)),
              packetnum_(0),netnum_(netnum)
    {
      message_port_register_in(pmt::mp("in"));
      message_port_register_out(pmt::mp("out"));

      set_msg_handler(pmt::mp("in"),boost::bind(&timehop_impl::handle_fun,this,_1));
      netnum_ <<= 5;
    }

    /*
     * Our virtual destructor.
     */
    timehop_impl::~timehop_impl()
    {
    }

    void
    timehop_impl::handle_fun(pmt::pmt_t msg) {
      packetnum_++;
      general_burst(msg);
      general_time();
      struct timespec t1;
      struct timespec t2;
      for(int i = 0;i < 27;i++) {
        t1.tv_sec = 0;
        t1.tv_nsec = (long)times[i]*1000000000;
        nanosleep(&t1,&t2);
        pmt::pmt_t sd = pmt::string_to_symbol(bursts[i]);
        message_port_pub(pmt::mp("out"),sd);
      }
    }

    void
    timehop_impl::general_burst(pmt::pmt_t msg) {
      re_msg = pmt::symbol_to_string(msg);
      std::string tmp(11,'a');
      for(int i = 0;i < 27;i++) {
        uint8_t burst_num = i;
        uint8_t mixnum = burst_num;
        mixnum |= netnum_;
        memcpy(&tmp[0],&packetnum_,1);
        memcpy(&tmp[1],&mixnum,1);
        memcpy(&tmp[2],&re_msg[i*9],9);
        bursts.push_back(tmp);
      }
      if(packetnum_ == 255) packetnum_ = 0;
    }

    void 
    timehop_impl::general_time() {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::exponential_distribution<> d(1);

      double total = 0.0;
      for(int i = 0;i < 28;i++) {
        times[i] = d(gen);
        total += times[i];
      }

      for(int i = 0;i < 28;i++) {
        times[i] /= total;
      }
    }

    

  } /* namespace howto */
} /* namespace gr */

