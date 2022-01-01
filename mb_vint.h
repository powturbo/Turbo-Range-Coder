/**
    Copyright (C) powturbo 2013-2022
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

//-------------------- Variable Length Codes: Gamma Coding (1..MAX_UINT) --------------------------------------------------
#include "mb_o0.h"
/* predictor definition. 
   mg0: first bit
   mgu: length encoded in unary
   mgb: mantissa value with length as context encoded in binary
   minimum value to encode with gamma coding must be >= 1
   _gau_ : max. unary length    (= max. integer size in bits + 1)
   _gab_ : max. binary length   (= max. integer size in bits + 1) 
*/
// Order 0 
#define MBG_DEF(  _mg0_, _mgu_, _mgb_, _gau_, _gab_) mbu _mg0_, _mgu_[_gau_], _mgb_[_gau_][_gab_]  
#define MBG_INIT( _mg0_, _mgu_, _mgb_, _gau_, _gab_) mbu_init(&_mg0_, mbu_probinit()); mbu_init1(_mgu_, _gau_); mbu_init2(_mgb_, _gau_, _gab_) // predictor init
#define MBG_DEC(  _mg0_, _mgu_, _mgb_, _gau_, _gab_) MBG_DEF( _mg0_, _mgu_, _mgb_, _gau_, _gab_); MBG_INIT( _mg0_, _mgu_, _mgb_, _gau_, _gab_)  // predictor definition+init

// Order 1: context only for first bit mg0
#define MBG_DEF0( _mg0_, _g0b_, _mgu_, _mgb_, _gau_, _gab_) mbu _mg0_[_g0b_], _mgu_[_gau_], _mgb_[_gau_][_gab_]
#define MBG_INIT0(_mg0_, _g0b_, _mgu_, _mgb_, _gau_, _gab_) mbu_init1(_mg0_, _g0b_); mbu_init1(_mgu_, _gau_); mbu_init2(_mgb_, _gau_, _gab_)
#define MBG_DEC0( _mg0_, _g0b_, _mgu_, _mgb_, _gau_, _gab_) MBG_DEF0(_mg0_, _g0b_, _mgu_, _mgb_, _gau_, _gab_); MBG_INIT0(_mg0_, _g0b_, _mgu_, _mgb_, _gau_, _gab_)

// Order 1: context for mg0 + mgu
#define MBG_DEF1( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gau_, _gab_) mbu _mg0_[_g0b_], _mgu_[_g1b_][_gau_], _mgb_[_gau_][_gab_]
#define MBG_INIT1(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gau_, _gab_) mbu_init1(_mg0_, _g0b_); mbu_init2(_mgu_, _g1b_, _gau_); mbu_init2(_mgb_, _gau_, _gab_)
#define MBG_DEC1( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gau_, _gab_) MBG_DEF1( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gau_, _gab_); MBG_INIT1(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gau_, _gab_)

// Order 1: context for mg0 + mgu + mgb
#define MBG_DEF2(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_, _gau_, _gab_) mbu _mg0_[_g0b_], _mgu_[_g1b_][_gau_], _mgb_[_gbb_][_gau_][_gab_]
#define MBG_INI2(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_, _gau_, _gab_) mbu_init1(_mg0_, _g0b_); mbu_init2(_mgu_, _g1b_, _gau_); mbu_init3(_mgb_, _gbb_, _gau_, _gab_)
#define MBG_DEC2(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_, _gau_, _gab_) MBG_DEF2( _mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_, _gau_, _gab_); MBG_INI2(_mg0_, _g0b_, _mgu_, _g1b_, _mgb_, _gbb_, _gau_, _gab_)

// unary
#define _mbgue(_rcrange_,_rclow_, _mg_,_prm0_,_prm1_, _op_, _gb_) do { mbu *_mg = _mg_,*_mgp;\
  for(_mgp = _mg; _mgp < _mg+(_gb_); _mgp++)\
    mbu_enc(_rcrange_,_rclow_, _mgp,_prm0_,_prm1_,_op_, 0);\
  mbu_enc(  _rcrange_,_rclow_, _mgp,_prm0_,_prm1_,_op_, 1);\
} while(0)

