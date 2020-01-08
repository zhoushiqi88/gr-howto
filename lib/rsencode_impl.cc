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
#include<stdio.h>

#include <gnuradio/io_signature.h>
#include "rsencode_impl.h"

namespace gr {
  namespace howto {

    rsencode::sptr
    rsencode::make(int msglen,int ecclen)
    {
      return gnuradio::get_initial_sptr
        (new rsencode_impl(msglen,ecclen));
    }

    /*
     * The private constructor
     */
    rsencode_impl::rsencode_impl(int msglen,int ecclen)
      : gr::block("rsencode",
              gr::io_signature::make(0,0,0),
              gr::io_signature::make(0,0,0)),
              msg_length(msglen),ecc_length(ecclen)
    {
      const uint8_t enc_len = msg_length+ecc_length;
      const uint8_t poly_len = ecc_length*2;

      uint8_t** memptr = &memory;
      uint16_t offset = 0;

      polynoms[0].Init(ID_MSG_IN,offset,enc_len,memptr);
      offset += enc_len;

      polynoms[1].Init(ID_MSG_OUT,offset,enc_len,memptr);
      offset += enc_len;

      for(uint8_t i = ID_GENERATOR;i < ID_MSG_E;i++) {
        polynoms[i].Init(i,offset,poly_len,memptr);
        offset += poly_len;
      }

      polynoms[5].Init(ID_MSG_E,offset,enc_len,memptr);
      offset += enc_len;

      for(uint8_t i = ID_TPOLY3;i < ID_ERR_EVAL+2;i++) {
        polynoms[i].Init(i,offset,poly_len,memptr);
        offset += poly_len;
      }

      message_port_register_out(pmt::mp("out"));

      message_port_register_in(pmt::mp("in"));

      set_msg_handler(pmt::mp("in"), boost::bind(&rsencode_impl::handle_fun,this,_1));
      
      dst = new char[msg_length];
    }

    /*
     * Our virtual destructor.
     */
    rsencode_impl::~rsencode_impl()
    {
    }


  void rsencode_impl::handle_fun(pmt::pmt_t msg) {
		std::string str(msg_length+ecc_length,'z'); //需要改@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if(pmt::is_symbol(msg)) {
			src = pmt::symbol_to_string(msg);
		}

		else {
			throw std::invalid_argument("data wrong");
			return;
		}

		rsencode_impl::Encode(src.data(),rsencode_impl::dst);
		//FILE*fp = fopen("/home/zhou/src/RS/Reed-Solomon/include/GccEncode.txt","wb");
		//fwrite(dst,msg_length+ecc_length,1,fp);  //需要改@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		pmt::pmt_t s = pmt::mp(dst,msg_length+ecc_length);  //需要改@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//std::cout << "---------------dst:    " << dst << std::endl;
		message_port_pub(pmt::mp("out"),s);

	}




    void rsencode_impl::Encode(const void* src, void* dst) {
        uint8_t* dst_ptr = (uint8_t*) dst;

        // Copying message to the output buffer
        memcpy(dst_ptr, src, msg_length * sizeof(uint8_t));

        // Calling EncodeBlock to write ecc to out[ut buffer
        EncodeBlock(src, dst_ptr+msg_length);
    }

	void rsencode_impl::EncodeBlock(const void* src, void* dst) {
        assert(msg_length + ecc_length < 256);

        /* Generator cache, it dosn't change for one template parameters */
        static uint8_t generator_cache[117+1] = {0};
        static bool    generator_cached = false;

        /* Allocating memory on stack for polynomials storage */
        uint8_t stack_memory[MSG_CNT * msg_length + POLY_CNT * ecc_length * 2];
        this->memory = stack_memory;

        const uint8_t* src_ptr = (const uint8_t*) src;
        uint8_t* dst_ptr = (uint8_t*) dst;

        Poly *msg_in  = &polynoms[ID_MSG_IN];
        Poly *msg_out = &polynoms[ID_MSG_OUT];
        Poly *gen     = &polynoms[ID_GENERATOR];

        // Weird shit, but without reseting msg_in it simply doesn't work
        msg_in->Reset();
        msg_out->Reset();

        // Using cached generator or generating new one
        if(generator_cached) {
            gen->Set(generator_cache, sizeof(generator_cache));
        } else {
            GeneratorPoly();
            memcpy(generator_cache, gen->ptr(), gen->length);
            generator_cached = true;
        }

        // Copying input message to internal polynomial
        msg_in->Set(src_ptr, msg_length);
        msg_out->Set(src_ptr, msg_length);
        msg_out->length = msg_in->length + ecc_length;

        // Here all the magic happens
        uint8_t coef = 0; // cache
        for(uint8_t i = 0; i < msg_length; i++){
            coef = msg_out->at(i);
            if(coef != 0){
                for(uint32_t j = 1; j < gen->length; j++){
                    msg_out->at(i+j) ^= gf::mul(gen->at(j), coef);
                }
            }
        }

        // Copying ECC to the output buffer
        memcpy(dst_ptr, msg_out->ptr()+msg_length, ecc_length * sizeof(uint8_t));
    }


	void rsencode_impl::GeneratorPoly() {
        Poly *gen = polynoms + ID_GENERATOR;
        gen->at(0) = 1;
        gen->length = 1;

        Poly *mulp = polynoms + ID_TPOLY1;
        Poly *temp = polynoms + ID_TPOLY2;
        mulp->length = 2;

        for(int8_t i = 0; i < ecc_length; i++){
            mulp->at(0) = 1;
            mulp->at(1) = gf::pow(2, i);

            gf::poly_mul(gen, mulp, temp);

            gen->Copy(temp);
        }
    }


  } /* namespace howto */
} /* namespace gr */

