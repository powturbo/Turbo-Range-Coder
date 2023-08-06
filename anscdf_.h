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
  #endif
#include "include_/sse_neon.h"
#include "include_/conf.h"

#define ANS_BITS 15  // maximum = 15
#define RC_BITS ANS_BITS
#include "cdf_.h"

#define IOBITS         16
typedef unsigned       state_t;
typedef unsigned short io_t;
#define ANS_LBITS                   (8*(sizeof(state_t) - sizeof(io_t)) - 1)
#define ANS_LOW                     (1u << ANS_LBITS)

#define _putc(_ch_, _out_)          _out_ -= sizeof(io_t),*(io_t *)_out_ = (_ch_)
#define _getc(_in_)                 *(io_t *)_in_;_in_+=sizeof(io_t)
//------------------------------- Entropy coder -----------------------
#define eceflush(_x_, _op_)         { _op_ -= sizeof(state_t); *(state_t *)_op_ = _x_; }
#define ecdini(  _x_, _ip_)         { _x_   = *(state_t *)_ip_; _ip_ += sizeof(state_t); }
#define ecenorm(_st_,_mbp_,_out_)   if(_st_ >= (state_t)(_mbp_)<<(sizeof(state_t)*8-1-ANS_BITS)) { _putc(_st_, _out_); _st_ >>= IOBITS; }

  #ifdef __x86_64 // https://gcc.godbolt.org/z/ae5vP7EEG
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
//#define ecdnorm(_x_,_ip_) if(unlikely(_x_ < ANS_LOW)) _x_ = _x_ << 16 | ctou16(_ip_),_ip_+=2
#define ecdnorm(_x_,_ip_)  do { unsigned _c = _x_ < ANS_LOW; _x_ = _x_ << (_c<<4) | ctou16(_ip_)&(-_c);  _ip_ += 2&(-_c); } while(0)
  #endif 

typedef unsigned short mbu;

//--------------------------- Divison ------------------------------------------------
  #ifdef _DIVLUT
#define RC_MULTISYMBOL
#define RC_MACROS 
#define RC_SIZE 32             //32 bit RC only for division free w/ reciprocal multiplication
#define RC_IO   16             //#define RC_BITS 15
#define DIV_BITS (32-RC_BITS)  //=17 include division free coder
#include "turborc_.h"
  #else
#define DIVTDIV32(_st_,_pb_) _st_/_pb_	  
  #endif
#endif // ANSN__H

#define ece(_st_, _pb_, _cpb_, _out_) { \
  ecenorm(_st_,_pb_,_out_);\
  unsigned _q = DIVTDIV32(_st_,_pb_);\
  _st_ += (_q<<ANS_BITS) - _q*(_pb_) + (_cpb_);\
}

//------------------------------- Model --------------------------------------------
#define STATEDEC(_st_, _ansn_) state_t _st_[_ansn_]
#define STATEINI(_st_, _ansn_) { int _i; for(_i = 0; _i < _ansn_; _i++) _st_[_i] = ANS_LOW; }
#define STATEDEF(_st_, _ansn_) STATEDEC(_st_, _ansn_); STATEINI(_st_, _ansn_)

#define ansenc(_m_, _sti_, _ep_, _x_) { unsigned _bp = _m_[_x_], _cbp = _m_[(_x_)+1] - _bp; ece(_sti_, _cbp, _bp, _ep_); }
#define ansflush(_st_, _ep_, _ansn_) for(int _i = 0; _i < _ansn_; _i++) eceflush(_st_[_i], _ep_)
#define ansdec(_m_, _sti_, _ip_, _x_) do { cdf16sansdec(_m_, _sti_, _x_); ecdnorm(_sti_, _ip_); } while(0)

//-- encode	
#define mnenc4(_m_,_si_, _x_, _stk_)  { *_stk_++ = (unsigned)(_si_)<<(2*ANS_BITS) | _m_[_x_]<<ANS_BITS | (_m_[(_x_)+1] - _m_[_x_]); cdf16upd(_m_,_x_); } /*store probs in stack*/
#define mnenc8(_mh_, _ml_, _x_,_stk_) { /*encode 1 byte / 2x interleaved*/\
  unsigned _x = _x_, _yh = _x>>4, _yl = _x & 0xf;\
  mnenc4(_mh_, 1, _yh, _stk_);\
  mbu *_m = _ml_[_yh];\
  mnenc4(_m, 0, _yl, _stk_);\
}