#define _mbgud(_rcrange_,_rccode_, _mgu_, _prm0_,_prm1_, _ip_, _ub_) do {\
  mbu *_mgu = _mgu_;\
  for(;;) {                                                         /*read the gamma length in unary */\
    if_rc0(_rcrange_,_rccode_, mbu_p(_mgu,_prm0_),_ip_) { rcupdate0(_rcrange_,_rccode_, mbu_update0, _mgu,_prm0_,_prm1_); ++_mgu;}\
    else {                                                rcupdate1(_rcrange_,_rccode_, mbu_update1, _mgu,_prm0_,_prm1_); _ub_ = _mgu - (_mgu_); break; }\
  }\
} while(0)

// binary
#define _mbgbe(_rcrange_,_rclow_, _mg_,_prm0_,_prm1_, _op_, _x_, _gb_) do {\
  int _gi; mbu *_mga = _mg_;\
  for(_gi = (_gb_)-1; _gi >= 0; --_gi) {\
    mbu *_mg = _mga + _gi;\
    mbu_enc(_rcrange_,_rclow_, _mg,_prm0_,_prm1_,_op_, ((_x_)>> _gi) & 1);\
  }\
} while(0)

#define _mbgbd(_rcrange_,_rccode_, _mgb_, _prm0_,_prm1_,_ip_, _x_,_ub_) do {\
  mbu *_mga = _mgb_, *_mgp = _mga+(_ub_);\
  while(--_mgp >= _mga) mbu_dec(_rcrange_,_rccode_, _mgp,_prm0_,_prm1_,_ip_, _x_);\
}  while(0)

// Gamma Coding: 0..0xfffffffe 
#define mbgenc(_rcrange_,_rclow_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = (_x_)+1;    										 			    AS(_x != 0, "mbgenc: can't encode 0xffffffffu in gamma");\
  mbu *_mg0 = _mg0_;\
  if(_x == 1) mbu_enc(_rcrange_,_rclow_, _mg0,_prm0_,_prm1_,_op_, 1);           	/*value 1*/\
  else {      mbu_enc(_rcrange_,_rclow_, _mg0,_prm0_,_prm1_,_op_, 0);\
    unsigned _gb = __bsr32(_x);\
	_mbgue(_rcrange_,_rclow_, _mgu_,         _prm0_,_prm1_,_op_,    _gb-1);     	/*encode the length in unary */\
	_mbgbe(_rcrange_,_rclow_, (_mgb_)[_gb-1],_prm0_,_prm1_,_op_,_x, _gb);  			/* encode the value in binary with lengt gb as context*/\
  }\
} while(0)

#define _mbgdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_, _act0_, _act1_) {\
  unsigned _x = 1,_ub;\
  mbu *_mg0 = _mg0_;\
  if_rc0(_rcrange_,_rccode_, mbu_p(_mg0,_prm0_),_ip_) { rcupdate0(_rcrange_,_rccode_, mbu_update0,_mg0,_prm0_,_prm1_);\
	_mbgud(_rcrange_,_rccode_, _mgu_,        _prm0_,_prm1_,_ip_,    _ub);			/*get the length in unary */\
	_mbgbd(_rcrange_,_rccode_, (_mgb_)[_ub], _prm0_,_prm1_,_ip_, _x,_ub+1); 		/*get the value  in binary */\
	_x_ = _x-1; _act1_;\
  } else {                                              rcupdate1(_rcrange_,_rccode_, mbu_update1,_mg0,_prm0_,_prm1_); _x_ = _x-1; _act0_; } \
}

#define mbgdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_) _mbgdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_, ;, ;)

