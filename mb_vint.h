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

//------------------- 2 - variable integer gamma coding (1..MAX_UINT) --------------------------------------------------

// predictor definition. 
// mg0:first bit
// mgu:length encoded in unary
// mgb:value w/ length as context encoded in binary
#define GN 32
// Order 0 
#define MBG_DEF(  _mg0_, _mgu_, _mgb_) mbu _mg0_, _mgu_[GN], _mgb_[GN][GN]  
#define MBG_INIT( _mg0_, _mgu_, _mgb_) mbu_init(&_mg0_, mbu_probinit()); mbu_init1(_mgu_, GN); mbu_init2(_mgb_, GN, GN) // predictor init
#define MBG_DEC(  _mg0_, _mgu_, _mgb_) MBG_DEF( _mg0_, _mgu_, _mgb_); MBG_INIT( _mg0_, _mgu_, _mgb_)  // predictor definition+init

// Order 1: context only for first bit mg0
#define MBG_DEF0( _mg0_, _g0b_, _mgu_, _mgb_) mbu _mg0_[_g0b_], _mgu_[GN], _mgb_[GN][GN]
#define MBG_INIT0(_mg0_, _g0b_, _mgu_, _mgb_) mbu_init1(_mg0_, _g0b_); mbu_init1(_mgu_, GN); mbu_init2(_mgb_, GN, GN)
#define MBG_DEC0( _mg0_, _g0b_, _mgu_, _mgb_) MBG_DEF0(_mg0_, _g0b_, _mgu_, _mgb_); MBG_INIT0(_mg0_, _g0b_, _mgu_, _mgb_)

// Order 1: context for mg0 + mgu
#define MBG_DEF1( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_) mbu _mg0_[_g0b_], _mgu_[_g1b_][GN], _mgb_[GN][GN]
#define MBG_INIT1(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_) mbu_init1(_mg0_, _g0b_); mbu_init2(_mgu_, _g1b_, GN); mbu_init2(_mgb_, GN, GN)
#define MBG_DEC1( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_) MBG_DEF1( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_); MBG_INIT1(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_)

// Order 1: context for mg0 + mgu+mgb
#define MBG_DEF2( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_) mbu _mg0_[_g0b_], _mgu_[_g1b_][GN], _mgb_[_gbb_][GN][GN]
#define MBG_INIT2(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_) mbu_init1(_mg0_, _g0b_); mbu_init2(_mgu_, _g1b_, GN); mbu_init3(_mgb_, _gbb_, GN, GN)
#define MBG_DEC2( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_) MBG_DEF2( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_); MBG_INIT2(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_)

#define mbgenc(rcrange,rclow, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_op_, _y) do {\
  mbu_enc(rcrange,rclow, _mg0_,_prm0_,_prm1_,_op_, _y==1);\
  if(_y != 1) {\
    unsigned _ub = 0, _gbit; \
    do { /*encode the length in unary */\
      _gbit = (_y >> ++_ub) <= 1;\
      mbu_enc(rcrange,rclow, &(_mgu_)[_ub],_prm0_,_prm1_,_op_,_gbit);\
    } while(!_gbit);\
    mbu *_mgb = (_mgb_)[_ub-1]; /* encode the value in binary */\
    int _sb;\
    for(_sb = _ub-1; _sb >= 0; --_sb) { \
      mbu *_ml = _mgb + _sb;\
      unsigned _gbit = (_y >> _sb) & 1;\
      mbu_enc(rcrange,rclow, _ml,_prm0_,_prm1_,_op_, _gbit);\
    }\
  }\
} while(0)

#define _mbgdec(rcrange,rccode, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_, _act0_, _act_) {\
  _x_= 1; \
  if_rc0(rcrange,rccode, mbu_p(_mg0_),_ip_) {    rcupdate0(rcrange,rccode, mbu_update0,_mg0_,_prm0_,_prm1_);\
    mbu *_mgl = _mgu_;\
    for(;;) { /*read the gamma length */\
      if_rc0(rcrange,rccode, mbu_p(_mgl),_ip_) { rcupdate0(rcrange,rccode, mbu_update0, _mgl,_prm0_,_prm1_); }\
      else {                                     rcupdate1(rcrange,rccode, mbu_update1, _mgl,_prm0_,_prm1_); break; }\
      ++_mgl;\
    }\
    unsigned _ub = _mgl - (_mgu_); /*read the value */\
    mbu *_mgb = _mgb_[_ub],*_mgs = _mgb+_ub;\
    do { mbu_dec(rcrange,rccode, _mgs,_prm0_,_prm1_,_ip_, _x_); } while(_mgs-- > _mgb); _act_;\
  }\
  else {                                         rcupdate1(rcrange,rccode, mbu_update1, _mg0_,_prm0_,_prm1_); _act0_; } \
}

#define mbgdec(rcrange,rccode, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_) _mbgdec(rcrange,rccode, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_, ;, ;)

//----------------------------- 2 - Encode/Decode small integers up to LEN_SYMBOLS (1..300) ------------------------------------------------------------------------------------
  #ifndef MBL_BITS0
#define MBL_BITS0 3
#define MBL_BITS1 5
#define MBL_BITS2 8
  #endif

#define LEN_SYMBOLS ((1 << MBL_BITS0) + (1 << MBL_BITS1) + (1 << MBL_BITS2) )

