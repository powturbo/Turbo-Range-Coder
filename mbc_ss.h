/**
    Copyright (C) powturbo 2013-2020
    GPL v3 License

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
// TurboRC: Range Coder - dual predictor (two 16 bits counters) 
#define _RC_SS_
#define RC_PRED ss

  #ifndef RCPRM0
#define RCPRM0 3    //lz: 4,5  bwt:3,6
#define RCPRM1 6
  #endif  

  #ifndef _MBC_SS_H
#include "conf.h" // _PACKED
#define _MBC_SS_H
#pragma pack(1) 
typedef struct { unsigned short p,q; } _PACKED mbu;
#pragma pack()
  #endif

  #if RC_BITS < 11 || RC_BITS > 16
#error "RC_BITS must be : 11 <= RC_BITS <= 16"
  #endif

#define mbu_init(_mb_, _p0_) { (_mb_)->p = (_mb_)->q = _p0_; }
#define mbu_probinit() (1<<(RC_BITS-1))

#ifdef RC_MACROS
#define mbu_p(_mb_) (((_mb_)->p+(_mb_)->q)>>(17-RC_BITS))
#else
static inline int mbu_p(mbu *bm) { return (bm->p+bm->q)>>(17-RC_BITS); }
#endif

// Predictor update for bit 0/1
#define mbu_update0(_mb_, _mbp_, _prm0_, _prm1_)  (_mb_)->p += ((_mb_)->p^((1<<16)-1)) >> _prm0_,\
                                                  (_mb_)->q += ((_mb_)->q^((1<<16)-1)) >> _prm1_

#define mbu_update1(_mb_, _mbp_, _prm0_, _prm1_)  (_mb_)->p -= (_mb_)->p >> _prm0_,\
                                                  (_mb_)->q -= (_mb_)->q >> _prm1_

#include "mbc.h"

