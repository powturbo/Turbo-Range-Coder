/**
    Copyright (C) powturbo 2013-2023
    SPDX-License-Identifier: GPL v3 License

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
#define CDFRATE 7
#define CDFDEC0(_mb_,       _n_) cdf_t _mb_[_n_+1];        { int _j;                               for(_j = 0; _j <= _n_;  _j++)     _mb_[_j] = _j << (RC_BITS-4); }
#define CDFDEC1(_mb_,_n1_, _n0_) cdf_t _mb_[_n1_][_n0_+1]; { int _i,_j; for(_i=0; _i < _n1_; _i++) for(_j = 0; _j <= _n0_; _j++) _mb_[_i][_j] = _j << (RC_BITS-4); }
#define CDFDEC2(_mb_, _n2_, _n1_,_n0_) cdf_t _mb_[_n2_][_n1_][_n0_+1]; { int _i,_j,_k;\
  for(    _i = 0; _i <  _n2_; _i++) \
    for(  _j = 0; _j <  _n1_; _j++)\
	  for(_k = 0; _k <= _n0_; _k++) _mb_[_i][_j][_k] = _k << (RC_BITS-4);\
}

#ifndef _CDF2
#define IC   10 
#define MIXD ( ((1u<<RC_BITS)-1) & ~((1<<5)-1) )
#define STATEUPD(_mb_, _st_, _x_) (state_t)((_mb_)[_x_] - (_mb_)[--_x_]) * (_st_ >> ANS_BITS) + BZHI32(_st_, ANS_BITS) - (_mb_)[_x_]
//#define mbcheck(_mb_) for(int i=0; i < 16; i++) AC(_mb_[i+1] > _mb_[i], "Fatal probs") 
//---- CDF16 -----------------------------------------------------------------
#define CDF16DEC0(_mb_)     CDFDEC0(_mb_,      16)
#define CDF16DEC1(_mb_,_n_) CDFDEC1(_mb_, _n_, 16)
#define CDF16DEC2(_mb_,_n1_,_n0_) CDFDEC2(_mb_, _n1_,_n0_, 16)

  #ifdef __AVX2__
#define CDF16DEF __m256i _cmv = _mm256_set1_epi16(MIXD), _crv = _mm256_set_epi16(15*IC,14*IC,13*IC,12*IC,11*IC,10*IC, 9*IC, 8*IC, 7*IC, 6*IC, 5*IC, 4*IC, 3*IC, 2*IC, 1*IC, 0)
#define cdf16upd(_mb_, _x_) {\
  __m256i _mv = _mm256_loadu_si256((const __m256i *)(_mb_));\
  _mv = _mm256_add_epi16(_mv,_mm256_srai_epi16(_mm256_add_epi16(_mm256_sub_epi16(_crv,_mv),_mm256_and_si256(_mm256_cmpgt_epi16(_mv, _mm256_set1_epi16((_mb_)[_x_])),_cmv)), CDFRATE));\
  _mm256_storeu_si256((__m256i *)(_mb_), _mv);\
}

#define cdf16ansdec(_mb_, _st_, _x_) {\
  __m256i _mv = _mm256_loadu_si256((const __m256i *)(_mb_)),\
          _gv = _mm256_cmpgt_epi16(_mv, _mm256_set1_epi16(BZHI32(_st_, ANS_BITS)));\
  _x_  = ctz32(_mm256_movemask_epi8(_gv))>>1;\
  _st_ = STATEUPD((_mb_), _st_,_x_);\
  _mv  = _mm256_add_epi16(_mv,_mm256_srai_epi16(_mm256_add_epi16(_mm256_sub_epi16(_crv,_mv),_mm256_and_si256(_gv,_cmv)), CDFRATE));\
  _mm256_storeu_si256((__m256i *)(_mb_), _mv);\
}

#define cdf16sansdec(_mb_, _st_, _x_) {\
  __m256i _mv = _mm256_loadu_si256((const __m256i *)(_mb_)), \
          _gv = _mm256_cmpgt_epi16(_mv, _mm256_set1_epi16(BZHI32(_st_, ANS_BITS))); \
  _x_  = (ctz32(_mm256_movemask_epi8(_gv))>>1); \
  _st_ = STATEUPD((_mb_), _st_,_x_);\
}

  #elif defined(__SSE2__) || defined(__powerpc64__) || defined(__ARM_NEON)
#define CDF16DEF __m128i _cmv  = _mm_set1_epi16(MIXD),                              /*adaptive CDF*/\
                         _crv0 = _mm_set_epi16( 7*IC,  6*IC,  5*IC,  4*IC,  3*IC,  2*IC, 1*IC, 0   ), \
                         _crv1 = _mm_set_epi16(15*IC, 14*IC, 13*IC, 12*IC, 11*IC, 10*IC, 9*IC, 8*IC)