//---Gamma coding lower 5 bits values encoded in context ---------------------- 
#define mbgxenc(_rcrange_,_rclow_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_op_, _x_) do {\
  unsigned _x = (_x_)+1;    										 					AS(_x != 0, "mbgenc: can't encode 0xffffffffu in gamma");\
  mbu *_mg0 = _mg0_;\
  if(_x == 1) mbu_enc(_rcrange_,_rclow_, _mg0,_prm0_,_prm1_,_op_, 1); 					/*value 1*/\
  else {      mbu_enc(_rcrange_,_rclow_, _mg0,_prm0_,_prm1_,_op_, 0);\
    unsigned _gb = __bsr32(_x);\
	_mbgue(_rcrange_,_rclow_, _mgu_,_prm0_,_prm1_, _op_, _gb-1); 						 /*encode the length in unary */\
	if(_gb < 6) mbnenc(_rcrange_,_rclow_, (_mgb_)[_gb-1], _prm0_,_prm1_, _op_, _x, _gb); /*encode the value (context = gb) as integer */\
	else        _mbgbe(_rcrange_,_rclow_, (_mgb_)[_gb-1], _prm0_,_prm1_, _op_, _x, _gb); /*encode the value (context = gb) in binary */\
  }\
} while(0)
	
#define _mbgxdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_, _act0_, _act1_) {\
  unsigned _x = 1, _ub;\
  mbu *_mg0 = _mg0_;\
  if_rc0(_rcrange_,_rccode_, mbu_p(_mg0,_prm0_),_ip_) { rcupdate0(_rcrange_,_rccode_, mbu_update0,_mg0,_prm0_,_prm1_);\
	_mbgud(_rcrange_,_rccode_, _mgu_, _prm0_,_prm1_, _ip_, _ub);\
	mbu *_mbg = (_mgb_)[_ub]; _ub++; \
	if(_ub < 6) { 														                /*read the value in binary */\
	       mbndec(_rcrange_,_rccode_, _mbg, _prm0_,_prm1_,_ip_, _x,_ub); \
	} else _mbgbd(_rcrange_,_rccode_, _mbg, _prm0_,_prm1_,_ip_, _x,_ub);\
 	_x_ = _x-1; _act1_;\
  }\
  else {                                                rcupdate1(_rcrange_,_rccode_, mbu_update1, _mg0,_prm0_,_prm1_); _x_ = _x-1; _act0_; } \
}

#define mbgxdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_) _mbgxdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_, ;, ;)

//**** Limited length Gamma Coding. Full 32 bit range 0..UINT_MAX + 64 bits (expect last 64 bits value) ****----------------------------
#define mbgenc32(_rcrange_, _rclow_, _mgu_, _mgb_,_prm0_,_prm1_,_op_, _x_, _qmax_) do {\
  uint64_t _x = (uint64_t)(_x_)+1, \
           _q = __bsr64(_x), _log2m_ = _q; 												/*_x >> _log2m_;*/\
  if(_q > _qmax_) {											           					/* quotient greater than limit _qmax_*/\
    unsigned _qx = _q - _qmax_, _qb = __bsr32(_qx)+1; 		           					/* (_q - _qmax_) size in bits*/\
    _mbgue(_rcrange_,_rclow_, _mgu_,     _prm0_,_prm1_, _op_,      _qmax_+_qb); 		/* encode (_qmax_+_qb) in unary coding*/\
	_mbgbe(_rcrange_,_rclow_, (_mgb_)[0],_prm0_,_prm1_, _op_, _qx, _qb-1);  		    /* encode _qb-1 lsb bits (without the msb bit)*/\
  } else _mbgue(_rcrange_,_rclow_, _mgu_,_prm0_,_prm1_, _op_,      _q);      	        /* _q in unary coding*/\
    _mbgbe(_rcrange_,_rclow_, _mgb_[__bsr32(_q+1)+1],_prm0_,_prm1_, _op_,_x, _log2m_);  /* _log2m_ lsb bits of the remainder*/\
} while(0)

