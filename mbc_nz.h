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
// TurboRC: Range Coder - nanozip predictor 
// (https://encode.su/threads/3323-nanozip-decoder-source?p=63407&viewfull=1#post63407) 
#define _RC_NZ_
#define RC_PRED nz

  #ifndef RCPRM0
#define RCPRM0 255
  #endif
#define RCPRM1 0

  #ifndef _MBC_NZ_H
#define _MBC_NZ_H
typedef unsigned mbu;

static unsigned char rcdivlut[256] = {
255, 204, 146, 113, 93, 78, 68, 60, 53, 48, 44, 40, 37, 35, 33, 31, 
29, 27, 26, 24, 23, 22, 21, 20, 20, 19, 18, 17, 17, 16, 16, 15, 
15, 14, 14, 14, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 
10, 10, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 7, 
7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 
6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};
  #endif

  #ifdef RC_MACROS
#define mbu_p(_mb_) ((*(unsigned *)(_mb_))>>(32-RC_BITS))
  #else
static inline unsigned mbu_p(mbu *mb) { return (*(unsigned *)(_mb_))>>(32-RC_BITS); } // get probability 
  #endif

#define mbu_init(_m_, _p0_) { *(_m_) = _p0_; }     // Init predictor  
#define mbu_probinit()      0x80000000             // initial probability 0.5

#define mbu_update1(_mb_,  _mbp_, _prm0_, _prm1_) { unsigned _zu = *(_mb_),_zy =            (_zu>>9) *rcdivlut[(unsigned char)_zu]; *(_mb_) = _zu - (_zy & 0xffffff00) + ((unsigned char)_zu<_prm0_); }
#define mbu_update0(_mb_,  _mbp_, _prm0_, _prm1_) { unsigned _zu = *(_mb_),_zy = ((1<<23) - (_zu>>9))*rcdivlut[(unsigned char)_zu]; *(_mb_) = _zu + (_zy & 0xffffff00) + ((unsigned char)_zu<_prm0_); }

#include "mbc.h"