#define MBL_DEF( _mbf_, _mb0_, _mb1_, _mb2_, _cxsize_) mbu _mbf_[_cxsize_][3], _mb0_[_cxsize_][1<<MBL_BITS0], _mb1_[_cxsize_][1<<MBL_BITS1], _mb2_[1<<MBL_BITS2];
#define MBL_INIT(_mbf_, _mb0_, _mb1_, _mb2_, _cxsize_) {\
  mbu_init2(_mbf_, _cxsize_, 3);\
  mbu_init2(_mb0_,_cxsize_, (1<<(MBL_BITS0)));\
  mbu_init2(_mb1_,_cxsize_, (1<<(MBL_BITS1)));\
  mbu_init1(_mb2_, (1<<(MBL_BITS2)));\
}

#define mbl3enc(rcrange,rclow, _mbf_,_mb0_,_mb1_,_mb2_, _prm0_,_prm1_,_op_, _x_, _cx_) do { unsigned _xx = _x_;\
  if(!_xx) {                                          mbu_enc(rcrange,rclow, &_mbf_[_cx_][0],_prm0_,_prm1_,_op_, 1); }\
  else {       _xx -= 1;                              mbu_enc(rcrange,rclow, &_mbf_[_cx_][0],_prm0_,_prm1_,_op_, 0);\
    if(  _xx < (1 << MBL_BITS0)) {                    mbu_enc(rcrange,rclow, &_mbf_[_cx_][1],_prm0_,_prm1_,_op_, 1);\
        TEMPLATE3(mb,MBL_BITS0,enc)(rcrange,rclow, _mb0_[_cx_],_prm0_,_prm1_,_op_, _xx);\
    } else {   _xx -= (1 << MBL_BITS0);               mbu_enc(rcrange,rclow, &_mbf_[_cx_][1],_prm0_,_prm1_,_op_, 0);\
      if(_xx < (1 << MBL_BITS1)) {                    mbu_enc(rcrange,rclow, &_mbf_[_cx_][2],_prm0_,_prm1_,_op_, 1);\
        TEMPLATE3(mb,MBL_BITS1,enc)(rcrange,rclow, _mb1_[_cx_],_prm0_,_prm1_,_op_, _xx);  \
      } else {  _xx -= (1 << MBL_BITS1);               mbu_enc(rcrange,rclow, &_mbf_[_cx_][2],_prm0_,_prm1_,_op_, 0);\
        mb8enc(rcrange,rclow, _mb2_,_prm0_,_prm1_,_op_, _xx);   \
      }\
    }\
  }\
} while(0)

#define _mbl3dec(rcrange,rccode, _mbf_, _mb0_,_mb1_,_mb2_, _prm0_,_prm1_,_ip_, _v_, _cx_, _act0_, _act1_) do { _v_ = 0;\
      mbu *_m0 = &_mbf_[_cx_][0];\
  if_rc0(rcrange,rccode, mbu_p(_m0),_ip_) { rcupdate0(rcrange,rccode, mbu_update0,_m0,_prm0_,_prm1_);\
      mbu *_m1  = &_mbf_[_cx_][1];\
    if_rc0(rcrange,rccode, mbu_p(_m1),_ip_) { rcupdate0(rcrange,rccode, mbu_update0,_m1,_prm0_,_prm1_);\
      mbu *_m2 = &_mbf_[_cx_][2];\
      if_rc0(rcrange,rccode, mbu_p(_m2),_ip_) { rcupdate0(rcrange,rccode, mbu_update0,_m2,_prm0_,_prm1_); \
        mb8dec(rcrange, rccode, _mb2_,RCPRM0,RCPRM1,_ip_, _v_); _v_&= (1<<MBL_BITS2)-1; _v_+=(1 << MBL_BITS0)+(1 << MBL_BITS1)+1;\
      } else {                                  rcupdate1(rcrange,rccode, mbu_update1,_m2,_prm0_,_prm1_);\
        TEMPLATE3(mb,MBL_BITS1,dec)(rcrange, rccode, _mb1_[_cx_],RCPRM0,RCPRM1,_ip_, _v_); _v_&= (1<<MBL_BITS1)-1; _v_+=(1 << MBL_BITS0)+1;\
      }\
    } else {                                  rcupdate1(rcrange,rccode, mbu_update1,_m1,_prm0_,_prm1_);\
        TEMPLATE3(mb,MBL_BITS0,dec)(rcrange, rccode, _mb0_[_cx_],RCPRM0,RCPRM1,_ip_, _v_); _v_= (_v_&((1<<MBL_BITS0)-1))+1;\
    }                                                                                                  _act1_; \
  } else {                                  rcupdate1(rcrange,rccode, mbu_update1,_m0,_prm0_,_prm1_);\
                                                                                                       _act0_;\
  }\
}  while(0)

#define mbl3dec(rcrange,rccode, _mbf_, _mb0_,_mb1_,_mb2_, _prm0_,_prm1_,_ip_, _v_, _cx_) _mbl3dec(rcrange,rccode, _mbf_, _mb0_,_mb1_,_mb2_, _prm0_,_prm1_,_ip_, _v_, _cx_, ;, ;) 

  #ifdef EXTRC
#include "mb_vintx.h"
  #endif

