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
// TurboRC: Range Coder - CDF include
  #ifdef __AVX2__
#include <immintrin.h>
  #elif defined(__AVX__)
#include <immintrin.h>
  #elif defined(__SSE4_1__)
#include <smmintrin.h>
  #elif defined(__SSSE3__)
    #ifdef __powerpc64__
#define __SSE__   1
#define __SSE2__  1
#define __SSE3__  1
#define NO_WARN_X86_INTRINSICS 1
    #endif
#include <tmmintrin.h>
  #elif defined(__SSE2__)
#include <emmintrin.h>
  #elif defined(__ARM_NEON)
#include <arm_neon.h>
#define __m128i uint32x4_t
#define _mm_loadu_si128(  _ip_)                 vld1q_u32(_ip_)
#define _mm_add_epi16(  _a_,_b_)                (__m128i)vaddq_u16((uint16x8_t)(_a_), (uint16x8_t)(_b_))
#define _mm_srai_epi16( _a_,_m_)                (__m128i)vshrq_n_s16((int16x8_t)(_a_), _m_)
#define _mm_sub_epi16(  _a_,_b_)                (__m128i)vsubq_u16((uint16x8_t)(_a_), (uint16x8_t)(_b_))
#define _mm_storeu_si128(_ip_,_a_)              vst1q_u32((__m128i *)(_ip_),_a_)
  #endif

#define MXV(_i_, _j_) _j_ + ((_j_ <= _i_)?0:(1<<RC_BITS)-16)
#define MIXIN16(_i_) { MXV(_i_,0), MXV(_i_,1), MXV(_i_, 2), MXV(_i_, 3), MXV(_i_, 4), MXV(_i_, 5), MXV(_i_, 6), MXV(_i_, 7),\
                       MXV(_i_,8), MXV(_i_,9), MXV(_i_,10), MXV(_i_,11), MXV(_i_,12), MXV(_i_,13), MXV(_i_,14), MXV(_i_,15) }

static cdf_t mixin16[16][16] = {
 MIXIN16( 0), MIXIN16( 1), MIXIN16( 2), MIXIN16( 3), MIXIN16( 4), MIXIN16( 5), MIXIN16( 6), MIXIN16( 7),
 MIXIN16( 8), MIXIN16( 9), MIXIN16(10), MIXIN16(11), MIXIN16(12), MIXIN16(13), MIXIN16(14), MIXIN16(15)
};

#define CDF16DEC0(_m_)     cdf_t _m_[17];      { int j;                          for(j = 0; j <= 16; j++)    _m_[j] = j << (RC_BITS-4); }
#define CDF16DEC1(_m_,_n_) cdf_t _m_[_n_][17]; { int i,j; for(i=0; i < _n_; i++) for(j = 0; j <= 16; j++) _m_[i][j] = j << (RC_BITS-4); }
#define CDF16DEF 
#define RATE16 7

  #ifdef __AVX2__
#define cdf16upd(_m_, _x_) {\
  __m256i _vx0 = _mm256_loadu_si256((const __m256i *)mixin16[_x_]);\
  __m256i _vm0 = _mm256_loadu_si256((const __m256i *)(_m_));\
	      _vm0 = _mm256_add_epi16(_vm0, _mm256_srai_epi16(_mm256_sub_epi16(_vx0, _vm0), RATE16));\
  _mm256_storeu_si256((const __m256i *)(_m_), _vm0);\
}
  #elif defined(__SSE2__) || defined(__ARM_NEON) || defined(__powerpc64__)
#define cdf16upd(_m_, _y_) {\
  __m128i _vx0 = _mm_loadu_si128((const __m128i *) mixin16[_y_]);\
  __m128i _vm0 = _mm_loadu_si128((const __m128i *)(_m_));\
  __m128i _vx1 = _mm_loadu_si128((const __m128i *)&mixin16[_y_][8]); \
  __m128i _vm1 = _mm_loadu_si128((const __m128i *)&(_m_)[8]);\
	_vm0 = _mm_add_epi16(_vm0, _mm_srai_epi16(_mm_sub_epi16(_vx0, _vm0), RATE16));\
	_vm1 = _mm_add_epi16(_vm1, _mm_srai_epi16(_mm_sub_epi16(_vx1, _vm1), RATE16));\
	_mm_storeu_si128((const __m128i *)( _m_),  _vm0);\
	_mm_storeu_si128((const __m128i *)&_m_[8], _vm1);\
}
  #else
