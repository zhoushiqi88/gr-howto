/* -*- c++ -*- */
/* 
 * Copyright 2019 N.
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

#ifndef INCLUDED_HOWTO_RSDECODE_IMPL_H
#define INCLUDED_HOWTO_RSDECODE_IMPL_H

#include <howto/rsdecode.h>
#include"poly.hpp"
#include"gf.hpp"

#define MSG_CNT 3
#define POLY_CNT 14

namespace gr {
  namespace howto {

    class rsdecode_impl : public rsdecode
    {
     private:
      // Nothing to declare in this block.
      const uint8_t msg_length;
      const uint8_t ecc_length;
      std::string src;
      char* dst; //需要改@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

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

        ID_ERASURES,
        ID_ERRORS,

        ID_COEF_POS,
        ID_ERR_EVAL
      };

      uint8_t* memory;
      Poly polynoms[MSG_CNT + POLY_CNT];

     public:
      rsdecode_impl(int msglen,int ecclen);
      ~rsdecode_impl();
      
      void handle_fun(pmt::pmt_t msg);


      int DecodeBlock(const void* src, const void* ecc, void* dst, uint8_t* erase_pos, size_t erase_count);
      void CalcSyndromes(const Poly *msg);
      void CalcForneySyndromes(const Poly *synd, const Poly *erasures_pos, size_t msg_in_size);
      bool FindErrorLocator(const Poly *synd, Poly *erase_loc, size_t erase_count);
      bool FindErrors(const Poly *error_loc, size_t msg_in_size);
      void CorrectErrata(const Poly *synd, const Poly *err_pos, const Poly *msg_in);
      void FindErrataLocator(const Poly *epos);
      void FindErrorEvaluator(const Poly *synd, const Poly *errata_loc, Poly *dst, uint8_t ecclen);
      int Decode(const void* src, void* dst, uint8_t* erase_pos = NULL, size_t erase_count = 0);
    };

  } // namespace howto
} // namespace gr

#endif /* INCLUDED_HOWTO_RSDECODE_IMPL_H */

