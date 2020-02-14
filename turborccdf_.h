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

static cdf_t mixin16[16][16] = {
{   0x0,0x7ff1,0x7ff2,0x7ff3,0x7ff4,0x7ff5,0x7ff6,0x7ff7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,0x7ff2,0x7ff3,0x7ff4,0x7ff5,0x7ff6,0x7ff7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,0x7ff3,0x7ff4,0x7ff5,0x7ff6,0x7ff7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,0x7ff4,0x7ff5,0x7ff6,0x7ff7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,0x7ff5,0x7ff6,0x7ff7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,0x7ff6,0x7ff7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,0x7ff7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,0x7ff8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,0x7ff9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,   0x9,0x7ffa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,   0x9,   0xa,0x7ffb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,   0x9,   0xa,   0xb,0x7ffc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,   0x9,   0xa,   0xb,   0xc,0x7ffd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,   0x9,   0xa,   0xb,   0xc,   0xd,0x7ffe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,   0x9,   0xa,   0xb,   0xc,   0xd,   0xe,0x7fff },
{   0x0,   0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x7,   0x8,   0x9,   0xa,   0xb,   0xc,   0xd,   0xe,   0xf }};

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

#define cdf4d(_rcrange_,rccode, _cdf_,_x_,_ip_) { cdflget(_rcrange_,rccode, _cdf_, 16, _x_, _ip_); cdf16upd(_cdf_,_x_); }

#define cdf8d(_rcrange_,rccode, _cdfh_, _cdfl_, _x_, _ip_) {\
  unsigned _xh,_xl;           cdf4d(_rcrange_,rccode,_cdfh_,_xh,_ip_);\
  cdf_t *_cdfl = _cdfl_[_xh]; cdf4d(_rcrange_,rccode,_cdfl, _xl, _ip_);\
 _x_ = _xh << 4| _xl;\
}

#define cdf8d2(rcrange0,rccode0,rcrange1,rccode1, _cdfh_, _cdfl_, _x_, _ip0_, _ip1_) {\
  unsigned _xh,_xl;           cdf4d(rcrange0,rccode0,_cdfh_,_xh,_ip0_);\
  cdf_t *_cdfl = _cdfl_[_xh]; cdf4d(rcrange1,rccode1,_cdfl, _xl, _ip1_);\
 _x_ = _xh << 4| _xl;\
}

/* 1:0-13   		  0-13    
// 2:14	  xxxx        14+15  = 29
// 3:15	  xxxx+xxxx   30+255 =
#define cdfe8(rcrange0,rclow0,rcrange1,rclow1, _m0_, _m1_, _m2_, _op_, _x_) { unsigned _x = _x_;\
  if(likely(_x < 14))                         {     cdf4e(rcrange0,rclow0,_m0_, _x, _op_); }\
  else if  (_x < 30) { _x -= 14;                    cdf4e(rcrange0,rclow0,_m0_, 14, _op_); cdf4e(rcrange1,rclow1,_m1_, _x, _op_); }\
  else { unsigned _y;  _x -= 30; _y = _x>>4;        cdf4e(rcrange0,rclow0,_m0_, 15, _op_); cdf4e(rcrange1,rclow1,_m1_, _y, _op_); _x &= 0xf; cdf4e(rcrange0,rclow0,_m2_, _x, _op_); }\
}*/


