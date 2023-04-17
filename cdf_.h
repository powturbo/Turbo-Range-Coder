/**
    Copyright (C) powturbo 2013-2023
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
// CDF : Cumulative distribution function for range coder + rans  
#define RATE16 7
#define CDF16DEC0(_m_)     cdf_t _m_[17];      { int j;                          for(j = 0; j <= 16; j++)    _m_[j] = j << (RC_BITS-4); }
#define CDF16DEC1(_m_,_n_) cdf_t _m_[_n_][17]; { int i,j; for(i=0; i < _n_; i++) for(j = 0; j <= 16; j++) _m_[i][j] = j << (RC_BITS-4); }

#ifndef _CDF2
#define IC 10 
#define MIXD ( ((1u<<RC_BITS)-1) & ~((1<<5)-1) )
  
  #ifdef __AVX2__
#define CDF16DEF __m256i _cmv = _mm256_set1_epi16(MIXD), _crv = _mm256_set_epi16(15*IC,14*IC,13*IC,12*IC,11*IC,10*IC, 9*IC, 8*IC, 7*IC, 6*IC, 5*IC, 4*IC, 3*IC, 2*IC, 1*IC, 0)
#define cdf16upd(_m_, _x_) {\
  __m256i _mv = _mm256_loadu_si256((const __m256i *)_m_);\
  __m256i _gv = _mm256_cmpgt_epi16(_mv, _mm256_set1_epi16(_m_[_x_]));\
  _mv = _mm256_add_epi16(_mv,_mm256_srai_epi16(_mm256_add_epi16(_mm256_sub_epi16(_crv,_mv),_mm256_and_si256(_gv,_cmv)), RATE16));\
  _mm256_storeu_si256((__m256i *)_m_, _mv);\
}

#define cdfansdec(_st_, _m_, _y_) { unsigned _xx;\
  __m256i _mv = _mm256_loadu_si256((const __m256i *)_m_), \
          _gv = _mm256_cmpgt_epi16(_mv, _mm256_set1_epi16(BZHI32(_st_, ANS_BITS))); \
  _y_  = ctz32(_mm256_movemask_epi8(_gv))>>1;\
  _st_ = (state_t)(_m_[_y_] - _m_[_y_-1]) * (_st_ >> ANS_BITS) + BZHI32(_st_, ANS_BITS) - _m_[_y_-1]; _y_--;\
  _mv = _mm256_add_epi16(_mv,_mm256_srai_epi16(_mm256_add_epi16(_mm256_sub_epi16(_crv,_mv),_mm256_and_si256(_gv,_cmv)), RATE16)); \
  _mm256_storeu_si256((__m256i *)_m_, _mv);\
}

  #elif defined(__SSE2__) || defined(__powerpc64__) || defined(__ARM_NEON)
#define CDF16DEF __m128i _cmv  = _mm_set1_epi16(MIXD),\
                         _crv0 = _mm_set_epi16( 7*IC,  6*IC,  5*IC,  4*IC,  3*IC,  2*IC, 1*IC, 0   ), \
                         _crv1 = _mm_set_epi16(15*IC, 14*IC, 13*IC, 12*IC, 11*IC, 10*IC, 9*IC, 8*IC)

#define cdfansdec(_st_, _m_, _y_) {\
  __m128i _m0 = _mm_loadu_si128((const __m128i *)_m_),\
          _m1 = _mm_loadu_si128((const __m128i *)&_m_[8]),\
          _sv = _mm_set1_epi16(BZHI32(_st_, ANS_BITS)),\
		  _g0 = _mm_cmpgt_epi16(_m0, _sv),\
		  _g1 = _mm_cmpgt_epi16(_m1, _sv);\
  _y_ = ctz16(_mm_movemask_epi8(_mm_packs_epi16(_g0, _g1)));\
  _st_ = (state_t)(_m_[_y_] - _m_[_y_-1]) * (_st_ >> ANS_BITS) + BZHI32(_st_, ANS_BITS) - _m_[_y_-1]; _y_--;\
  _m0 = _mm_add_epi16(_m0,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv0,_m0),_mm_and_si128(_g0,_cmv)), RATE16));\
  _m1 = _mm_add_epi16(_m1,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv1,_m1),_mm_and_si128(_g1,_cmv)), RATE16));\
  _mm_storeu_si128((const __m128i *)(_m_),   _m0);\
  _mm_storeu_si128((const __m128i *)&_m_[8], _m1);\
}

#define cdf16upd(_m_, _x_) {\
  __m128i _m0 = _mm_loadu_si128((const __m128i *)_m_),\
          _m1 = _mm_loadu_si128((const __m128i *)&_m_[8]),\
          _sv = _mm_set1_epi16(_m_[_x_]),\
		  _g0 = _mm_cmpgt_epi16(_m0, _sv),\
		  _g1 = _mm_cmpgt_epi16(_m1, _sv);\
  _m0 = _mm_add_epi16(_m0,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv0,_m0),_mm_and_si128(_g0,_cmv)), RATE16));\
  _m1 = _mm_add_epi16(_m1,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv1,_m1),_mm_and_si128(_g1,_cmv)), RATE16));\
  _mm_storeu_si128((const __m128i *)(_m_),   _m0);\
  _mm_storeu_si128((const __m128i *)&_m_[8], _m1);\
}
  #elif defined(__ARM_NEON) // TODO: custom arm functions
  #else
#define CDF16DEF
#define cdf16upd(_m_,_y_) { unsigned _i; \
  for(_i = 0; _i < 16; _i++) { \
    int _tmp = 2 - (1<<RATE16) + _i*IC + (32767 + (1<<RATE16) - 16)*(_i > _y_);\
    _m_[_i] -= (_m_[_i] - _tmp) >> RATE16; \
  }\
}

#define cdfansdec(_st_, _m_, _y_) { _y_=0; while(BZHI32(_st_, ANS_BITS) >= _m_[_y_]) ++_y_;\
  unsigned _s = _m_[_y_--],_t=_m_[_y_]; _st_ = (state_t)(_s - _t) * (_st_ >> ANS_BITS) + BZHI32(_st_, ANS_BITS) - _t; cdf16upd(_m_,_y_);\
}                                                                                                   
  #endif
#else //----------------------------------------------------------------------------
#include "xcdf_.h"
#endif