#define cdf16ansdec(_mb_, _st_, _x_) {\
  __m128i _mv0 = _mm_loadu_si128((const __m128i *)(_mb_)),\
          _mv1 = _mm_loadu_si128((const __m128i *)&(_mb_)[8]),\
          _sv = _mm_set1_epi16(BZHI32(_st_, ANS_BITS)),\
		  _gv0 = _mm_cmpgt_epi16(_mv0, _sv),\
		  _gv1 = _mm_cmpgt_epi16(_mv1, _sv);\
  _x_  = ctz16(_mm_movemask_epi8(_mm_packs_epi16(_gv0, _gv1))); \
  _st_ = STATEUPD(_mb_,_st_,_x_);\
  _mv0 = _mm_add_epi16(_mv0,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv0,_mv0),_mm_and_si128(_gv0,_cmv)), CDFRATE));\
  _mv1 = _mm_add_epi16(_mv1,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv1,_mv1),_mm_and_si128(_gv1,_cmv)), CDFRATE));\
  _mm_storeu_si128((const __m128i *)(_mb_),   _mv0);\
  _mm_storeu_si128((const __m128i *)&(_mb_)[8], _mv1);\
}

#define cdf16upd(_mb_, _x_) {\
  __m128i _mv0 = _mm_loadu_si128((const __m128i *)(_mb_)),\
          _mv1 = _mm_loadu_si128((const __m128i *)&(_mb_)[8]),\
          _sv  = _mm_set1_epi16((_mb_)[_x_]),\
	      _gv0 = _mm_cmpgt_epi16(_mv0, _sv),\
	      _gv1 = _mm_cmpgt_epi16(_mv1, _sv);\
  _mv0 = _mm_add_epi16(_mv0,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv0,_mv0),_mm_and_si128(_gv0,_cmv)), CDFRATE));\
  _mv1 = _mm_add_epi16(_mv1,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv1,_mv1),_mm_and_si128(_gv1,_cmv)), CDFRATE));\
  _mm_storeu_si128((const __m128i *)(_mb_),   _mv0);\
  _mm_storeu_si128((const __m128i *)&(_mb_)[8], _mv1);\
}

#define cdf16sansdec(_mb_, _st_, _x_) { /*static CDF*/\
  __m128i _mv0 = _mm_loadu_si128((const __m128i *)(_mb_)),\
          _mv1 = _mm_loadu_si128((const __m128i *)&(_mb_)[8]),\
          _sv = _mm_set1_epi16(BZHI32(_st_, ANS_BITS)),\
		  _gv0 = _mm_cmpgt_epi16(_mv0, _sv),\
		  _gv1 = _mm_cmpgt_epi16(_mv1, _sv);\
  _x_  = ctz16(_mm_movemask_epi8(_mm_packs_epi16(_gv0, _gv1))); \
  _st_ = STATEUPD(_mb_,_st_,_x_);\
}

  #elif defined(__ARM_NEON) // TODO: custom arm functions
  #else