#define mnenc8x2(_mh_, _ml_, _x0_,_x1_, _stk_) { /*encode 2 bytes / 4x interleaved*/\
  unsigned _x = _x0_, _yh = _x>>4, _yl = _x & 0xf; mnenc4(_mh_, 3, _yh, _stk_);\
  mbu *_m = _ml_[_yh];                             mnenc4(_m,   2, _yl, _stk_);\
           _x = _x1_, _yh = _x>>4, _yl = _x & 0xf; mnenc4(_mh_, 1, _yh, _stk_);\
      _m = _ml_[_yh];                              mnenc4(_m,   0, _yl, _stk_);\
}

#define mnenc8x2x(_mh_, _ml_, _x0_,_x1_, _stk_, cx) { mbu *_mh; /*encode 2 bytes / 4x interleaved*/\
  unsigned _x = _x0_, _yh = _x>>4, _yl = _x & 0xf; _mh = _mh_[cx]; mnenc4(_mh, 3, _yh, _stk_);  \
  mbu *_m = _ml_[cx][_yh];                                         mnenc4(_m,   2, _yl, _stk_); cx = _x0_;\
           _x = _x1_, _yh = _x>>4, _yl = _x & 0xf; _mh = _mh_[cx]; mnenc4(_mh, 1, _yh, _stk_);  \
       _m = _ml_[cx][_yh];                         _mh = _mh_[cx]; mnenc4(_m,   0, _yl, _stk_); cx = _x1_;\
}

#define mnflush(_op_,_op__,__stk_,_stk_, _ansn_) { /*process the stack, encode all probs*/\
  STATEDEF(_st, _ansn_);\
  unsigned char *_ep = _op__,_i;\
  while(_stk_ != __stk_) {\
    unsigned _si = *--_stk_, _cpb = BEXTR32(_si,ANS_BITS,ANS_BITS), _pb = BZHI32(_si,ANS_BITS);  if(_ep <= _op_+sizeof(io_t)+(_ansn_)*sizeof(_st[0])) goto ovr;\
    ece(_st[_si>>(2*ANS_BITS)], _pb, _cpb, _ep);\
  }\
  for(_i = 0; _i < _ansn_; _i++) eceflush(_st[_i], _ep);                        if(_ep <= _op_) goto ovr;\
  unsigned _l = _op__ - _ep;                                                    if(_op_ + _l >= _op__) goto ovr;\
  memmove(_op_, _ep, _l); _op_ += _l; \
}

//-- Decode
#define mndec4(_m_, _st_, _ip_, _x_) do { cdf16ansdec(_m_, _st_, _x_); ecdnorm(_st_, _ip_); } while(0)
#define mndec8(_mh_, _ml_, _st_, _ip_, _x_) {\
  unsigned _yh,_yl;\
                       cdf16ansdec(_mh_, _st_[0], _yh);\
  mbu *_m = _ml_[_yh];\
                       cdf16ansdec(_m,   _st_[1], _yl);\
  _x_ = _yh << 4| _yl;\
  ecdnorm(_st_[0], _ip_);\
  ecdnorm(_st_[1], _ip_);\
}  
 
#define mndec8x2(_mh_, _ml_, _st_, _ip_, _x0_, _x1_) {\
  unsigned _yh,_yl; \
                       cdf16ansdec(_mh_,_st_[0], _yh);\
  mbu *_m = _ml_[_yh]; cdf16ansdec(_m,  _st_[1], _yl); _x0_ = _yh << 4| _yl;\
                       cdf16ansdec(_mh_,_st_[2], _yh);\
       _m = _ml_[_yh]; cdf16ansdec(_m,  _st_[3], _yl); _x1_ = _yh << 4| _yl;\
  ecdnorm(_st_[0], _ip_);\
  ecdnorm(_st_[1], _ip_);\
  ecdnorm(_st_[2], _ip_);\
  ecdnorm(_st_[3], _ip_);\
}