#define cdf16upd(_m_, _x_) { int _i; for(_i = 0; _i < 16; _i++) _m_[_i] += (mixin16[_x_][_i] - _m_[_i]) >> RATE16; }
  #endif
//#include "turborccdfx_.h"

#define cdf4e(_rcrange_,_rclow_,_cdf_,_x_,_op_) { cdfenc(_rcrange_,_rclow_, _cdf_, _x_, _op_); cdf16upd(_cdf_,_x_); }

#define cdf8e(_rcrange_,_rclow_,_cdfh_, _cdfl_, _x_, _op_) { \
  unsigned _x = _x_, _xh = _x>>4, _xl=_x & 0xf;\
  cdf4e(_rcrange_,_rclow_, _cdfh_, _xh, _op_);\
  cdf4e(_rcrange_,_rclow_, _cdfl_[_xh], _xl, _op_);\
}

#define cdf8e2(_rcrange0_,_rclow0_,_rcrange1_,_rclow1_,_cdfh_, _cdfl_, _x_, _op0_, _op1_) {\
  unsigned _x = _x_, _xh = _x>>4, _xl=_x & 0xf;\
  cdf4e(_rcrange0_,_rclow0_, _cdfh_, _xh, _op0_);\
  cdf4e(_rcrange1_,_rclow1_, _cdfl_[_xh], _xl, _op1_);\
}

#define cdf4d(_rcrange_,_rccode_, _cdf_,_x_,_ip_) { cdflget16(_rcrange_,_rccode_, _cdf_, _x_, _ip_); cdf16upd(_cdf_,_x_); }

#define cdf8d(_rcrange_,rccode, _cdfh_, _cdfl_, _x_, _ip_) {\
  unsigned _xh,_xl;           cdf4d(_rcrange_,rccode,_cdfh_,_xh, _ip_);\
  cdf_t *_cdfl = _cdfl_[_xh]; cdf4d(_rcrange_,rccode,_cdfl, _xl, _ip_);\
 _x_ = _xh << 4| _xl;\
}

  #if 0
#define cdf8d2(rcrange0,rccode0,rcrange1,rccode1, _cdfh_, _cdfl_, _x_, _ip0_, _ip1_) {\
  unsigned _xh,_xl;           cdf4d(rcrange0,rccode0,_cdfh_,_xh, _ip0_);\
  cdf_t *_cdfl = _cdfl_[_xh]; cdf4d(rcrange1,rccode1,_cdfl, _xl, _ip1_);\
 _x_ = _xh << 4| _xl;\
}
  #else
#define cdf8d2(rcrange0,rccode0,rcrange1,rccode1, _cdfh_, _cdfl_, _x_, _ip0_, _ip1_) {\
  unsigned _xh,_xl;\
  _rccdfrange(rcrange0);\
  _rccdfrange(rcrange1);\
  _cdflget16(rcrange0,rccode0, _cdfh_, _xh);\
  cdf_t *_cdfl = _cdfl_[_xh]; \
  _cdflget16(rcrange1,rccode1, _cdfl, _xl); _x_ = _xh << 4| _xl;\
  _rccdfupdate(rcrange0,rccode0, _cdfh_[_xh], _cdfh_[_xh+1],_ip0_);\
  _rccdfupdate(rcrange1,rccode1, _cdfl[ _xl], _cdfl[ _xl+1],_ip1_);\
  cdf16upd(_cdfh_,_xh); cdf16upd(_cdfl,_xl);  \
}
  #endif

