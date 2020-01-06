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

#ifndef INCLUDED_HOWTO_RSENCODE_IMPL_H
#define INCLUDED_HOWTO_RSENCODE_IMPL_H

#include <howto/rsencode.h>
#include"poly.hpp"
#include"gf.hpp"


#define MSG_CNT 3
#define POLY_CNT 14


namespace gr {
  namespace howto {

    class rsencode_impl : public rsencode
    {
     private:
      const uint8_t msg_length;
      const uint8_t ecc_length;
      std::string src;
      char* dst;  /*需要改@@@@@@@@@@@@@@@@@@@@@@@@*/
      enum POLY_ID {
			ID_MSG_IN = 0,
			ID_MSG_OUT,
			ID_GENERATOR,
			ID_TPOLY1,
			ID_TPOLY2,

			ID_MSG_E,

			ID_TPOLY3,
			ID_TPOLY4,

			ID_SYNDROMES,
			ID_FORNEY,

			ID_ERASURES_LOC,
			ID_ERRORS_LOC,

			ID_ERASURE,
			ID_ERRORS,

			ID_COEF_POS,
			ID_ERR_EVAL,

		};

		uint8_t* memory;
		Poly polynoms[MSG_CNT+POLY_CNT];    

     public:
      rsencode_impl(int msglen,int ecclen);
      ~rsencode_impl();

      void handle_fun(pmt::pmt_t msg);
      void Encode(const void* src,void * dst);
      void EncodeBlock(const void *src,void* dst);
      void GeneratorPoly();
    };

  } // namespace howto
} // namespace gr

#endif /* INCLUDED_HOWTO_RSENCODE_IMPL_H */

