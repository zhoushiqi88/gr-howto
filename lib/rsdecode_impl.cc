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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "rsdecode_impl.h"
#include<stdio.h>

namespace gr {
  namespace howto {

    rsdecode::sptr
    rsdecode::make(int msglen,int ecclen)
    {
      return gnuradio::get_initial_sptr
        (new rsdecode_impl(msglen,ecclen));
    }

    /*
     * The private constructor
     */
    rsdecode_impl::rsdecode_impl(int msglen,int ecclen)
      : gr::block("rsdecode",
              gr::io_signature::make(0,0,0),
              gr::io_signature::make(0,0,0)),
              msg_length(msglen),ecc_length(ecclen)
    {

        const uint8_t   enc_len  = msg_length + ecc_length;
        const uint8_t   poly_len = ecc_length * 2;
        uint8_t** memptr   = &memory;
        uint16_t  offset   = 0;

        /* Initialize first six polys manually cause their amount depends on template parameters */

        polynoms[0].Init(ID_MSG_IN, offset, enc_len, memptr);
        offset += enc_len;

        polynoms[1].Init(ID_MSG_OUT, offset, enc_len, memptr);
        offset += enc_len;

        for(uint8_t i = ID_GENERATOR; i < ID_MSG_E; i++) {
            polynoms[i].Init(i, offset, poly_len, memptr);
            offset += poly_len;
        }

        polynoms[5].Init(ID_MSG_E, offset, enc_len, memptr);
        offset += enc_len;

        for(uint8_t i = ID_TPOLY3; i < ID_ERR_EVAL+2; i++) {
            polynoms[i].Init(i, offset, poly_len, memptr);
            offset += poly_len;
        }

      message_port_register_out(pmt::mp("out"));

      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"),boost::bind(&rsdecode_impl::handle_fun,this,_1));

      dst = new char[msg_length];

    }


    /*
     * Our virtual destructor.
     */
    rsdecode_impl::~rsdecode_impl()
    {
        delete[] dst;
    }

    void rsdecode_impl::handle_fun(pmt::pmt_t msg) {
      if(pmt::is_symbol(msg)) {

        src = pmt::symbol_to_string(msg);

      }

      else {
        throw std::invalid_argument("wrongs ----");
        return;
      }
      Decode(src.data(),dst);
      //std::cout << "------------"<< (char*)dst << std::endl;
      std::string s(dst,dst+msg_length);
      //std::cout << "------------"<< s << std::endl;
      pmt::pmt_t str0= pmt::string_to_symbol(s);
      message_port_pub(pmt::mp("out"),str0);
    }


    int rsdecode_impl::Decode(const void* src, void* dst, uint8_t* erase_pos, size_t erase_count) {
         const uint8_t *src_ptr = (const uint8_t*) src;
         const uint8_t *ecc_ptr = src_ptr + msg_length;

         return DecodeBlock(src, ecc_ptr, dst, erase_pos, erase_count);
     }


    int rsdecode_impl::DecodeBlock(const void* src, const void* ecc, void* dst, uint8_t* erase_pos = NULL, size_t erase_count = 0) {
        assert(msg_length + ecc_length < 256);

        const uint8_t *src_ptr = (const uint8_t*) src;
        const uint8_t *ecc_ptr = (const uint8_t*) ecc;
        uint8_t *dst_ptr = (uint8_t*) dst;

        const uint8_t src_len = msg_length + ecc_length;
        const uint8_t dst_len = msg_length;

        bool ok;

        /* Allocation memory on stack */
        uint8_t stack_memory[MSG_CNT * msg_length + POLY_CNT * ecc_length * 2];
        this->memory = stack_memory;

        Poly *msg_in  = &polynoms[ID_MSG_IN];
        Poly *msg_out = &polynoms[ID_MSG_OUT];
        Poly *epos    = &polynoms[ID_ERASURES];

        // Copying message to polynomials memory
        msg_in->Set(src_ptr, msg_length);
        msg_in->Set(ecc_ptr, ecc_length, msg_length);
        msg_out->Copy(msg_in);

        // Copying known errors to polynomial
        if(erase_pos == NULL) {
            epos->length = 0;
        } else {
            epos->Set(erase_pos, erase_count);
            for(uint8_t i = 0; i < epos->length; i++){
                msg_in->at(epos->at(i)) = 0;
            }
        }

        // Too many errors
        if(epos->length > ecc_length) return 1;

        Poly *synd   = &polynoms[ID_SYNDROMES];
        Poly *eloc   = &polynoms[ID_ERRORS_LOC];
        Poly *reloc  = &polynoms[ID_TPOLY1];
        Poly *err    = &polynoms[ID_ERRORS];
        Poly *forney = &polynoms[ID_FORNEY];

        CalcSyndromes(msg_in);

        // Checking for errors
        bool has_errors = false;
        for(uint8_t i = 0; i < synd->length; i++) {
            if(synd->at(i) != 0) {
                has_errors = true;
                break;
            }
        }

        // Going to exit if no errors
        if(!has_errors) goto return_corrected_msg;

        CalcForneySyndromes(synd, epos, src_len);
        FindErrorLocator(forney, NULL, epos->length);

        reloc->length = eloc->length;
        for(int8_t i = eloc->length-1, j = 0; i >= 0; i--, j++){
            reloc->at(j) = eloc->at(i);
        }

        // Fing errors
        ok = FindErrors(reloc, src_len);
        if(!ok) return 1;

        // Error happened while finding errors (so helpfull :D)
        if(err->length == 0) return 1;

        /* Adding found errors with known */
        for(uint8_t i = 0; i < err->length; i++) {
            epos->Append(err->at(i));
        }

        // Correcting errors
        CorrectErrata(synd, epos, msg_in);


      return_corrected_msg:
        // Wrighting corrected message to output buffer
        msg_out->length = dst_len;
        memcpy(dst_ptr, msg_out->ptr(), msg_out->length * sizeof(uint8_t));
        return 0;
    
    
    
    
    
    
    }


