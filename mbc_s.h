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
// TurboRC: Range Coder - simple predictor (16 bits counter)
#define _RC_S_
#define RC_PRED s

  #ifdef RC_MACROS
#define mbu unsigned short
#define mbu_p(_mb_) *(_mb_) 
  #else
typedef unsigned short mbu;
static inline int mbu_p(mbu *mb) { return (*mb); } // get probability 
  #endif

#define mbu_probinit() (1<<(RC_BITS-1))            // initial probability 0.5
#define mbu_init(_m_, _p0_) { *(_m_) = _p0_; }     // Init predictor 

#define mbu_update0(_mb_, _mbp_, _prm0_, _prm1_) (*(_mb_) = (_mbp_) + (((1u<<RC_BITS)- (_mbp_)) >> 5)) // Predictor update for bit 0
#define mbu_update1(_mb_, _mbp_, _prm0_, _prm1_) (*(_mb_) = (_mbp_) - ((_mbp_) >> 5))                  // Predictor update for bit 1

#include "mbc.h"