#define mndec8x2x(_mh_, _ml_, _st_, _ip_, _x0_, _x1_, cx) { mbu *_mh;\
  unsigned _yh,_yl; \
                       _mh = _mh_[cx]; cdf16ansdec(_mh,_st_[0], _yh);                       \
  mbu *_m = _ml_[cx][_yh];             cdf16ansdec(_m, _st_[1], _yl); _x0_ = _yh << 4| _yl; cx = _x0_;\
                       _mh = _mh_[cx]; cdf16ansdec(_mh,_st_[2], _yh);                       \
       _m = _ml_[cx][_yh];             cdf16ansdec(_m, _st_[3], _yl); _x1_ = _yh << 4| _yl; cx = _x1_;\
  ecdnorm(_st_[0], _ip_);\
  ecdnorm(_st_[1], _ip_);\
  ecdnorm(_st_[2], _ip_);\
  ecdnorm(_st_[3], _ip_);\
}

#define mnfill(_st_, _ip_, _ansn_) {\
  unsigned _i; for(_i = 0; _i < _ansn_; _i++) { if(_st_[_i] != ANS_LOW) die("Fatal error: st=%X\n", _st_[_i]); ecdini(_st_[_i], _ip_); }\
}  

//--------------------------- Variable Length Coding ----------------------------------
//-- Integer 0-299 (3 nibbles)
// 1:0-12   		   0-12
// 2:13,14 xxxx        13,14...45(13+32)
// 3:15	   xxxx+xxxx   46,47..299(255+13+32)
#define cdfenc8(_m0_, _m1_, _m2_, _x_, _stk_) { unsigned _x = _x_;\
  if(likely(_x < 13))   {                                        mnenc4(_m0_, 0, _x, _stk_); }\
  else if  (_x < 13+32) { _x -= 13;    unsigned _y = (_x>>4)+13; mnenc4(_m0_, 0, _y, _stk_);  \
                                                _y = _x&0xf;     mnenc4(_m1_, 1, _y, _stk_); }\
  else {                  _x -= 13+32; unsigned _y = _x>>4;      mnenc4(_m0_, 0, 15, _stk_);  \
                                                                 mnenc4(_m1_, 1, _y, _stk_);  \
											    _y = _x&0xf;     mnenc4(_m2_, 0, _y, _stk_); }\
}

#define cdfdec8(_m0_, _m1_, _m2_, _st_, _ip_, _x_) {\
                                 mndec4(_m0_, _st_[0], _x_,_ip0_);\
  if(_x_ >= 13) {\
    if(_x_ != 15) { unsigned _y; mndec4(_m1_, _st_[1], _ip_, _y); _x_ = ((_x_-13)<<4|_y)+13; }\
    else {          unsigned _y; mndec4(_m1_, _st_[1], _ip_, _y);\
	                             mndec4(_m2_, _st_[0], _ip_, _x_); _x_ = (_y<<4|_x_)+13+32; }\
  }\
}

//-- 7 bits: 8+127 (2 nibbles)
//1: 0-8                            xxxx 
//2: 9+3bits: 9,10,11, 12,13,14,15  1xxx xxxx 
#define cdfenc7(_m0_, _m1_, _x_, _stk_) { unsigned _x = _x_;\
  if(likely(_x < 8))    {                  mnenc4(_m0_, 1, _x, _stk_); }\
  else { _x -= 8; unsigned _y = (_x>>4)+8; mnenc4(_m0_, 1, _y, _stk_); \
                           _y = _x&0xf;    mnenc4(_m1_, 0, _y, _stk_); }\
}

#define cdfdec7(_m0_, _m1_, _st_, _ip_, _x_) {\
                              mndec4(_m0_, _st_[0], _ip_, _x_);\
  if(_x_ >= 8) { unsigned _y; mndec4(_m1_, _st_[1], _ip_, _y); _x_ = ((_x_-8)<<4|_y)+8; }\
}


//-- 6 bits: 0-76 (2 nibbles)
// 1: 0-12 
// 2: 13,14,15: bbxxxx 13 - 13+63 bits
#define cdfenc6(_m0_, _m1_, _x_, _stk_) { unsigned _x = _x_;\
  if(likely(_x < 12))    {                   mnenc4(_m0_, 1, _x, _stk_); }\
  else { _x -= 12; unsigned _y = (_x>>4)+12; mnenc4(_m0_, 1, _y, _stk_);\
                            _y = _x&0xf;     mnenc4(_m1_, 0, _y, _stk_); }\
}

#define cdfdec6(_m0_, _m1_, _st_, _ip_, _x_) {\
                               mndec4(_m0_, _st_[0], _ip_, _x_);\
  if(_x_ >= 12) { unsigned _y; mndec4(_m1_, _st_[1], _ip_, _y ); _x_=((_x_-12)<<4|_y)+12; }\
}
