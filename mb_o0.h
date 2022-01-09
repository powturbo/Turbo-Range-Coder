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
// TurboRC: Range Coder 

/* bitwise adaptive range coder, can be used as order 0..N */
  #if (RC_SIZE - RC_IO)/2 >= RC_BITS
#define _RCENORM1(_rcrange_,_rclow_,_op_) 
#define _RCDNORM1(_rcrange_,_rccode_,_ip_) 
  #else
#define _RCENORM1(_rcrange_,_rclow_,_op_) _rcenorm_(_rcrange_,_rclow_, _op_)
#define _RCDNORM1(_rcrange_,_rccode_,_ip_) _rcdnorm_(_rcrange_,_rccode_,_ip_)
  #endif 

  #if (RC_SIZE - RC_IO)/4 >= RC_BITS
#define _RCENORM2(_rcrange_,_rclow_,_op_) 
#define _RCDNORM2(_rcrange_,_rccode_,_ip_) 
  #else
#define _RCENORM2(_rcrange_,_rclow_,_op_) _rcenorm_(_rcrange_,_rclow_, _op_)
#define _RCDNORM2(_rcrange_,_rccode_,_ip_) _rcdnorm_(_rcrange_,_rccode_,_ip_)
  #endif
  
// encode n bits
#define mbnenc(_rcrange_,_rclow_, _mb_,_prm0_,_prm1_,_op_, _x_, _n_) do {\
  mbu *_mbn = _mb_; unsigned _xn = 1<<(_n_) | (_x_); int _in;\
  for(_in = (_n_)-1; _in >= 0; --_in) {\
    _rcenorm_(_rcrange_,_rclow_,_op_);\
	mbu *_mn = &(_mbn)[_xn>>(_in+1)]; unsigned _mnp = mbu_p(_mn,_prm0_); mbu_e(_rcrange_,_rclow_, _mnp, _mn, _prm0_,_prm1_,_op_, _xn & (1<<_in));\
  }\
} while(0)

#define mbndec(_rcrange_,_rccode_, _mb_,_prm0_,_prm1_,_ip_, _x_, _n_) { \
  mbu *_mb = _mb_; \
  int _i; unsigned _xn = 1;\
  for(_i = (_n_)-1; _i >= 0; --_i) {\
    _rcdnorm_(_rcrange_,_rccode_,_ip_);\
    mbu *_m = &_mb[_xn]; mbu_d(_rcrange_,_rccode_, mbu_p(_m,_prm0_), _m, _prm0_,_prm1_, _xn);\
  }\
  _x_ = _xn; /*BZHI32(_xn, _n_);*/\
}

#if 0 //TEST
#define mbmnenc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_, _n_, _mb1_, _mb2_, _sse2_) do {\
  unsigned _x = 1<<(_n_) | (_x_); int _i;\
  for(_i = (_n_)-1; _i >= 0; --_i) {\
	mbu *_m = &_m_[_x>>(_i+1)]; unsigned _mp = mbu_p(_m,_prm0_); mbum_enc(_rcrange_,_rclow_, _mp, _m, _prm0_,_prm1_,_op_, _x & (1<<_i), _mb1_, _mb2_, _sse2_);\
  }\
} while(0)

#define mbmndec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_, _n_, _mb1_, _mb2_, _sse2_) { \
  int _i; unsigned _x = 1;\
  for(_i = (_n_)-1; _i >= 0; --_i) {\
    mbu *_m = &_m_[_x]; mbum_dec(_rcrange_,_rccode_, mbu_p(_m,_prm0_), _m, _prm0_,_prm1_, _x, _mb1_, _mb2_, _sse2_);\
  }\
}
#endif

//-------------- 10,12,16 bits
#define mb16enc(_rcrange_,_rclow_,  _mb_,_prm0_,_prm1_,_op_, _x_)  mbnenc(_rcrange_,_rclow_,  _mb_,_prm0_,_prm1_,_op_, _x_, 16) 
#define mb16dec(_rcrange_,_rccode_, _mb_,_prm0_,_prm1_,_ip_, _x_)  mbndec(_rcrange_,_rccode_, _mb_,_prm0_,_prm1_,_ip_, _x_, 16)

#define mb12enc(_rcrange_,_rclow_,  _mb_,_prm0_,_prm1_,_op_, _x_)  mbnenc(_rcrange_,_rclow_,  _mb_,_prm0_,_prm1_,_op_, _x_, 12) 
#define mb12dec(_rcrange_,_rccode_, _mb_,_prm0_,_prm1_,_ip_, _x_)  mbndec(_rcrange_,_rccode_, _mb_,_prm0_,_prm1_,_ip_, _x_, 12)

#define mb10enc(_rcrange_,_rclow_,  _mb_,_prm0_,_prm1_,_op_, _x_)  mbnenc(_rcrange_,_rclow_,  _mb_,_prm0_,_prm1_,_op_, _x_, 10) 
#define mb10dec(_rcrange_,_rccode_, _mb_,_prm0_,_prm1_,_ip_, _x_)  mbndec(_rcrange_,_rccode_, _mb_,_prm0_,_prm1_,_ip_, _x_, 10)

//--------------- 8 bits
#define mb8enc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<8 | (_x_),_mp;\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[    1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<7));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>7]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<6));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>6]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<5));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>5]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<4));}\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>4]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>3]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>2]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb8dec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  mbu *_m = _m_,*_mx; unsigned _x = 1,_mp;                                                          _rcdnorm_(_rcrange_,_rccode_,_ip_);\
  { _mx = &_m[ 1]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _rcdnorm_(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); }\
  _x_ = (unsigned char)_x;\
}