#define mbgdec32(_rcrange_,_rccode_, _mgu_,_mgb_,_prm0_,_prm1_, _ip_, _x_, _qmax_) do {\
  unsigned _q; \
  _mbgud(_rcrange_,_rccode_, _mgu_, _prm0_,_prm1_, _ip_, _q);\
  if(_q > _qmax_) { unsigned _qb = _q - _qmax_; _q = 1;                               	/* lsb bits length of the quotient */\
	if(_qb>1) {\
	  _mbgbd(_rcrange_,_rccode_, _mgb_[0], _prm0_,_prm1_,_ip_, _q,_qb-1); 	            /* decode lsb bits (msb always 1)*/\
      _q = 1<<(_qb-1) | _q;\
    }\
    _q += _qmax_;                                                                       /* build the quotient*/\
  }\
  uint64_t _x = 1; _mbgbd(_rcrange_,_rccode_, &(_mgb_)[__bsr32(_q+1)+1], _prm0_,_prm1_,_ip_, _x, _q); /* decode mantissa _q bits */\
  _x_ = _x - 1;\
} while(0)	

//-- Golomb Rice Coding : _qmax_ length limited rice coding with rice parameter _log2m_ -----------------------------------------------
#define mbrenc32(_rcrange_, _rclow_, _mgu_,_mgb_,_prm0_,_prm1_, _op_, _x_, _qmax_, _log2m_) {\
  unsigned      _x = _x_, _q = _x >> _log2m_;\
  if(_q > _qmax_) {											           		            /* quotient greater than limit _qmax_ */\
    unsigned _qx = _q - _qmax_; int _qb = __bsr32(_qx)+1; 	   				            /*AS(_qmax_+_qb+1 < 46,"mbrenc32: Fatal %d\n", _qmax_+_qb+1);*/	/* (_q - _qmax_) size in bits */\
    _mbgue(_rcrange_,_rclow_, _mgu_,      _prm0_,_prm1_, _op_, _qmax_+_qb); 		    /* encode (_qmax_+_qb) in unary coding */\
	_mbgbe(_rcrange_,_rclow_, &(_mgb_)[0],_prm0_,_prm1_, _op_, _qx, _qb-1); \
  } else _mbgue(_rcrange_,_rclow_, _mgu_, _prm0_,_prm1_, _op_, _q); 		            /* _q in unary coding*/\
  _mbgbe(_rcrange_,_rclow_, &(_mgb_)[__bsr32(_q+1)+1],_prm0_,_prm1_, _op_, _x, _log2m_);/* _log2m_ lsb bits of the remainder*/\
}

#define mbrdec32(_rcrange_, _rccode_, _mgu_,_mgb_,_prm0_,_prm1_, _ip_, _x_, _qmax_, _log2m_) {\
  unsigned      _x,_q;\
  _mbgud(_rcrange_,_rccode_, _mgu_, _prm0_,_prm1_, _ip_, _q);\
  if(_q > _qmax_) {\
    int _qb = _q - _qmax_;                                                              /* lsb bits length of the quotient */\
	_x = 1; _mbgbd(_rcrange_,_rccode_, &(_mgb_)[0], _prm0_,_prm1_,_ip_, _x, _qb-1);     /* decode lsb bits (msb always 1) */\
    _x += _qmax_;                                                                       /* build the quotient*/\
    _q  = _x;\
  } else _x = _q;\
  _mbgbd(_rcrange_,_rccode_, &(_mgb_)[__bsr32(_q+1)+1], _prm0_,_prm1_,_ip_, _x, _log2m_);/* decode mantissa _log2m_ bits */\
  _x_ = _x; \
}

//------------------------------- Turbo VLC with exponent coded in gamma range coder and mantissa in bitio --------------------------------------------------------
#define mbvenc(_rcrange_,_rclow_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_op_, _x_, _bw_, _br_,_vn_, _vb_) do { unsigned _vx = _x_;\
  if(_vx >= vlcfirst(_vn_)+_vb_) { \
    unsigned _expo, _ma, _mb;\
    vlcenc(_vx-_vb_, _vn_, _expo, _mb, _ma); \
	_vx = _expo+(_vb_); bitput(_bw_,_br_, _mb, _ma);\
  }\
  mbgenc(_rcrange_,_rclow_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_op_, _vx);\
} while(0)
	
