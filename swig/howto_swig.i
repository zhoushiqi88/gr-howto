/* -*- c++ -*- */

#define HOWTO_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "howto_swig_doc.i"

%{
#include "howto/noneblock.h"
#include "howto/rsencode.h"
#include "howto/rsdecode.h"
#include "howto/timehop.h"
#include "howto/msgpool.h"
%}


%include "howto/noneblock.h"
GR_SWIG_BLOCK_MAGIC2(howto, noneblock);
%include "howto/rsencode.h"
GR_SWIG_BLOCK_MAGIC2(howto, rsencode);

%include "howto/rsdecode.h"
GR_SWIG_BLOCK_MAGIC2(howto, rsdecode);
%include "howto/timehop.h"
GR_SWIG_BLOCK_MAGIC2(howto, timehop);
%include "howto/msgpool.h"
GR_SWIG_BLOCK_MAGIC2(howto, msgpool);