    void rsdecode_impl::CalcSyndromes(const Poly *msg)
     {
        Poly *synd = &polynoms[ID_SYNDROMES];
        synd->length = ecc_length+1;
        synd->at(0) = 0;
        for(uint8_t i = 1; i < ecc_length+1; i++){
            synd->at(i) = gf::poly_eval(msg, gf::pow(2, i-1));
        }
    }


    void rsdecode_impl::CalcForneySyndromes(const Poly *synd, const Poly *erasures_pos, size_t msg_in_size) {
        Poly *erase_pos_reversed = &polynoms[ID_TPOLY1];
        Poly *forney_synd = &polynoms[ID_FORNEY];
        erase_pos_reversed->length = 0;

        for(uint8_t i = 0; i < erasures_pos->length; i++){
            erase_pos_reversed->Append(msg_in_size - 1 - erasures_pos->at(i));
        }

        forney_synd->Reset();
        forney_synd->Set(synd->ptr()+1, synd->length-1);

        uint8_t x;
        for(uint8_t i = 0; i < erasures_pos->length; i++) {
            x = gf::pow(2, erase_pos_reversed->at(i));
            for(int8_t j = 0; j < forney_synd->length - 1; j++){
                forney_synd->at(j) = gf::mul(forney_synd->at(j), x) ^ forney_synd->at(j+1);
            }
        }
    }

    bool rsdecode_impl::FindErrorLocator(const Poly *synd, Poly *erase_loc = NULL, size_t erase_count = 0) {
        Poly *error_loc = &polynoms[ID_ERRORS_LOC];
        Poly *err_loc   = &polynoms[ID_TPOLY1];
        Poly *old_loc   = &polynoms[ID_TPOLY2];
        Poly *temp      = &polynoms[ID_TPOLY3];
        Poly *temp2     = &polynoms[ID_TPOLY4];

        if(erase_loc != NULL) {
            err_loc->Copy(erase_loc);
            old_loc->Copy(erase_loc);
        } else {
            err_loc->length = 1;
            old_loc->length = 1;
            err_loc->at(0)  = 1;
            old_loc->at(0)  = 1;
        }

        uint8_t synd_shift = 0;
        if(synd->length > ecc_length) {
            synd_shift = synd->length - ecc_length;
        }

        uint8_t K = 0;
        uint8_t delta = 0;
        uint8_t index;

        for(uint8_t i = 0; i < ecc_length - erase_count; i++){
            if(erase_loc != NULL)
                K = erase_count + i + synd_shift;
            else
                K = i + synd_shift;

            delta = synd->at(K);
            for(uint8_t j = 1; j < err_loc->length; j++) {
                index = err_loc->length - j - 1;
                delta ^= gf::mul(err_loc->at(index), synd->at(K-j));
            }

            old_loc->Append(0);

            if(delta != 0) {
                if(old_loc->length > err_loc->length) {
                    gf::poly_scale(old_loc, temp, delta);
                    gf::poly_scale(err_loc, old_loc, gf::inverse(delta));
                    err_loc->Copy(temp);
                }
                gf::poly_scale(old_loc, temp, delta);
                gf::poly_add(err_loc, temp, temp2);
                err_loc->Copy(temp2);
            }
        }

        uint32_t shift = 0;
        while(err_loc->length && err_loc->at(shift) == 0) shift++;

        uint32_t errs = err_loc->length - shift - 1;
        if(((errs - erase_count) * 2 + erase_count) > ecc_length){
            return false; /* Error count is greater then we can fix! */
        }

        memcpy(error_loc->ptr(), err_loc->ptr() + shift, (err_loc->length - shift) * sizeof(uint8_t));
        error_loc->length = (err_loc->length - shift);
        return true;
    }