#define mbvdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _x_, _bw_, _br_, _vn_, _vb_) {\
  unsigned _vx; \
  mbgdec(_rcrange_,_rccode_, _mg0_,_mgu_,_mgb_,_prm0_,_prm1_,_ip_, _vx);\
  if(_vx >= vlcfirst(_vn_)+_vb_) {\
    _vx -= _vb_; \
	int _mb = vlcmbits(_vx, _vn_), _ma;\
    bitget(_bw_,_br_, _mb,_ma); \
	_vx = vlcdec(_vx, _mb, _ma, _vn_)+_vb_; \
  } _x_ = _vx;\
}

//------------------------------ Structured/Segmented encoding: encode/decode small integers up to MBU3LEN -----------------------------------------
/*Example : nb0=3 bits, nb1=5 bits, nb2=8 bits
1          :  0                 1
01xxx      :  1 -   9 = 1+  8   8
000xxxxx   :  9 -  41 = 9+ 32  32
001xxxxxxxx: 42 - 296 =41+256  256*/

#define MBU3LEN(_nb0_,_nb1_,_nb2_) (1 + (1 << _nb0_) + (1 << _nb1_) + (1 << _nb2_))

#define MBU3_DEF(_mbf_, _mb0_, _nb0_, _mb1_, _nb1_, _mb2_, _nb2_) mbu _mbf_[3], _mb0_[1<<(_nb0_)], _mb1_[1<<(_nb1_)], _mb2_[1<<(_nb2_)]
#define MBU3_INI(_mbf_, _mb0_, _nb0_, _mb1_, _nb1_, _mb2_, _nb2_) {\
  mbu_init1(_mbf_, 3);\
  mbu_init1(_mb0_, (1<<(_nb0_)));\
  mbu_init1(_mb1_, (1<<(_nb1_)));\
  mbu_init1(_mb2_, (1<<(_nb2_)));\
}

/* with cxb bits context */
#define MBU3_DEF1( _mbf_, _mb0_, _nb0_, _mb1_, _nb1_, _mb2_, _nb2_, _cxb_) mbu _mbf_[_cxb_][3], _mb0_[_cxb_][1<<_nb0_], _mb1_[_cxb_][1<<_nb1_], _mb2_[_cxb_][1<<_nb2_];
#define MBU3_INI1( _mbf_, _mb0_, _nb0_, _mb1_, _nb1_, _mb2_, _nb2_, _cxb_) {\
  mbu_init2(_mbf_, _cxb_,       3     );\
  mbu_init2(_mb0_, _cxb_, (1<<(_nb0_)));\
  mbu_init2(_mb1_, _cxb_, (1<<(_nb1_)));\
  mbu_init1(_mb2_, _cxb_, (1<<(_nb2_)));\
}

#define MBU3_DEC( _mbf_, _mb0_,_nb0_, _mb1_,_nb1_, _mb2_,_nb2_)           MBU3_DEF( _mbf_, _mb0_,_nb0_, _mb1_,_nb1_, _mb2_,_nb2_);\
                                                                          MBU3_INI( _mbf_, _mb0_,_nb0_, _mb1_,_nb1_, _mb2_,_nb2_) 
#define MBU3_DEC1(_mbf_, _mb0_, _nb0_, _mb1_, _nb1_, _mb2_, _nb2_, _cxb_) MBU3_DEF1(_mbf_, _mb0_,_nb0_, _mb1_,_nb1_, _mb2_,_nb2_, _cxb_);\
                                                                          MBU3_INI1(_mbf_, _mb0_,_nb0_, _mb1_,_nb1_, _mb2_,_nb2_, _cxb_);


