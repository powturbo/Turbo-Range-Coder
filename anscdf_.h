/**
    Copyright (C) powturbo 2013-2023
    SPDX-License-Identifier: GPL v3 License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
// TurboRANS Range Asymmetric Numeral Systems : include  
#ifndef ANSCDF__H_
#define ANSCDF__H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX2__
#include <immintrin.h>
  #elif defined(__AVX__)
#include <immintrin.h>
  #elif defined(__SSE4_1__)
#include <smmintrin.h>
  #elif defined(__SSSE3__)
#include <tmmintrin.h>
  #elif defined(__SSE2__)
#include <emmintrin.h>
  #elif defined(__ARM_NEON)
#include <arm_neon.h>
#include "include/sse2neon.h"
  #endif
#include "include_/conf.h"

#define ANS_BITS 15
#define RC_BITS ANS_BITS
#include "cdf_.h"

#define IOBITS   16
typedef unsigned state_t;
typedef unsigned short io_t;
#define ANS_LBITS                       (8*(sizeof(state_t) - sizeof(io_t)) - 1)
#define ANS_LOW                         (1u << ANS_LBITS)

#define _putc(_ch_, _out_) 	            _out_ -= sizeof(io_t),*(io_t *)_out_ = (_ch_)
#define _getc(_in_) 		            *(io_t *)_in_;_in_+=sizeof(io_t)
//------------------------------- Entropy coder -----------------------
#define eceflush(_x_, _op_) 		{ _op_ -= sizeof(state_t); *(state_t *)_op_ = _x_; }
#define ecdini(  _x_, _ip_) 		{ _x_   = *(state_t *)_ip_; _ip_ += sizeof(state_t); }
#define ecenorm(_st_,_mbp_,_out_) 	if(_st_ >= (state_t)(_mbp_)<<(sizeof(state_t)*8-1-ANS_BITS)) { _putc(_st_, _out_); _st_ >>= IOBITS; }

  #ifdef __x86_64 // https://gcc.godbolt.org/z/5fY1M6qsd
#define ecdnorm(_x_,_ip_) { asm(\
    "mov    %1,    %%edx\n\t"\
    "shl    $0x10, %%edx\n\t"\
    "movzwl (%0),  %%eax\n\t"\
    "or     %%eax, %%edx\n\t"\
    "lea    2(%0), %%rax\n\t"\
    "cmp    $0x8000,%1\n\t"\
    "cmovb  %%edx, %1\n\t"\
    "cmovb  %%rax, %0\n\t"\
     : "=r" (_ip_), "=r" (_x_)\
     : "0"  (_ip_), "1"  (_x_)\
     : "eax", "edx", "memory" );\
  }
  #elif defined(__clang__)
#define ecdnorm(_x_,_ip_) do { \
unsigned _y = _x_ << 16 | ctou16(_ip_); \
       _ip_ += (_x_ < ANS_LOW)<<1; \
        _x_ = (_x_ < ANS_LOW) ? _y : _x_;\
} while(0)
  #else
#define ecdnorm(_x_,_ip_)  do { unsigned _c = _x_ < ANS_LOW; _x_ = _x_ << (_c<<4) | ctou16(_ip_)&(-_c);  _ip_ += 2&(-_c); } while(0)
  #endif 

typedef unsigned short mbu;

//--------------------------- Divison ------------------------------------------------
#define RC_MULTISYMBOL
#define RC_MACROS 
#define RC_SIZE 32  //32 bit RC only for division free w/ reciprocal multiplication
#define RC_BITS 15
#define RC_IO   16
#define DIV_BITS (32-RC_BITS)  //=17 include division free coder
#include "turborc_.h"

#endif // ANSN__H

#define ece(_st_, _pb_, _cpb_, _out_) { \
  ecenorm(_st_,_pb_,_out_);\
  unsigned _q = DIVTDIV32(_st_,_pb_);\
  _st_ += (_q<<ANS_BITS) - _q*(_pb_) + (_cpb_);\
}

//------------------------------- Model --------------------------------------------
#define ANSN           2 
#define STATEDEF(_st_) state_t _st_[ANSN] = {ANS_LOW, ANS_LOW}

//-- encode
#define mn4enc(_m_,_y_,_si_, _stk_) {\
  unsigned _cp = _m_[_y_];                   /*AC(_stk_ + 2 <= _stk + blksize*4, "mn4enc:Fatal error");*/\
  *_stk_++ = _si_<<15 | _cp;\
  *_stk_++ = _m_[(_y_)+1] - _cp;\
  cdf16upd(_m_,_y_);\
}

#define mn8enc(_mh_, _ml_, _x_,_stk_) {\
  unsigned _x = _x_, _yh = _x>>4, _yl = _x & 0xf;\
  mn4enc(_mh_, _yh, 1, _stk_);\
  mbu *_m = _ml_[_yh];\
  mn4enc(_m, _yl, 0, _stk_);\
}

#define mnflush(_op_,_op__,__stk_,_stk_) {\
  unsigned char *_ep = _op__, _i; /*printf("stk=%d ", (_stk_ - __stk_)/sizeof(_stk[0]) );*/ \
  STATEDEF(_st);\
  while(_stk_ != __stk_) { 							if(_ep < _op_+sizeof(io_t)+ANSN*sizeof(_st[0])) { memcpy(out,in,inlen); op=out+inlen; goto end; }\
    unsigned _pb = *--_stk_, _si = *--_stk_, _cpb = _si&0x7fff;\
	ece(_st[_si>>15], _pb, _cpb, _ep);                         \
  }\
  for(_i = 0; _i < ANSN; _i++)\
    eceflush(_st[_i],_ep);\
  int l = _op__-_ep;\
  memcpy(_op_, _ep, l); \
  _op_ += l;\
}

//-- Decode
#define mn4dec(_m_,_y_,_si_, _st_) cdfansdec(_st_[_si_], _m_, _y_)
#define mn4dec0(_ip_,_m_,_y_,_si_, _st_) do { mn4dec(_m_,_y_,_si_, _st_); ecdnorm(_st_[_si_], _ip_); } while(0)
#define mn8dec(_mh_, _ml_, _x_, _st_) { \
  unsigned _yh,_yl;\
  mn4dec(_mh_,_yh,0, _st_);\
  mbu *_m = _ml_[_yh];\
  mn4dec(_m,_yl,1, _st_);\
  _x_ = _yh << 4| _yl;\
  ecdnorm(_st_[0], ip);\
  ecdnorm(_st_[1], ip);\
}

#define mnfill(_st_, _ip_) {\
  unsigned _i; for(_i = 0; _i < ANSN; _i++) { if(_st_[_i] != ANS_LOW) die("Fatal error: st=%X\n", _st_[_i]); ecdini(_st_[_i], _ip_); }\
}  
