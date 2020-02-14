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
// TurboRC: Range Coder - Predictor Encode + Decode single bit

// Encode with predictor
#define mbu_e(  rcrange,rclow,  _mbp_, _mb_,_prm0_,_prm1_,_op_, _bit_)                                   rcbe(  rcrange,rclow,  _mbp_, mbu_update0,mbu_update1, _mb_,_prm0_,_prm1_,_op_, _bit_)
// Encode with predictor + renorm
																									  
#define mbu_enc(rcrange,rclow,         _mb_,_prm0_,_prm1_,_op_, _bit_) do { unsigned _mbp = mbu_p(_mb_); rcbenc(rcrange,rclow,  _mbp,  mbu_update0,mbu_update1, _mb_,_prm0_,_prm1_,_op_, _bit_); } while(0)

// Decode with predictor
#define mbu_d(  rcrange,rccode, _mbp_, _mb_,_prm0_,_prm1_, _x_)                                          rcbd(  rcrange,rccode, _mbp_, mbu_update0,mbu_update1, _mb_,_prm0_,_prm1_, _x_)

// Decode with predictor + renorm
#define mbu_dec(rcrange,rccode, _mb_,_prm0_,_prm1_,_ip_, _x_) do { unsigned _mbp = mbu_p(_mb_);          rcbdec(rcrange,rccode, _mbp,  mbu_update0,mbu_update1, _mb_,_prm0_,_prm1_,_ip_, _x_); } while(0)

  #ifdef RC_MACROS
#define mbu_init0(_mb_) { mbu_init(_mb_, mbu_probinit()); }
#define mbu_init1(_mb_, _n_) { int _i; for(_i = 0; _i < _n_; _i++) mbu_init(&_mb_[_i], mbu_probinit()); } 
  #else
    #ifndef _MBC_H_
#define _MBC_H_	
// Init predictor value
static inline mbu_init0(mbu *mb) { mbu_init(mb, mbu_probinit()); } 

// Init order 0 predictor (1D array) 
static void mbu_init1(mbu *mb, unsigned n) {
  int i; 
  for(i = 0; i < n; i++)
    mbu_init(&mb[i], mbu_probinit()); 
} 
    #endif
  #endif

// Init order 1 predictor (2D array mb[a0][a1])
#define mbu_init2(_mb_, _a0_, _a1_) {                  \
  int _ix,_is,_iy;                                     \
  for(_ix = 0; _ix < (_a0_); _ix++)                    \
    for(_is = 0; _is < (_a1_); _is++)                  \
      mbu_init(&(_mb_)[_ix][_is], mbu_probinit());     \
}  

// Init order 2 predictor (3D array mb[a0][a1][a2])
#define mbu_init3(_mb_, _a0_, _a1_, _a2_) {                \
  int _iw,_ix,_is,_iy;                                     \
  for(_iw = 0; _iw < (_a0_); _iw++)                        \
    for(_ix = 0; _ix < (_a1_); _ix++)                      \
      for(_is = 0; _is < (_a2_); _is++)                    \
        mbu_init(&(_mb_)[_iw][_ix][_is], mbu_probinit());  \
}