#define mbu3enc(_rcrange_,_rclow_, _mbf_,_mb0_,_nb0_, _mb1_,_nb1_, _mb2_,_nb2_, _prm0_,_prm1_,_op_, _x_) do { unsigned _xx = _x_;\
  if(!_xx) {                                      mbu_enc(_rcrange_,_rclow_, &_mbf_[0],_prm0_,_prm1_,_op_, 1); /*  1*/}\
  else {        _xx -= 1;                         mbu_enc(_rcrange_,_rclow_, &_mbf_[0],_prm0_,_prm1_,_op_, 0);\
    if(         _xx < (1 << _nb0_)) {             mbu_enc(    _rcrange_,_rclow_, &_mbf_[1],_prm0_,_prm1_,_op_, 1); /* 01*/\
      T3(mb,_nb0_,enc)(_rcrange_,_rclow_, _mb0_,_prm0_,_prm1_,_op_, _xx);\
    } else {   _xx -= (1 << _nb0_);               mbu_enc(    _rcrange_,_rclow_, &_mbf_[1],_prm0_,_prm1_,_op_, 0); \
      if(      _xx <  (1 << _nb1_)) {             mbu_enc(    _rcrange_,_rclow_, &_mbf_[2],_prm0_,_prm1_,_op_, 0); /*000*/\
        T3(mb,_nb1_,enc)(_rcrange_,_rclow_, _mb1_,_prm0_,_prm1_,_op_, _xx);  \
      } else { _xx -= (1 << _nb1_);               mbu_enc(    _rcrange_,_rclow_, &_mbf_[2],_prm0_,_prm1_,_op_, 1);\
        T3(mb,_nb2_,enc)(_rcrange_,_rclow_, _mb2_,_prm0_,_prm1_,_op_, _xx);                                 /*001*/\
      }\
    }\
  }\
} while(0)

#define _mbu3dec(_rcrange_,_rccode_, _mbf_,_mb0_,_nb0_,_mb1_,_nb1_,_mb2_,_nb2_,_prm0_,_prm1_,_ip_, _x_, _act0_, _act1_) do { unsigned _xx = 0;\
      mbu *_m0 = &_mbf_[0];\
  if_rc0(_rcrange_,_rccode_, mbu_p(_m0,_prm0_),_ip_) { rcupdate0(_rcrange_,_rccode_, mbu_update0,_m0,_prm0_,_prm1_);\
      mbu *_m1  = &_mbf_[1];\
    if_rc0(_rcrange_,_rccode_, mbu_p(_m1,_prm0_),_ip_) { rcupdate0(_rcrange_,_rccode_, mbu_update0,_m1,_prm0_,_prm1_);\
      mbu *_m2 = &_mbf_[2];\
      if_rc0(_rcrange_,_rccode_, mbu_p(_m2,_prm0_),_ip_) { rcupdate0(_rcrange_,_rccode_, mbu_update0,_m2,_prm0_,_prm1_); \
        T3(mb,_nb1_,dec)(_rcrange_, _rccode_, _mb1_,RCPRM0,RCPRM1,_ip_, _xx); _xx&= (1<<_nb1_)-1; _xx+=(1 << _nb0_)+1;\
      } else {                                             rcupdate1(_rcrange_,_rccode_, mbu_update1,_m2,_prm0_,_prm1_);\
        T3(mb,_nb2_,dec)(_rcrange_, _rccode_, _mb2_,RCPRM0,RCPRM1,_ip_, _xx); _xx&= (1<<_nb2_)-1; _xx+=(1 << _nb0_)+(1 << _nb1_)+1;\
      }\
    } else {                                            rcupdate1(_rcrange_,_rccode_, mbu_update1,_m1,_prm0_,_prm1_);\
        T3(mb,_nb0_,dec)(_rcrange_, _rccode_, _mb0_,RCPRM0,RCPRM1,_ip_, _xx); _xx+= 1;\
    }                                                                                  _x_ = _xx; _act1_; \
  } else {                                            rcupdate1(_rcrange_,_rccode_, mbu_update1,_m0,_prm0_,_prm1_);               /*1*/\
                                                                                       _x_ = _xx; _act0_;\
  }\
}  while(0)

#define mbu3dec(_rcrange_,_rccode_, _mbf_,_mb0_,_nb0_,_mb1_,_nb1_,_mb2_,_nb2_,_prm0_,_prm1_,_ip_, _x_) _mbu3dec(_rcrange_,_rccode_, _mbf_,_mb0_,_nb0_,_mb1_,_nb1_,_mb2_,_nb2_,_prm0_,_prm1_,_ip_, _x_, ;, ;)

