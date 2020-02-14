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
// TurboRC: Range Coder - Eugene Shelwien predictor
#define _RC_SH_
#define RC_PRED sh

  #ifdef RC_MACROS
#define mbu unsigned short
#define mbu_p(_mb_) *(_mb_) 

#define mbu_update0(_mb_, _mbp_, wr, Mw) {\
  int dp = _mbp_ + Mw - (1<<RC_BITS);/*+ (((1<<RC_BITS)-2*Mw)&(-0));*/\
      dp = (dp*wr)>>RC_BITS;\
  int  q = _mbp_ - dp + 0;\
  *_mb_  = q & ((1<<RC_BITS)-1);\
}

#define mbu_update1(_mb_, _mbp_, wr, Mw) {\
  int dp = _mbp_ + Mw-(1<<RC_BITS) + (((1<<RC_BITS)-2*Mw)&(-1));\
      dp = (dp*wr)>>RC_BITS;\
  int  q = _mbp_ - dp + 1;\
  *_mb_  = q & ((1<<RC_BITS)-1);\
}
  #else
typedef unsigned short mbu;
static inline int mbu_p(mbu *mb) { return (*mb); } // get probability 

static inline mbu_update0(mbu *_mb_, mbu _mbp_, const int wr, const int Mw) {
  int dp = _mbp_ + Mw - (1<<RC_BITS);/*+ (((1<<RC_BITS)-2*Mw)&(-0));*/
      dp = (dp*wr)>>RC_BITS;
  int  q = _mbp_ - dp + 0;
  *_mb_  = q & ((1<<RC_BITS)-1);
}

static inline mbu_update1(mbu *_mb_, mbu _mbp_, const int wr, const int Mw) {
  int dp = _mbp_ + Mw-(1<<RC_BITS) + (((1<<RC_BITS)-2*Mw)&(-1));
      dp = (dp*wr)>>RC_BITS;
  int  q = _mbp_ - dp + 1;
  *_mb_  = q & ((1<<RC_BITS)-1);
}
  #endif

#define mbu_probinit() (1<<(RC_BITS-1))            // initial probability 0.5
#define mbu_init(_m_, _p0_) { *(_m_) = _p0_; }     // Init predictor 

#include "mbc.h"