//--------------- 7 bits
#define mb7enc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<7 | (_x_),_mp;\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[    1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<6));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>6]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<5));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>5]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<4));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>4]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>3]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>2]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb7dec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  mbu *_m = _m_,*_mx; unsigned _x = 1,_mp;                                                              _rcdnorm_(_rcrange_,_rccode_,_ip_);\
  { _mx = &_m[ 1]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _rcdnorm_(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); }\
   _x = _x_ & 0x7f;\
}

//--------------- 6 bits
#define mb6enc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<6 | (_x_),_mp;\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[    1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<5));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>5]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<4));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>4]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>3]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>2]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb6dec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  mbu *_m = _m_,*_mx; unsigned _x = 1,_mp;                                                          _rcdnorm_(_rcrange_,_rccode_,_ip_);\
  { _mx = &_m[ 1]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _rcdnorm_(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x);  }\
   _x_ = _x & 0x3f;\
}

//--------------- 5 bits
#define mb5enc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<5 | (_x_),_mp;\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[    1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<4));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>4]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>3]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>2]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb5dec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  mbu *_m = _m_,*_mx; unsigned _x = 1,_mp;                                                     		_rcdnorm_(_rcrange_,_rccode_,_ip_);\
  { _mx = &_m[ 1]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _rcdnorm_(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x);  }\
   _x_ = _x & 0x1f;\
}

//--------------- 4 bits
#define mb4enc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<4 | (_x_),_mp;\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[    1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<3));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>3]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>2]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb4dec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  mbu *_m = _m_,*_mx; unsigned _x = 1,_mp;                                              		    _rcdnorm_(_rcrange_,_rccode_,_ip_);\
  { _mx = &_m[ 1]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x);  }\
   _x_ = _x & 0xf;\
}

//--------------- 3 bits
#define mb3enc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<3 | (_x_),_mp;\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[    1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<2));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>2]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM2(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb3dec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  mbu *_m = _m_,*_mx; unsigned _x = 1,_mp;                                                       	_rcdnorm_(_rcrange_,_rccode_,_ip_);\
  { _mx = &_m[ 1]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM2(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x);  }\
   _x_ = _x & 7;\
}
            
//--------------- 2 bits
#define mb2enc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<2 | (_x_),_mp;\
  { _rcenorm_(_rcrange_,_rclow_,_op_); _mx = &_m[    1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<1));}\
  { _RCENORM1(_rcrange_,_rclow_,_op_); _mx = &_m[_x>>1]; _mp = mbu_p(_mx,_prm0_); mbu_e(_rcrange_,_rclow_, _mp, _mx, _prm0_,_prm1_,_op_, _x & (1<<0));}\
} while(0)

#define mb2dec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_,_ip_, _x_) { \
  mbu *_m = _m_,*_mx; unsigned _x = 1,_mp;                                                       	_rcdnorm_(_rcrange_,_rccode_,_ip_);\
  { _mx = &_m[ 1]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x); _RCDNORM1(_rcrange_,_rccode_,_ip_); }\
  { _mx = &_m[_x]; _mp = mbu_p(_mx,_prm0_); mbu_d(_rcrange_,_rccode_, _mp, _mx, _prm0_,_prm1_, _x);  }\
   _x_ = _x & 3;\
}
                                                                      
// Static (without predictor)
#define mbu_senc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_,_op_, _b_) do { mbu *_m = _m_; unsigned _mbp = mbu_p(_m,_prm0_); rcbenc(_rcrange_,_rclow_ , _mbp, ;, _m, 0,0,_op_, _b_); } while(0)
#define mbu_sdec(_rcrange_,_rccode_,_m_,_prm0_,_prm1_,_ip_, _x_) do { mbu *_m = _m_; unsigned _mbp = mbu_p(_m,_prm0_); rcbdec(_rcrange_,_rccode_, _mbp, ;, _m, 0,0,_ip_, _x_); } while(0)

#define mb4senc(_rcrange_,_rclow_, _m_,_prm0_,_prm1_, _op_, _x_) do {\
  mbu *_m = _m_,*_mx; unsigned _x = 1<<4 | (_x_);\
  _mx = &_m[    1]; mbu_senc(_rcrange_,_rclow_, _mx,_prm0_,_prm1_, _op_, _x & (1<<3));\
  _mx = &_m[_x>>3]; mbu_senc(_rcrange_,_rclow_, _mx,_prm0_,_prm1_, _op_, _x & (1<<2));\
  _mx = &_m[_x>>2]; mbu_senc(_rcrange_,_rclow_, _mx,_prm0_,_prm1_, _op_, _x & (1<<1));\
  _mx = &_m[_x>>1]; mbu_senc(_rcrange_,_rclow_, _mx,_prm0_,_prm1_, _op_, _x & (1<<0));\
} while(0)

// Decode char x from memory buffer in
#define mb4sdec(_rcrange_,_rccode_, _m_,_prm0_,_prm1_, _ip_, _x_) {\
  mbu *_m = _m_,*_mx; unsigned _x = 1;\
  _mx = &_m[ 1]; mbu_sdec(_rcrange_,_rccode_, _mx,_prm0_,_prm1_, _ip_, _x);\
  _mx = &_m[_x]; mbu_sdec(_rcrange_,_rccode_, _mx,_prm0_,_prm1_, _ip_, _x);\
  _mx = &_m[_x]; mbu_sdec(_rcrange_,_rccode_, _mx,_prm0_,_prm1_, _ip_, _x);\
  _mx = &_m[_x]; mbu_sdec(_rcrange_,_rccode_, _mx,_prm0_,_prm1_, _ip_, _x);\
  _x_ = _x & 0xf;\
}