    bool rsdecode_impl::FindErrors(const Poly *error_loc, size_t msg_in_size) {
        Poly *err = &polynoms[ID_ERRORS];

        uint8_t errs = error_loc->length - 1;
        err->length = 0;

        for(uint8_t i = 0; i < msg_in_size; i++) {
            if(gf::poly_eval(error_loc, gf::pow(2, i)) == 0) {
                err->Append(msg_in_size - 1 - i);
            }
        }

        /* Sanity check:
         * the number of err/errata positions found
         * should be exactly the same as the length of the errata locator polynomial */
        if(err->length != errs)
            /* couldn't find error locations */
            return false;
        return true;
    }



    void rsdecode_impl::CorrectErrata(const Poly *synd, const Poly *err_pos, const Poly *msg_in) {
        Poly *c_pos     = &polynoms[ID_COEF_POS];
        Poly *corrected = &polynoms[ID_MSG_OUT];
        c_pos->length = err_pos->length;

        for(uint8_t i = 0; i < err_pos->length; i++)
            c_pos->at(i) = msg_in->length - 1 - err_pos->at(i);

        /* uses t_poly 1, 2, 3, 4 */
        FindErrataLocator(c_pos);
        Poly *errata_loc = &polynoms[ID_ERASURES_LOC];

        /* reversing syndromes */
        Poly *rsynd = &polynoms[ID_TPOLY3];
        rsynd->length = synd->length;

        for(int8_t i = synd->length-1, j = 0; i >= 0; i--, j++) {
            rsynd->at(j) = synd->at(i);
        }

        /* getting reversed error evaluator polynomial */
        Poly *re_eval = &polynoms[ID_TPOLY4];

        /* uses T_POLY 1, 2 */
        FindErrorEvaluator(rsynd, errata_loc, re_eval, errata_loc->length-1);

        /* reversing it back */
        Poly *e_eval = &polynoms[ID_ERR_EVAL];
        e_eval->length = re_eval->length;
        for(int8_t i = re_eval->length-1, j = 0; i >= 0; i--, j++) {
            e_eval->at(j) = re_eval->at(i);
        }

        Poly *X = &polynoms[ID_TPOLY1]; /* this will store errors positions */
        X->length = 0;

        int16_t l;
        for(uint8_t i = 0; i < c_pos->length; i++){
            l = 255 - c_pos->at(i);
            X->Append(gf::pow(2, -l));
        }

        /* Magnitude polynomial
           Shit just got real */
        Poly *E = &polynoms[ID_MSG_E];
        E->Reset();
        E->length = msg_in->length;

        uint8_t Xi_inv;

        Poly *err_loc_prime_temp = &polynoms[ID_TPOLY2];

        uint8_t err_loc_prime;
        uint8_t y;

        for(uint8_t i = 0; i < X->length; i++){
            Xi_inv = gf::inverse(X->at(i));

            err_loc_prime_temp->length = 0;
            for(uint8_t j = 0; j < X->length; j++){
                if(j != i){
                    err_loc_prime_temp->Append(gf::sub(1, gf::mul(Xi_inv, X->at(j))));
                }
            }

            err_loc_prime = 1;
            for(uint8_t j = 0; j < err_loc_prime_temp->length; j++){
                err_loc_prime = gf::mul(err_loc_prime, err_loc_prime_temp->at(j));
            }

            y = gf::poly_eval(re_eval, Xi_inv);
            y = gf::mul(gf::pow(X->at(i), 1), y);

            E->at(err_pos->at(i)) = gf::div(y, err_loc_prime);
        }

        gf::poly_add(msg_in, E, corrected);
    }


    void rsdecode_impl::FindErrataLocator(const Poly *epos) {
        Poly *errata_loc = &polynoms[ID_ERASURES_LOC];
        Poly *mulp = &polynoms[ID_TPOLY1];
        Poly *addp = &polynoms[ID_TPOLY2];
        Poly *apol = &polynoms[ID_TPOLY3];
        Poly *temp = &polynoms[ID_TPOLY4];

        errata_loc->length = 1;
        errata_loc->at(0)  = 1;

        mulp->length = 1;
        addp->length = 2;

        for(uint8_t i = 0; i < epos->length; i++){
            mulp->at(0) = 1;
            addp->at(0) = gf::pow(2, epos->at(i));
            addp->at(1) = 0;

            gf::poly_add(mulp, addp, apol);
            gf::poly_mul(errata_loc, apol, temp);

            errata_loc->Copy(temp);
        }
    }


    void rsdecode_impl::FindErrorEvaluator(const Poly *synd, const Poly *errata_loc, Poly *dst, uint8_t ecclen) {
        Poly *mulp = &polynoms[ID_TPOLY1];
        gf::poly_mul(synd, errata_loc, mulp);

        Poly *divisor = &polynoms[ID_TPOLY2];
        divisor->length = ecclen+2;

        divisor->Reset();
        divisor->at(0) = 1;

        gf::poly_div(mulp, divisor, dst);
    }

    

    

  } /* namespace howto */
} /* namespace gr */