// Variable Length Coding: Integer 0-299
// 1:0-12   		   0-12
// 2:13,14 xxxx        13,14...45(13+32)
// 3:15	   xxxx+xxxx   46,47..299(255+13+32)
#define cdfe8(rcrange0,rclow0,rcrange1,rclow1, _m0_, _m1_, _m2_, _x_, _op0_, _op1_) { unsigned _x = _x_;\
  if(likely(_x < 13))   {                                        cdf4e(rcrange0,rclow0,_m0_, _x, _op0_); }\
  else if  (_x < 13+32) { _x -= 13;    unsigned _y = (_x>>4)+13; cdf4e(rcrange0,rclow0,_m0_, _y, _op0_); \
                                                _y = _x&0xf;     cdf4e(rcrange1,rclow1,_m1_, _y, _op1_); }\
  else {                  _x -= 13+32; unsigned _y = _x>>4;      cdf4e(rcrange0,rclow0,_m0_, 15, _op0_); \
                                                                 cdf4e(rcrange1,rclow1,_m1_, _y, _op1_);\
											    _y = _x&0xf;     cdf4e(rcrange0,rclow0,_m2_, _y, _op0_); }\
}

#define cdfd8(rcrange0,rccode0,rcrange1,rccode1, _m0_, _m1_, _m2_, _x_, _ip0_, _ip1_) {\
                                 cdf4d(rcrange0,rccode0,_m0_,_x_,_ip0_);\
  if(_x_ >= 13) { \
    if(_x_ != 15) { unsigned _y; cdf4d(rcrange1,rccode1,_m1_,_y, _ip1_); _x_=((_x_-13)<<4|_y)+13; }\
    else {          unsigned _y; cdf4d(rcrange1,rccode1,_m1_,_y, _ip1_); \
	                             cdf4d(rcrange0,rccode0,_m2_,_x_,_ip0_); _x_=(_y<<4|_x_)+13+32; } \
  }\
}
// 7bits: 8+127
//1: 0-8                            xxxx 
//2: 9+3bits: 9,10,11, 12,13,14,15  1xxx xxxx 
#define cdfe7(rcrange0,rclow0,rcrange1,rclow1, _m0_, _m1_, _x_, _op0_, _op1_) { unsigned _x = _x_;\
  if(likely(_x < 8))    {                  cdf4e(rcrange0,rclow0,_m0_, _x, _op0_); }\
  else { _x -= 8; unsigned _y = (_x>>4)+8; cdf4e(rcrange0,rclow0,_m0_, _y, _op0_); \
                           _y = _x&0xf;    cdf4e(rcrange1,rclow1,_m1_, _y, _op1_); }\					   
}

#define cdfd7(rcrange0,rccode0,rcrange1,rccode1, _m0_, _m1_, _x_, _ip0_, _ip1_) {\
                              cdf4d(rcrange0,rccode0,_m0_,_x_,_ip0_);\
  if(_x_ >= 8) { unsigned _y; cdf4d(rcrange1,rccode1,_m1_,_y, _ip1_); _x_=((_x_-8)<<4|_y)+8; }\
}
// 6 bits: 0-76
// 1: 0-12 
// 2: 13,14,15: bbxxxx 13 - 13+63 bits
#define cdfe6(rcrange0,rclow0,rcrange1,rclow1, _m0_, _m1_, _x_, _op0_, _op1_) { unsigned _x = _x_;\
  if(likely(_x < 12))    {                   cdf4e(rcrange0,rclow0,_m0_, _x, _op0_); }\
  else { _x -= 12; unsigned _y = (_x>>4)+12; cdf4e(rcrange0,rclow0,_m0_, _y, _op0_); \
                            _y = _x&0xf;     cdf4e(rcrange1,rclow1,_m1_, _y, _op1_); }\					   
}

#define cdfd6(rcrange0,rccode0,rcrange1,rccode1, _m0_, _m1_, _x_, _ip0_, _ip1_) {\
                               cdf4d(rcrange0,rccode0,_m0_,_x_,_ip0_);\
  if(_x_ >= 12) { unsigned _y; cdf4d(rcrange1,rccode1,_m1_,_y, _ip1_); _x_=((_x_-12)<<4|_y)+12; }\
}