#define CDF16DEF
#define cdf16upd(_mb_,_x_) { unsigned _i;\
  for(_i = 0; _i < 16; _i++) {\
    int _tmp = 2 - (1<<CDFRATE) + _i*IC + (32767 + (1<<CDFRATE) - 16)*(_i > _x_);\
    (_mb_)[_i] -= ((_mb_)[_i] - _tmp) >> CDFRATE;\
  }\
}

#define cdf16ansdec(_mb_, _st_, _x_) { _x_=0; while(BZHI32(_st_, ANS_BITS) >= (_mb_)[_x_]) ++_x_;\
  unsigned _s = (_mb_)[_x_--],_t=(_mb_)[_x_]; _st_ = (state_t)(_s - _t) * (_st_ >> ANS_BITS) + BZHI32(_st_, ANS_BITS) - _t; cdf16upd((_mb_),_x_);\
} 

#define cdf16sansdec(_mb_, _st_, _x_) { _x_=0; while(BZHI32(_st_, ANS_BITS) >= (_mb_)[_x_]) ++_x_; /*static CDF*/\
  unsigned _s = (_mb_)[_x_--],_t=(_mb_)[_x_]; _st_ = (state_t)(_s - _t) * (_st_ >> ANS_BITS) + BZHI32(_st_, ANS_BITS) - _t;\
} 
  #endif
 
//----- CDF8 --------------------------------------------------------------------------------------------------
#define CDF8DEC0(_mb_)     CDFDEC0(_mb_,      8)
#define CDF8DEC1(_mb_,_n_) CDFDEC1(_mb_, _n_, 8)

  #if defined(__SSE2__) || defined(__powerpc64__) || defined(__ARM_NEON)
#define CDF8DEF __m128i _cmv  = _mm_set1_epi16(MIXD),\
                        _crv0 = _mm_set_epi16( 7*IC,  6*IC,  5*IC,  4*IC,  3*IC,  2*IC, 1*IC, 0)

#define cdf8ansdec(_mb_, _st_, _x_) {\
  __m128i _mv0 = _mm_loadu_si128((const __m128i *)_mb_),\
          _sv  = _mm_set1_epi16(BZHI32(_st_, ANS_BITS)),\
		  _gv0 = _mm_cmpgt_epi16(_mv0, _sv),\
  _x_  = ctz16(_mm_movemask_epi8(_gv0))); \
  _st_ =  STATEUPD(_mb_,_st_,_x_);\
  _mv0 = _mm_add_epi16(_mv0,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv0,_mv0),_mm_and_si128(_gv0,_cmv)), CDFRATE));\
  _mm_storeu_si128((const __m128i *)(_mb_),   _mv0);\
}

#define cdf8upd(_mb_, _x_) {\
  __m128i _mv0 = _mm_loadu_si128((const __m128i *)_mb_),\
          _sv  = _mm_set1_epi16(_mb_[_x_]),\
	      _gv0 = _mm_cmpgt_epi16(_mv0, _sv),\
  _mv0 = _mm_add_epi16(_mv0,_mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_crv0,_mv0),_mm_and_si128(_gv0,_cmv)), CDFRATE));\
  _mm_storeu_si128((const __m128i *)(_mb_),   _mv0);\
}
#else
#define CDF16DEF
#define cdf8upd(_mb_,_x_) { unsigned _i; \
  for(_i = 0; _i < 8; _i++) { \
    int _tmp = 2 - (1<<CDFRATE) + _i*IC + (32767 + (1<<CDFRATE) - 16)*(_i > _x_);\
    _mb_[_i] -= (_mb_[_i] - _tmp) >> CDFRATE; \
  }\
}

#define cdf8ansdec(_mb_, _st_, _x_) { _x_=0; while(BZHI32(_st_, ANS_BITS) >= _mb_[_x_]) ++_x_;\
  unsigned _s = _mb_[_x_--],_t=_mb_[_x_]; _st_ = (state_t)(_s - _t) * (_st_ >> ANS_BITS) + BZHI32(_st_, ANS_BITS) - _t; cdf8upd(_mb_,_x_);\
} 
#endif
 
#else //----------------------------------------------------------------------------
#include "xcdf_.h"
#endif
