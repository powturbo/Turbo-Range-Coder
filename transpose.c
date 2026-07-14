/**
    Copyright (C) powturbo 2013-2026
    SPDX-License-Identifier: GPL v2 License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
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
//   Nibble/Byte transpose
#ifndef ESIZE //---------------------------------- Functions -----------------------------------------------------------------
#include <string.h>
#include "include_/conf.h"
#include "include_/transpose.h"
#include "include_/bitutil_.h"

//-- 24 bits / 3 bytes (scalar only) ----------------------
#define ESIZE  3
#define STRIDE ESIZE

#define TP               tp
#include "transpose.c"

#define TP               tpz
#include "transpose.c"

#define TP               tpx
#include "transpose.c"

//-- 128 bits / 16 bytes (scalar) --------------------
#define ESIZE  16
#define STRIDE ESIZE

#define TP               tp
#include "transpose.c"

#define TP               tpz
#include "transpose.c"

#define TP               tpx
#include "transpose.c"

// ### SIMD ################################################################################################ 
#define LD128(_ip_)         _mm_loadu_si128((__m128i *)(_ip_))
#define LD256(_ip_)      _mm256_loadu_si256((__m256i *)(_ip_))
#define ST128(_op_,_v_)     _mm_storeu_si128((__m128i *)(_op_),_v_)
#define ST256(_op_,_v_)  _mm256_storeu_si256((__m128i *)(_op_),_v_)

// *** 16 bits *******************************************************
#define ESIZE  2
#define USIZE 16

#define VINI128
#define VINI256

#define VE128(_v_,_vs_)
#define VE256(_v_,_vs_)
#define VD128(_ov_,_vs_)
#define VD256(_ov_,_vs_)
#define STRIDE           ESIZE
#define TP               tp
#include "transpose.c"
#define STRIDE 4
#define TP               tp4
#include "transpose.c"

#define ISDELTA
#define VINI128          __m128i vs = _mm_setzero_si128()
#define VINI256          __m256i vs = _mm256_setzero_si256()

#define VE128(_v_,_vs_)  { __m128i _v = _v_;  _v_ =    mm_delta_epi16(_v_,_vs_); _v_ =    mm_zzage_epi16(_v_); _vs_ = _v; }
#define VE256(_v_,_vs_)  { __m256i _v = _v_;  _v_ = mm256_delta_epi16(_v_,_vs_); _v_ = mm256_zzage_epi16(_v_); _vs_ = _v; }
#define VD128(_v_,_vs_)  _v_ =    mm_zzagd_epi16(_v_); _vs_ = _v_ =    mm_scan_epi16(_v_,_vs_)
#define VD256(_v_,_vs_)  _v_ = mm256_zzagd_epi16(_v_); _vs_ = _v_ = mm256_scan_epi16(_v_,_vs_)
#define STRIDE           ESIZE
#define TP               tpz
#include "transpose.c"
#define STRIDE 4
#define TP               tp4z
#include "transpose.c"

#define VE128(_v_,_vs_)  { __m128i _v = _v_; _v_ =    mm_xore_epi16(_v_,_vs_); _vs_ = _v; }
#define VE256(_v_,_vs_)  { __m256i _v = _v_; _v_ = mm256_xore_epi16(_v_,_vs_); _vs_ = _v; }
#define VD128(_v_,_vs_)  _vs_ = _v_ = mm_xord_epi16(_v_,_vs_)
#define VD256(_v_,_vs_)  _vs_ = _v_ = mm256_xord_epi16(_v_,_vs_)
#define STRIDE           ESIZE
#define TP               tpx
#include "transpose.c"
#define STRIDE 4
#define TP               tp4x
#include "transpose.c"

// *** 32 bits ***************************************************************
#define ESIZE 4
#define USIZE 32

#undef ISDELTA
#define VINI128
#define VINI256

#define VE128(_v_,_vs_)
#define VE256(_v_,_vs_)
#define VDQ128(_v0_,_v1_,_v2_,_v3_,_vs_) 
#define VDQ256(_v0_,_v1_,_v2_,_v3_,_vs_) 
#define STRIDE ESIZE
#define TP               tp
#include "transpose.c"
#define STRIDE 8
#define TP               tp4
#include "transpose.c"

#define ISDELTA
#define VINI128          __m128i vs = _mm_setzero_si128()
#define VINI256          __m256i vs = _mm256_setzero_si256()

#define VE128(_v_,_vs_) { __m128i _v = _v_; _v_ =    mm_delta_epi32(_v_,_vs_); _v_ =    mm_zzage_epi32(_v_); _vs_ = _v; }
#define VE256(_v_,_vs_) { __m256i _v = _v_; _v_ = mm256_delta_epi32(_v_,_vs_); _v_ = mm256_zzage_epi32(_v_); _vs_ = _v; }
#define VDQ128(_v0_,_v1_,_v2_,_v3_,_vs_)    MM_SCANZQ_EPI32(_v0_,_v1_,_v2_,_v3_,_vs_)
#define VDQ256(_v0_,_v1_,_v2_,_v3_,_vs_) MM256_SCANZQ_EPI32(_v0_,_v1_,_v2_,_v3_,_vs_)
#define STRIDE ESIZE
#define TP               tpz
#include "transpose.c"
#define STRIDE 8
#define TP               tp4z
#include "transpose.c"

#define VE128(_v_,_vs_) { __m128i _v = _v_; _v_ =    mm_xore_epi32(_v_,_vs_); _vs_ = _v; }
#define VE256(_v_,_vs_) { __m256i _v = _v_; _v_ = mm256_xore_epi32(_v_,_vs_); _vs_ = _v; }
#define VDQ128(_v0_,_v1_,_v2_,_v3_,_vs_)    MM_XORDQ_EPI32(_v0_,_v1_,_v2_,_v3_,_vs_)
#define VDQ256(_v0_,_v1_,_v2_,_v3_,_vs_) MM256_XORDQ_EPI32(_v0_,_v1_,_v2_,_v3_,_vs_)
#define STRIDE ESIZE
#define TP               tpx
#include "transpose.c"
#define STRIDE 8
#define TP               tp4x
#include "transpose.c"

// *** 64 bits *********************************************************************************************
#define ESIZE 8
#define USIZE 64

#undef ISDELTA
#define VINI128
#define VINI256

#define VE128(_v_,_vs_)
#define VE256(_v_,_vs_)
#define VDQ128(_v0_,_v1_,_v2_,_v3_,_vs_) 
#define VDQ256(_v0_,_v1_,_v2_,_v3_,_vs_) 
#define STRIDE ESIZE
#define TP               tp
#include "transpose.c"
#define STRIDE 16
#define TP               tp4
#include "transpose.c"

#define ISDELTA
#define VINI128          __m128i vs =    _mm_setzero_si128()
#define VINI256          __m256i vs = _mm256_setzero_si256()

#define VE128(_v_,_vs_)  { __m128i _v = mm_delta_epi64(_v_,_vs_); _vs_ = _v_; _v_ = mm_zzage_epi64(_v); }
#define VE256(_v_,_vs_)  { __m256i _v = mm256_delta_epi64(_v_,_vs_); _vs_ = _v_; _v_ = mm256_zzage_epi64(_v); }
#define VDQ128(_v0_,_v1_,_v2_,_v3_,_vs_)    MM_SCANZQ_EPI64(_v0_,_v1_,_v2_,_v3_,_vs_)
#define VDQ256(_v0_,_v1_,_v2_,_v3_,_vs_) MM256_SCANZQ_EPI64(_v0_,_v1_,_v2_,_v3_,_vs_)
#define STRIDE ESIZE
#define TP               tpz
#include "transpose.c"
#define STRIDE 16
#define TP               tp4z
#include "transpose.c"

#define VE128(_v_,_vs_) { __m128i _v = _v_; _v_ =    mm_xore_epi64(_v_,_vs_); _vs_ = _v; }  
#define VE256(_v_,_vs_) { __m256i _v = _v_; _v_ = mm256_xore_epi64(_v_,_vs_); _vs_ = _v; } 
#define VDQ128(_v0_,_v1_,_v2_,_v3_,_vs_)    MM_XORDQ_EPI64(_v0_,_v1_,_v2_,_v3_,_vs_)
#define VDQ256(_v0_,_v1_,_v2_,_v3_,_vs_) MM256_XORDQ_EPI64(_v0_,_v1_,_v2_,_v3_,_vs_)
#define STRIDE ESIZE
#define TP               tpx
#include "transpose.c"
#define STRIDE 16
#define TP               tp4x
#include "transpose.c"

#else //*************************************************************  Templates ********************************************************************************************************

#define SIE(p,si)  (p+=stride) //faster on ARM //#define SIE(_p_,_i_)  (_p_+ _i_*stride)
#define SID(p,i)   (p+=stride) //#define SID(_p_,_i_)  (_p_+ _i_*stride)

  #if defined(__AVX2__)
    #ifdef VINI256
void T3(TP, enc256v, ESIZE)(unsigned char *in, unsigned n, unsigned char *out) {
  unsigned      v = n&~(ESIZE*32-1);
  unsigned      stride = v/STRIDE;
  unsigned char *op,*ip;
  VINI256;
    #if ESIZE == 2
  __m256i sf = _mm256_set_epi8( 15, 13, 11, 9, 7, 5, 3, 1,
                                14, 12, 10, 8, 6, 4, 2, 0,
                                15, 13, 11, 9, 7, 5, 3, 1,
                                14, 12, 10, 8, 6, 4, 2, 0);
  __m256i sv0 = _mm256_set_epi8(15, 13, 11, 9,
                                 7,  5,  3, 1,
                                14, 12, 10, 8,
                                 6,  4,  2, 0,
                                15, 13, 11, 9,
                                 7,  5,  3, 1,
                                14, 12, 10, 8,
                                 6,  4,  2, 0);
  __m256i sv1 = _mm256_set_epi8(14, 12, 10, 8,
                                 6,  4,  2, 0,
                                15, 13, 11, 9,
                                 7,  5,  3, 1,
                                14, 12, 10, 8,
                                 6,  4,  2, 0,
                                15, 13, 11, 9,
                                 7,  5,  3, 1);
   #else
  __m256i pv = _mm256_set_epi32( 7, 3, 6, 2, 5, 1, 4, 0),
      #if ESIZE == 4
          sv0 = _mm256_set_epi8(15, 11, 7, 3,
                                13,  9, 5, 1,
                                14, 10, 6, 2,
                                12,  8, 4, 0,
                                15, 11, 7, 3,
                                13,  9, 5, 1,
                                14, 10, 6, 2,
                                12,  8, 4, 0),
           sv1= _mm256_set_epi8(13,  9, 5, 1,
                                15, 11, 7, 3,
                                12,  8, 4, 0,
                                14, 10, 6, 2,
                                13,  9, 5, 1,
                                15, 11, 7, 3,
                                12,  8, 4, 0,
                                14, 10, 6, 2);
      #else
           sf = _mm256_set_epi8(15,  7,
                                14,  6,
                                13,  5,
                                12,  4,
                                11,  3,
                                10,  2,
                                 9,  1,
                                 8,  0,
                                15,  7,
                                14,  6,
                                13,  5,
                                12,  4,
                                11,  3,
                                10,  2,
                                 9,  1,
                                 8,  0 ),
           tv = _mm256_set_epi8(15, 14, 11, 10, 13, 12,  9,  8,
                                 7,  6,  3,  2,  5,  4,  1,  0,
                                15, 14, 11, 10, 13, 12,  9,  8,
                                 7,  6,  3,  2,  5,  4,  1,  0);
      #endif
    #endif
    #if STRIDE > ESIZE // ------------------ byte transpose ----------------------------------
  __m256i cl = _mm256_set1_epi8( 0x0f),
          ch = _mm256_set1_epi8( 0xf0),
          cb = _mm256_set1_epi16(0xff);
    #endif

  for(ip = in,op = out; ip != in+v; ip += ESIZE*32, op += ESIZE*32/STRIDE) {
    unsigned char *p = op;                                                      PREFETCH(ip+ESIZE*192,0);
    __m256i iv0, iv1, ov0, ov1;
	  #if ESIZE >= 2
    __m256i iv2, iv3, ov2, ov3;
	    #if ESIZE > 4
    __m256i iv4, iv5, iv6, iv7, ov4, ov5, ov6, ov7;
	    #endif
	#endif
      #if   ESIZE == 2
    ov0 = LD256((__m256i *) ip    ); VE256(ov0,vs); ov0 = _mm256_shuffle_epi8(ov0, sv0);
    ov1 = LD256((__m256i *)(ip+32)); VE256(ov1,vs); ov1 = _mm256_shuffle_epi8(ov1, sv1);
    iv0 = _mm256_permute4x64_epi64(_mm256_blend_epi32(ov0, ov1,0b11001100),_MM_SHUFFLE(3, 1, 2, 0));
    iv1 = _mm256_blend_epi32(ov0, ov1,0b00110011);
    iv1 = _mm256_permute4x64_epi64(_mm256_shuffle_epi32(iv1,_MM_SHUFFLE(1, 0, 3, 2)),_MM_SHUFFLE(3, 1, 2, 0));
      #elif ESIZE == 4
    iv0 = LD256((__m256i *) ip    ); VE256(iv0,vs); iv0 = _mm256_shuffle_epi8(iv0, sv0);
    iv1 = LD256((__m256i *)(ip+32)); VE256(iv1,vs); iv1 = _mm256_shuffle_epi8(iv1, sv1);
    iv2 = LD256((__m256i *)(ip+64)); VE256(iv2,vs); iv2 = _mm256_shuffle_epi8(iv2, sv0);
    iv3 = LD256((__m256i *)(ip+96)); VE256(iv3,vs); iv3 = _mm256_shuffle_epi8(iv3, sv1);

    ov0 = _mm256_blend_epi32(iv0, iv1,0b10101010);
    ov1 = _mm256_shuffle_epi32(_mm256_blend_epi32(iv0, iv1,0b01010101),_MM_SHUFFLE(2, 3, 0, 1));
    ov2 = _mm256_blend_epi32(iv2, iv3,0b10101010);
    ov3 = _mm256_shuffle_epi32(_mm256_blend_epi32(iv2, iv3,0b01010101),_MM_SHUFFLE(2, 3, 0, 1));

    iv0 = _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(ov0, ov2), pv);
    iv1 = _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(ov0, ov2), pv);
    iv2 = _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(ov1, ov3), pv);
    iv3 = _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(ov1, ov3), pv);
      #else
    ov0 = LD256((__m256i *) ip    ); VE256(ov0,vs); ov0 = _mm256_shuffle_epi8(ov0, sf);
    ov1 = LD256((__m256i *)(ip+32)); VE256(ov1,vs); ov1 = _mm256_shuffle_epi8(ov1, sf);
    ov2 = LD256((__m256i *)(ip+64)); VE256(ov2,vs); ov2 = _mm256_shuffle_epi8(ov2, sf);
    ov3 = LD256((__m256i *)(ip+96)); VE256(ov3,vs); ov3 = _mm256_shuffle_epi8(ov3, sf);

    iv0 = _mm256_unpacklo_epi16(ov0, ov1); iv1 = _mm256_unpackhi_epi16(ov0, ov1);
    iv2 = _mm256_unpacklo_epi16(ov2, ov3); iv3 = _mm256_unpackhi_epi16(ov2, ov3);

    ov0 = _mm256_unpacklo_epi32(iv0, iv2); ov1 = _mm256_unpackhi_epi32(iv0, iv2);
    ov2 = _mm256_unpacklo_epi32(iv1, iv3); ov3 = _mm256_unpackhi_epi32(iv1, iv3);


    ov4 = LD256((__m256i *)(ip+128)); VE256(ov4,vs); ov4 = _mm256_shuffle_epi8(ov4, sf);
    ov5 = LD256((__m256i *)(ip+160)); VE256(ov5,vs); ov5 = _mm256_shuffle_epi8(ov5, sf);
    ov6 = LD256((__m256i *)(ip+192)); VE256(ov6,vs); ov6 = _mm256_shuffle_epi8(ov6, sf);
    ov7 = LD256((__m256i *)(ip+224)); VE256(ov7,vs); ov7 = _mm256_shuffle_epi8(ov7, sf);

    iv4 = _mm256_unpacklo_epi16(ov4, ov5); iv5 = _mm256_unpackhi_epi16(ov4, ov5);
    iv6 = _mm256_unpacklo_epi16(ov6, ov7); iv7 = _mm256_unpackhi_epi16(ov6, ov7);

    ov4 = _mm256_unpacklo_epi32(iv4, iv6); ov5 = _mm256_unpackhi_epi32(iv4, iv6);
    ov6 = _mm256_unpacklo_epi32(iv5, iv7); ov7 = _mm256_unpackhi_epi32(iv5, iv7);

    iv0 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(ov0, ov4), pv), tv);
    iv1 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(ov0, ov4), pv), tv);
    iv2 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(ov1, ov5), pv), tv);
    iv3 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(ov1, ov5), pv), tv);

    iv4 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(ov2, ov6), pv), tv);
    iv5 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(ov2, ov6), pv), tv);
    iv6 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(ov3, ov7), pv), tv);
    iv7 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(ov3, ov7), pv), tv);
      #endif

      #if STRIDE <= ESIZE
    _mm256_storeu_si256((__m256i *) p,          iv0);
    _mm256_storeu_si256((__m256i *)(p+=stride), iv1);
        #if ESIZE > 2
    _mm256_storeu_si256((__m256i *)(p+=stride), iv2);
    _mm256_storeu_si256((__m256i *)(p+=stride), iv3);
          #if ESIZE > 4
    _mm256_storeu_si256((__m256i *)(p+=stride), iv4);
    _mm256_storeu_si256((__m256i *)(p+=stride), iv5);
    _mm256_storeu_si256((__m256i *)(p+=stride), iv6);
    _mm256_storeu_si256((__m256i *)(p+=stride), iv7);
          #endif
        #endif

      #else //---------------------- Nibble Transpose ------------------------
    #define mm256_packus_epi16(a, b) _mm256_permute4x64_epi64(_mm256_packus_epi16(a, b), _MM_SHUFFLE(3, 1, 2, 0))
    #define ST128(_p_,_v_,_i_) _mm_storeu_si128((__m128i *)SIE(_p_,_i_), _mm256_castsi256_si128(_v_))
    #define ST1280(_p_,_v_)    _mm_storeu_si128((__m128i *)(_p_), _mm256_castsi256_si128(_v_))

    ov0 = _mm256_and_si256(iv0, cl);                      ov0 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov0,4), ov0),cb); ov0 = mm256_packus_epi16(ov0, _mm256_srli_si256( ov0,2));
    ov1 = _mm256_srli_epi16(_mm256_and_si256(iv0, ch),4); ov1 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov1,4), ov1),cb); ov1 = mm256_packus_epi16(ov1, _mm256_srli_si256( ov1,2));
    ov2 = _mm256_and_si256(iv1, cl);                      ov2 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov2,4), ov2),cb); ov2 = mm256_packus_epi16(ov2, _mm256_srli_si256( ov2,2));
    ov3 = _mm256_srli_epi16(_mm256_and_si256(iv1, ch),4); ov3 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov3,4), ov3),cb); ov3 = mm256_packus_epi16(ov3, _mm256_srli_si256( ov3,2));
    ST1280(p,ov0);  ST128(p,ov1,1); ST128(p,ov2,2); ST128(p,ov3,3);
        #if ESIZE > 2
    ov0 = _mm256_and_si256(iv2, cl);                      ov0 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov0,4), ov0),cb); ov0 = mm256_packus_epi16(ov0, _mm256_srli_si256( ov0,2));
    ov1 = _mm256_srli_epi16(_mm256_and_si256(iv2, ch),4); ov1 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov1,4), ov1),cb); ov1 = mm256_packus_epi16(ov1, _mm256_srli_si256( ov1,2));
    ov2 = _mm256_and_si256(iv3, cl);                      ov2 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov2,4), ov2),cb); ov2 = mm256_packus_epi16(ov2, _mm256_srli_si256( ov2,2));
    ov3 = _mm256_srli_epi16(_mm256_and_si256(iv3, ch),4); ov3 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov3,4), ov3),cb); ov3 = mm256_packus_epi16(ov3, _mm256_srli_si256( ov3,2));
    ST128(p,ov0,4); ST128(p,ov1,5); ST128(p,ov2,6); ST128(p,ov3,7);
          #if ESIZE > 4
    ov0 = _mm256_and_si256(iv4, cl);                      ov0 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov0,4), ov0),cb); ov0 = mm256_packus_epi16(ov0, _mm256_srli_si256(  ov0,2));
    ov1 = _mm256_srli_epi16(_mm256_and_si256(iv4, ch),4); ov1 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov1,4), ov1),cb); ov1 = mm256_packus_epi16(ov1, _mm256_srli_si256(  ov1,2));
    ov2 = _mm256_and_si256(iv5, cl);                      ov2 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov2,4), ov2),cb); ov2 = mm256_packus_epi16(ov2, _mm256_srli_si256(  ov2,2));
    ov3 = _mm256_srli_epi16(_mm256_and_si256(iv5, ch),4); ov3 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov3,4), ov3),cb); ov3 = mm256_packus_epi16(ov3, _mm256_srli_si256(  ov3,2));
    ST128(p,ov0,8); ST128(p,ov1,9); ST128(p,ov2,10); ST128(p,ov3,11);

    ov0 = _mm256_and_si256(iv6, cl);                      ov0 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov0,4), ov0),cb); ov0 =  mm256_packus_epi16(ov0, _mm256_srli_si256(  ov0,2));
    ov1 = _mm256_srli_epi16(_mm256_and_si256(iv6, ch),4); ov1 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov1,4), ov1),cb); ov1 =  mm256_packus_epi16(ov1, _mm256_srli_si256(  ov1,2));
    ov2 = _mm256_and_si256(iv7, cl);                      ov2 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov2,4), ov2),cb); ov2 =  mm256_packus_epi16(ov2, _mm256_srli_si256(  ov2,2));
    ov3 = _mm256_srli_epi16(_mm256_and_si256(iv7, ch),4); ov3 = _mm256_and_si256(_mm256_or_si256(_mm256_srli_epi16(ov3,4), ov3),cb); ov3 =  mm256_packus_epi16(ov3, _mm256_srli_si256(  ov3,2));
    ST128(p,ov0,12); ST128(p,ov1,13); ST128(p,ov2,14); ST128(p,ov3,15);
          #endif
        #endif
      #endif
  }
  T2(tpenc,ESIZE)(in+v, n-v, out+v);
}
    #endif 

    #ifdef VINI256
#define NBL0(x,y) ov##x = _mm256_permute4x64_epi64(_mm256_castsi128_si256(_mm_loadu_si128((__m128i *)(p       ))),_MM_SHUFFLE(3, 1, 2, 0));\
                  ov##y = _mm256_permute4x64_epi64(_mm256_castsi128_si256(_mm_loadu_si128((__m128i *)(p+=stride))),_MM_SHUFFLE(3, 1, 2, 0));

#define NBL(x,y)  ov##x = _mm256_permute4x64_epi64(_mm256_castsi128_si256(_mm_loadu_si128((__m128i *)(p+=stride))),_MM_SHUFFLE(3, 1, 2, 0));\
                  ov##y = _mm256_permute4x64_epi64(_mm256_castsi128_si256(_mm_loadu_si128((__m128i *)(p+=stride))),_MM_SHUFFLE(3, 1, 2, 0));

#define NB(x,y,_v_) {\
  ov##x = _mm256_and_si256(_mm256_unpacklo_epi8(ov##x, _mm256_srli_epi16(ov##x,4)), cl);\
  ov##y = _mm256_and_si256(_mm256_unpacklo_epi8(ov##y, _mm256_srli_epi16(ov##y,4)), cl);\
  _v_  = _mm256_or_si256(_mm256_slli_epi16(ov##y,4), ov##x); \
}

void T3(TP, dec256v, ESIZE)(unsigned char *in, unsigned n, unsigned char *out) {
  unsigned      v      = n&~(ESIZE*32-1);
  unsigned      stride = v/STRIDE;
  unsigned char *op,*ip;
  VINI256;

    #if STRIDE > ESIZE
  __m256i cl = _mm256_set1_epi8(0x0f), ch=_mm256_set1_epi8(0xf0), cb = _mm256_set1_epi16(0xff);
    #endif

  for(op = out,ip = in; op != out+v; ip += ESIZE*32/STRIDE, op += ESIZE*32) { unsigned char *p = ip;    PREFETCH(ip+ESIZE*192,0);
    __m256i iv0, iv1, ov0, ov1;
	  #if ESIZE >= 2
    __m256i iv2, iv3, ov2, ov3;
	    #if ESIZE > 4
    __m256i iv4, iv5, iv6, iv7, ov4, ov5, ov6, ov7;
	    #endif
	#endif

      #if STRIDE > ESIZE
    NBL0(0,1); NBL( 2,3); NB(0,1,iv0); NB(2,3,iv1);
        #if ESIZE > 2
    NBL( 0,1); NBL( 2,3); NB(0,1,iv2); NB(2,3,iv3);
          #if ESIZE > 4
    NBL(4,5); NBL( 6,7); NB(4,5,iv4); NB(6,7,iv5);
    NBL(4,5); NBL( 6,7); NB(4,5,iv6); NB(6,7,iv7);
          #endif
        #endif
      #else
    iv0 = _mm256_loadu_si256((__m256i *) p        );
    iv1 = _mm256_loadu_si256((__m256i *)(p+=stride));
        #if ESIZE > 2
    iv2 = _mm256_loadu_si256((__m256i *)(p+=stride));
    iv3 = _mm256_loadu_si256((__m256i *)(p+=stride));
          #if ESIZE > 4
    iv4 = _mm256_loadu_si256((__m256i *)(p+=stride));
    iv5 = _mm256_loadu_si256((__m256i *)(p+=stride));
    iv6 = _mm256_loadu_si256((__m256i *)(p+=stride));
    iv7 = _mm256_loadu_si256((__m256i *)(p+=stride));
          #endif
        #endif
      #endif

      #if ESIZE == 2
    ov0 = _mm256_permute4x64_epi64(iv0, _MM_SHUFFLE(3, 1, 2, 0));
    ov1 = _mm256_permute4x64_epi64(iv1, _MM_SHUFFLE(3, 1, 2, 0));
    iv0 = _mm256_unpacklo_epi8(ov0, ov1); VD256(iv0,vs); 
    iv1 = _mm256_unpackhi_epi8(ov0, ov1); VD256(iv1,vs); 
    _mm256_storeu_si256((__m256i *)op,      iv0);
    _mm256_storeu_si256((__m256i *)(op+32), iv1);
      #elif ESIZE == 4
    ov0 = _mm256_unpacklo_epi8( iv0, iv1); ov1 = _mm256_unpackhi_epi8( iv0, iv1);
    ov2 = _mm256_unpacklo_epi8( iv2, iv3); ov3 = _mm256_unpackhi_epi8( iv2, iv3);

    iv0 = _mm256_unpacklo_epi16(ov0, ov2); iv1 = _mm256_unpackhi_epi16(ov0, ov2);
    iv2 = _mm256_unpacklo_epi16(ov1, ov3); iv3 = _mm256_unpackhi_epi16(ov1, ov3);

    ov0 = _mm256_permute2x128_si256(iv0, iv1, (2 << 4) | 0);
    ov1 = _mm256_permute2x128_si256(iv2, iv3, (2 << 4) | 0);
    ov2 = _mm256_permute2x128_si256(iv0, iv1, (3 << 4) | 1);
    ov3 = _mm256_permute2x128_si256(iv2, iv3, (3 << 4) | 1);
    VDQ256(ov0,ov1,ov2,ov3,vs);
    _mm256_storeu_si256((__m256i *) op,     ov0);
    _mm256_storeu_si256((__m256i *)(op+32), ov1);
    _mm256_storeu_si256((__m256i *)(op+64), ov2);
    _mm256_storeu_si256((__m256i *)(op+96), ov3);
     #else
    ov0 = _mm256_unpacklo_epi8(iv0, iv1); ov1 = _mm256_unpackhi_epi8(iv0, iv1);
    ov2 = _mm256_unpacklo_epi8(iv2, iv3); ov3 = _mm256_unpackhi_epi8(iv2, iv3);
    iv0 = _mm256_permute4x64_epi64(_mm256_unpacklo_epi16(ov0, ov2), _MM_SHUFFLE(3, 1, 2, 0));
    iv1 = _mm256_permute4x64_epi64(_mm256_unpackhi_epi16(ov0, ov2), _MM_SHUFFLE(3, 1, 2, 0));
    iv2 = _mm256_permute4x64_epi64(_mm256_unpacklo_epi16(ov1, ov3), _MM_SHUFFLE(3, 1, 2, 0));
    iv3 = _mm256_permute4x64_epi64(_mm256_unpackhi_epi16(ov1, ov3), _MM_SHUFFLE(3, 1, 2, 0));

    ov4 = _mm256_unpacklo_epi8(iv4, iv5); ov5 = _mm256_unpackhi_epi8(iv4, iv5);
    ov6 = _mm256_unpacklo_epi8(iv6, iv7); ov7 = _mm256_unpackhi_epi8(iv6, iv7);
    iv4 = _mm256_permute4x64_epi64(_mm256_unpacklo_epi16(ov4, ov6), _MM_SHUFFLE(3, 1, 2, 0));
    iv5 = _mm256_permute4x64_epi64(_mm256_unpackhi_epi16(ov4, ov6), _MM_SHUFFLE(3, 1, 2, 0));
    iv6 = _mm256_permute4x64_epi64(_mm256_unpacklo_epi16(ov5, ov7), _MM_SHUFFLE(3, 1, 2, 0));
    iv7 = _mm256_permute4x64_epi64(_mm256_unpackhi_epi16(ov5, ov7), _MM_SHUFFLE(3, 1, 2, 0));

    ov0 = _mm256_unpacklo_epi32(iv0, iv4);
    ov1 = _mm256_unpacklo_epi32(iv1, iv5);
    ov2 = _mm256_unpacklo_epi32(iv2, iv6);
    ov3 = _mm256_unpacklo_epi32(iv3, iv7);
    
    ov4 = _mm256_unpackhi_epi32(iv0, iv4);
    ov5 = _mm256_unpackhi_epi32(iv1, iv5);
    ov6 = _mm256_unpackhi_epi32(iv2, iv6);
    ov7 = _mm256_unpackhi_epi32(iv3, iv7);

    VDQ256(ov0,ov1,ov2,ov3,vs);    
    ST256((__m256i *) op,      ov0 );
    ST256((__m256i *)(op+ 32), ov1 );
    ST256((__m256i *)(op+ 64), ov2 );
    ST256((__m256i *)(op+ 96), ov3 );
    VDQ256(ov4,ov5,ov6,ov7,vs);    
    ST256((__m256i *)(op+128), ov4 );
    ST256((__m256i *)(op+160), ov5 );
    ST256((__m256i *)(op+192), ov6 );
    ST256((__m256i *)(op+224), ov7 );
      #endif
  }
  if(n-v) T2(tpdec,ESIZE)(in+v, n-v, out+v);
}
    #endif
  #else //__AVX2__

	#if (defined(__SSE3__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)) && (ESIZE == 2 || ESIZE == 4 || ESIZE == 8)
#define ST(_p_,_v_,_i_)  _mm_storeu_si128((__m128i *)SIE(_p_,_i_), _v_)
#define ST0(_p_,_v_)  _mm_storeu_si128((__m128i *)(_p_), _v_)

void T3(TP, enc128v, ESIZE)(unsigned char *in, unsigned n, unsigned char *out) {
  unsigned           v = n&~(ESIZE*32-1);
  unsigned      stride = v/STRIDE;
  unsigned char *op,*ip;

    #if defined(__SSE3__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)
      #if ESIZE == 2
  __m128i sf = _mm_set_epi8(15, 13, 11, 9, 7, 5, 3, 1,
                            14, 12, 10, 8, 6, 4, 2, 0);
      #elif ESIZE == 4
  __m128i sf = _mm_set_epi8(15, 11, 7,3,
                            14, 10, 6,2,
                            13,  9, 5,1,
                            12,  8, 4,0);
      #else
  __m128i sf = _mm_set_epi8(15,  7,
                            14,  6,
                            13,  5,
                            12,  4,
                            11,  3,
                            10,  2,
                             9,  1,
                             8,  0  );
      #endif
    #endif
  VINI128;
      #if STRIDE > ESIZE
  __m128i cl = _mm_set1_epi8(0x0f), ch=_mm_set1_epi8(0xf0), cb = _mm_set1_epi16(0xff);
      #endif

  for(ip = in, op = out; ip != in+v; ip+=ESIZE*16,op += ESIZE*16/STRIDE) { unsigned char *p = op;   PREFETCH(ip+(ESIZE*16)*ESIZE,0);
    __m128i iv0, iv1, ov0, ov1;
	  #if ESIZE >= 2
    __m128i iv2, iv3, ov2, ov3;
	    #if ESIZE > 4
    __m128i iv4, iv5, iv6, iv7, ov4, ov5, ov6, ov7;
	    #endif
	#endif
	
      #if defined(__SSSE3__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)
        #if   ESIZE == 2
          #ifdef __ARM_NEON
    uint8x16x2_t w = vld2q_u8(ip);
            #if STRIDE <= ESIZE
    ST0(p,(__m128i)w.val[0]); ST(p,(__m128i)w.val[1],1);
            #else
    iv0 = (__m128i)w.val[0]; iv1 = (__m128i)w.val[1];
            #endif
          #else
    ov0 = LD128(ip);    VE128(ov0,vs); ov0 = _mm_shuffle_epi8(ov0, sf);
    ov1 = LD128(ip+16); VE128(ov1,vs); ov1 = _mm_shuffle_epi8(ov1, sf);

    iv0 = _mm_unpacklo_epi64(ov0, ov1); iv1 = _mm_unpackhi_epi64(ov0, ov1);
            #if STRIDE <= ESIZE
    ST0(p,iv0); ST(p,iv1,1);
            #endif
          #endif

        #elif ESIZE == 4
          #ifdef __ARM_NEON
    uint8x16x4_t w = vld4q_u8(ip);
            #if STRIDE <= ESIZE
    ST0(p,(__m128i)w.val[0]); ST(p,(__m128i)w.val[1],1); ST(p,(__m128i)w.val[2],2); ST(p,(__m128i)w.val[3],3);
            #else
    iv0 = (__m128i)w.val[0]; iv1 = (__m128i)w.val[1]; iv2 = (__m128i)w.val[2]; iv3 = (__m128i)w.val[3];
            #endif
          #else
    iv0 = LD128(ip   ); VE128(iv0,vs); iv0 = _mm_shuffle_epi8(iv0, sf);
    iv1 = LD128(ip+16); VE128(iv1,vs); iv1 = _mm_shuffle_epi8(iv1, sf);
    iv2 = LD128(ip+32); VE128(iv2,vs); iv2 = _mm_shuffle_epi8(iv2, sf);
    iv3 = LD128(ip+48); VE128(iv3,vs); iv3 = _mm_shuffle_epi8(iv3, sf);

    ov0 = _mm_unpacklo_epi32(iv0, iv1); ov1 = _mm_unpackhi_epi32(iv0, iv1);
    ov2 = _mm_unpacklo_epi32(iv2, iv3); ov3 = _mm_unpackhi_epi32(iv2, iv3);

    iv0 = _mm_unpacklo_epi64(ov0, ov2); iv1 = _mm_unpackhi_epi64(ov0, ov2);
    iv2 = _mm_unpacklo_epi64(ov1, ov3); iv3 = _mm_unpackhi_epi64(ov1, ov3);
            #if STRIDE <= ESIZE
    ST0(p,iv0); ST(p,iv1,1); ST(p,iv2,2); ST(p,iv3,3);
            #endif
          #endif

        #elif ESIZE == 8
          #ifdef __ARM_NEON
    #define vzipl_u16(_a_,_b_) vzip_u16(vget_low_u16((uint16x8_t)(_a_)), vget_low_u16((uint16x8_t)(_b_)))
    #define vziph_u16(_a_,_b_) vzip_u16(vget_high_u16((uint16x8_t)(_a_)), vget_high_u16((uint16x8_t)(_b_)))
    //#define VQ
            #ifndef VQ
    uint16x4x2_t v16[8];
    uint32x2x2_t v32[8];
            #else
    uint8x16x2_t v8[4];
    uint16x8x2_t v16[4];
    uint32x4x2_t v32[4];   //uint64x2x2_t v64[4];
            #endif
            #ifdef VQ
    ov0 = LD128(ip    ); VE128(ov0,vs); //ov0 = _mm_shuffle_epi8(ov0, vs);
    ov1 = LD128(ip+ 16); VE128(ov1,vs); //ov1 = _mm_shuffle_epi8(ov1, vs);
    ov2 = LD128(ip+ 32); VE128(ov2,vs); //ov2 = _mm_shuffle_epi8(ov2, vs);
    ov3 = LD128(ip+ 48); VE128(ov3,vs); //ov3 = _mm_shuffle_epi8(ov3, vs);
    ov4 = LD128(ip+ 64); VE128(ov4,vs); //ov4 = _mm_shuffle_epi8(ov4, vs);
    ov5 = LD128(ip+ 80); VE128(ov5,vs); //ov5 = _mm_shuffle_epi8(ov5, vs);
    ov6 = LD128(ip+ 96); VE128(ov6,vs); //ov6 = _mm_shuffle_epi8(ov6, vs);
    ov7 = LD128(ip+112); VE128(ov7,vs); //ov7 = _mm_shuffle_epi8(ov7, vs);

    v8[0]  = vzipq_u8((uint8x16_t)ov0, (uint8x16_t)ov1);
    v8[1]  = vzipq_u8((uint8x16_t)ov2, (uint8x16_t)ov3);
    v8[2]  = vzipq_u8((uint8x16_t)ov4, (uint8x16_t)ov5);
    v8[3]  = vzipq_u8((uint8x16_t)ov6, (uint8x16_t)ov7);

/*    v16[0] = vzipq_u16((uint16x8_t)ov0, (uint16x8_t)ov1);
    v16[1] = vzipq_u16((uint16x8_t)ov2, (uint16x8_t)ov3);
    v16[2] = vzipq_u16((uint16x8_t)ov4, (uint16x8_t)ov5);
    v16[3] = vzipq_u16((uint16x8_t)ov6, (uint16x8_t)ov7);*/
    v16[0] = vzipq_u16(vreinterpretq_u16_u8( v8[0].val[0]),  vreinterpretq_u16_u8(v8[1].val[0]));
    v16[1] = vzipq_u16(vreinterpretq_u16_u8( v8[0].val[1]),  vreinterpretq_u16_u8(v8[1].val[1]));
    v16[2] = vzipq_u16(vreinterpretq_u16_u8( v8[2].val[0]),  vreinterpretq_u16_u8(v8[3].val[0]));
    v16[3] = vzipq_u16(vreinterpretq_u16_u8( v8[2].val[1]),  vreinterpretq_u16_u8(v8[3].val[1]));

    v32[0] = vzipq_u32(vreinterpretq_u32_u16(v16[0].val[0]), vreinterpretq_u32_u16(v16[2].val[0]));
    v32[1] = vzipq_u32(vreinterpretq_u32_u16(v16[0].val[1]), vreinterpretq_u32_u16(v16[2].val[1]));
    v32[2] = vzipq_u32(vreinterpretq_u32_u16(v16[1].val[0]), vreinterpretq_u32_u16(v16[3].val[0]));
    v32[3] = vzipq_u32(vreinterpretq_u32_u16(v16[1].val[1]), vreinterpretq_u32_u16(v16[3].val[1]));

    iv0 = _mm_unpacklo_epi64(v32[0].val[0], v32[2].val[0]); iv1 = _mm_unpackhi_epi64(v32[0].val[0], v32[2].val[0]);
    iv2 = _mm_unpacklo_epi64(v32[0].val[1], v32[2].val[1]); iv3 = _mm_unpackhi_epi64(v32[0].val[1], v32[2].val[1]);
    iv4 = _mm_unpacklo_epi64(v32[1].val[0], v32[3].val[0]); iv5 = _mm_unpackhi_epi64(v32[1].val[0], v32[3].val[0]);
    iv6 = _mm_unpacklo_epi64(v32[1].val[1], v32[3].val[1]); iv7 = _mm_unpackhi_epi64(v32[1].val[1], v32[3].val[1]);
            #else
    ov0 = LD128(ip    ); VE128(ov0,vs); ov0 = _mm_shuffle_epi8(ov0, sf);
    ov1 = LD128(ip+ 16); VE128(ov1,vs); ov1 = _mm_shuffle_epi8(ov1, sf);
    ov2 = LD128(ip+ 32); VE128(ov2,vs); ov2 = _mm_shuffle_epi8(ov2, sf);
    ov3 = LD128(ip+ 48); VE128(ov3,vs); ov3 = _mm_shuffle_epi8(ov3, sf);
    ov4 = LD128(ip+ 64); VE128(ov4,vs); ov4 = _mm_shuffle_epi8(ov4, sf);
    ov5 = LD128(ip+ 80); VE128(ov5,vs); ov5 = _mm_shuffle_epi8(ov5, sf);
    ov6 = LD128(ip+ 96); VE128(ov6,vs); ov6 = _mm_shuffle_epi8(ov6, sf);
    ov7 = LD128(ip+112); VE128(ov7,vs); ov7 = _mm_shuffle_epi8(ov7, sf);
    v16[0] = vzipl_u16(ov0, ov1); v16[1] = vziph_u16(ov0, ov1);
    v16[2] = vzipl_u16(ov2, ov3); v16[3] = vziph_u16(ov2, ov3);
    v16[4] = vzipl_u16(ov4, ov5); v16[5] = vziph_u16(ov4, ov5);
    v16[6] = vzipl_u16(ov6, ov7); v16[7] = vziph_u16(ov6, ov7);

    v32[0] = vzip_u32(vreinterpret_u32_u16(v16[0].val[0]), vreinterpret_u32_u16(v16[2].val[0]) );
    v32[1] = vzip_u32(vreinterpret_u32_u16(v16[0].val[1]), vreinterpret_u32_u16(v16[2].val[1]) );
    v32[2] = vzip_u32(vreinterpret_u32_u16(v16[1].val[0]), vreinterpret_u32_u16(v16[3].val[0]) );
    v32[3] = vzip_u32(vreinterpret_u32_u16(v16[1].val[1]), vreinterpret_u32_u16(v16[3].val[1]) );
    v32[4] = vzip_u32(vreinterpret_u32_u16(v16[4].val[0]), vreinterpret_u32_u16(v16[6].val[0]) );
    v32[5] = vzip_u32(vreinterpret_u32_u16(v16[4].val[1]), vreinterpret_u32_u16(v16[6].val[1]) );
    v32[6] = vzip_u32(vreinterpret_u32_u16(v16[5].val[0]), vreinterpret_u32_u16(v16[7].val[0]) );
    v32[7] = vzip_u32(vreinterpret_u32_u16(v16[5].val[1]), vreinterpret_u32_u16(v16[7].val[1]) );

    iv0 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[0].val[0]), vreinterpret_u64_u32(v32[4].val[0]) );
    iv1 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[0].val[1]), vreinterpret_u64_u32(v32[4].val[1]) );
    iv2 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[1].val[0]), vreinterpret_u64_u32(v32[5].val[0]) );
    iv3 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[1].val[1]), vreinterpret_u64_u32(v32[5].val[1]) );

    iv4 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[2].val[0]), vreinterpret_u64_u32(v32[6].val[0]) );
    iv5 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[2].val[1]), vreinterpret_u64_u32(v32[6].val[1]) );
    iv6 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[3].val[0]), vreinterpret_u64_u32(v32[7].val[0]) );
    iv7 = (__m128i)vcombine_u64(vreinterpret_u64_u32(v32[3].val[1]), vreinterpret_u64_u32(v32[7].val[1]) );
            #endif
            #if STRIDE <= ESIZE
    ST0(p,iv0); ST(p,iv1,1); ST(p,iv2,2); ST(p,iv3,3); ST(p,iv4,4); ST(p,iv5,5); ST(p,iv6,6); ST(p,iv7,7);
            #endif
          #else // SSE
    ov0 = LD128(ip   ); VE128(ov0,vs); ov0 = _mm_shuffle_epi8(ov0, sf);
    ov1 = LD128(ip+16); VE128(ov1,vs); ov1 = _mm_shuffle_epi8(ov1, sf);
    ov2 = LD128(ip+32); VE128(ov2,vs); ov2 = _mm_shuffle_epi8(ov2, sf);
    ov3 = LD128(ip+48); VE128(ov3,vs); ov3 = _mm_shuffle_epi8(ov3, sf);

    iv0 = _mm_unpacklo_epi16(ov0, ov1); iv1 = _mm_unpackhi_epi16(ov0, ov1);
    iv2 = _mm_unpacklo_epi16(ov2, ov3); iv3 = _mm_unpackhi_epi16(ov2, ov3);

    ov0 = _mm_unpacklo_epi32(iv0, iv2); ov1 = _mm_unpackhi_epi32(iv0, iv2);
    ov2 = _mm_unpacklo_epi32(iv1, iv3); ov3 = _mm_unpackhi_epi32(iv1, iv3);

    ov4 = LD128(ip+ 64); VE128(ov4,vs); ov4 = _mm_shuffle_epi8(ov4, sf);
    ov5 = LD128(ip+ 80); VE128(ov5,vs); ov5 = _mm_shuffle_epi8(ov5, sf);
    ov6 = LD128(ip+ 96); VE128(ov6,vs); ov6 = _mm_shuffle_epi8(ov6, sf);
    ov7 = LD128(ip+112); VE128(ov7,vs); ov7 = _mm_shuffle_epi8(ov7, sf);

    iv4 = _mm_unpacklo_epi16(ov4, ov5); iv5 = _mm_unpackhi_epi16(ov4, ov5);
    iv6 = _mm_unpacklo_epi16(ov6, ov7); iv7 = _mm_unpackhi_epi16(ov6, ov7);

    ov4 = _mm_unpacklo_epi32(iv4, iv6); ov5 = _mm_unpackhi_epi32(iv4, iv6);
    ov6 = _mm_unpacklo_epi32(iv5, iv7); ov7 = _mm_unpackhi_epi32(iv5, iv7);

    iv0 = _mm_unpacklo_epi64(ov0, ov4); iv1 = _mm_unpackhi_epi64(ov0, ov4);
    iv2 = _mm_unpacklo_epi64(ov1, ov5); iv3 = _mm_unpackhi_epi64(ov1, ov5);

    iv4 = _mm_unpacklo_epi64(ov2, ov6); iv5 = _mm_unpackhi_epi64(ov2, ov6);
    iv6 = _mm_unpacklo_epi64(ov3, ov7); iv7 = _mm_unpackhi_epi64(ov3, ov7);
             #if STRIDE <= ESIZE
    ST0(p,iv0); ST(p,iv1,1); ST(p,iv2,2); ST(p,iv3,3); ST(p,iv4,4); ST(p,iv5,5); ST(p,iv6,6); ST(p,iv7,7);
            #endif
          #endif
        #endif

      #elif defined(__SSE3__)
        #if ESIZE == 2
    iv0 = LD128(ip   );  
    iv1 = LD128(ip+16)); 
    VE128(iv0,vs);
    VE128(iv1,vs);

    ov0 = _mm_unpacklo_epi8(iv0, iv1); ov1 = _mm_unpackhi_epi8(iv0, iv1);
    iv0 = _mm_unpacklo_epi8(ov0, ov1); iv1 = _mm_unpackhi_epi8(ov0, ov1);

    ov0 = _mm_unpacklo_epi8(iv0, iv1); ov1 = _mm_unpackhi_epi8(iv0, iv1);
    iv0 = _mm_unpacklo_epi8(ov0, ov1); iv1 = _mm_unpackhi_epi8(ov0, ov1);
    ST0(p,iv0); ST(p,iv1,1);
        #elif ESIZE == 4
    iv0 = LD128(ip   ); 
    iv1 = LD128(ip+16); 
    iv2 = LD128(ip+32); 
    iv3 = LD128(ip+48); 
    VE128(iv0,vs); VE128(iv1,vs); VE128(iv2,vs); VE128(iv3,vs);

    ov0 = _mm_unpacklo_epi8( iv0, iv1); ov1 = _mm_unpackhi_epi8( iv0, iv1);
    iv0 = _mm_unpacklo_epi8( ov0, ov1); iv1 = _mm_unpackhi_epi8( ov0, ov1);

    ov0 = _mm_unpacklo_epi8( iv0, iv1); ov1 = _mm_unpackhi_epi8( iv0, iv1);
    iv0 = _mm_unpacklo_epi64(ov0, ov2); iv1 = _mm_unpackhi_epi64(ov0, ov2);

    ov2 = _mm_unpacklo_epi8( iv2, iv3); ov3 = _mm_unpackhi_epi8( iv2, iv3);
    iv2 = _mm_unpacklo_epi8( ov2, ov3); iv3 = _mm_unpackhi_epi8( ov2, ov3);
    ov2 = _mm_unpacklo_epi8( iv2, iv3); ov3 = _mm_unpackhi_epi8( iv2, iv3);

    iv2 = _mm_unpacklo_epi64(ov1, ov3); iv3 = _mm_unpackhi_epi64(ov1, ov3);
    ST0(p,iv0); ST(p,iv1,1); ST(p,iv2,2); ST(p,iv3,3);
        #elif ESIZE == 8
    iv0 = LD128(ip   ); 
    iv1 = LD128(ip+16); 
    iv2 = LD128(ip+32); 
    iv3 = LD128(ip+48); 
    VE128(iv0,vs); VE128(iv1,vs); VE128(iv2,vs); VE128(iv2,vs);
    iv4 = LD128(ip+64); 
    iv5 = LD128(ip+80); 
    iv6 = LD128(ip+96); 
    iv7 = LD128(ip+112);
    VE128(iv4,vs); VE128(iv5,vs); VE128(iv6,vs); VE128(iv7,vs);

    ov0 = _mm_unpacklo_epi8( iv0, iv1); ov1 = _mm_unpackhi_epi8( iv0, iv1);
    ov2 = _mm_unpacklo_epi8( iv2, iv3); ov3 = _mm_unpackhi_epi8( iv2, iv3);
    ov4 = _mm_unpacklo_epi8( iv4, iv5); ov5 = _mm_unpackhi_epi8( iv4, iv5);
    ov6 = _mm_unpacklo_epi8( iv6, iv7); ov7 = _mm_unpackhi_epi8( iv6, iv7);

    iv0 = _mm_unpacklo_epi8( ov0, ov1); iv1 = _mm_unpackhi_epi8( ov0, ov1);
    iv2 = _mm_unpacklo_epi8( ov2, ov3); iv3 = _mm_unpackhi_epi8( ov2, ov3);
    iv4 = _mm_unpacklo_epi8( ov4, ov5); iv5 = _mm_unpackhi_epi8( ov4, ov5);
    iv6 = _mm_unpacklo_epi8( ov6, ov7); iv7 = _mm_unpackhi_epi8( ov6, ov7);

    ov0 = _mm_unpacklo_epi32(iv0, iv2); ov1 = _mm_unpackhi_epi32(iv0, iv2);
    ov2 = _mm_unpacklo_epi32(iv1, iv3); ov3 = _mm_unpackhi_epi32(iv1, iv3);
    ov4 = _mm_unpacklo_epi32(iv4, iv6); ov5 = _mm_unpackhi_epi32(iv4, iv6);
    ov6 = _mm_unpacklo_epi32(iv5, iv7); ov7 = _mm_unpackhi_epi32(iv5, iv7);
    ST0(p,iv0); ST(p,iv1,1); ST(p,iv2,2); ST(p,iv3,3);

    iv0 = _mm_unpacklo_epi64(ov0, ov4); iv1 = _mm_unpackhi_epi64(ov0, ov4);
    iv2 = _mm_unpacklo_epi64(ov1, ov5); iv3 = _mm_unpackhi_epi64(ov1, ov5);
    iv4 = _mm_unpacklo_epi64(ov2, ov6); iv5 = _mm_unpackhi_epi64(ov2, ov6);
    iv6 = _mm_unpacklo_epi64(ov3, ov7); iv7 = _mm_unpackhi_epi64(ov3, ov7);
    ST(p,iv4,4);
    ST(p,iv5,5);
    ST(p,iv6,6);
    ST(p,iv7,7);
        #endif
      #endif

      #if STRIDE > ESIZE // ---------------------- Nibble -------------------------------------------
    #define STL(_p_,_v_,_i_)  _mm_storel_epi64((__m128i *)SIE(_p_,_i_), _v_)
    #define STL0(_p_,_v_)     _mm_storel_epi64((__m128i *)(_p_), _v_)

    ov0 = _mm_and_si128(iv0, cl);                   ov0 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov0,4), ov0),cb); ov0 = _mm_packus_epi16(ov0, _mm_srli_si128(ov0,2));
    ov1 =  mm_srli_epi16(_mm_and_si128(iv0, ch),4); ov1 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov1,4), ov1),cb); ov1 = _mm_packus_epi16(ov1, _mm_srli_si128(ov1,2));
    ov2 = _mm_and_si128(iv1, cl);                   ov2 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov2,4), ov2),cb); ov2 = _mm_packus_epi16(ov2, _mm_srli_si128(ov2,2));
    ov3 =  mm_srli_epi16(_mm_and_si128(iv1, ch),4); ov3 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov3,4), ov3),cb); ov3 = _mm_packus_epi16(ov3, _mm_srli_si128(ov3,2));
    STL0(p,ov0); STL(p,ov1,1);STL(p,ov2,2);STL(p,ov3,3);
        #if ESIZE > 2
    ov0 = _mm_and_si128(iv2, cl);                   ov0 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov0,4), ov0),cb); ov0 = _mm_packus_epi16(ov0, _mm_srli_si128(ov0,2));
    ov1 =  mm_srli_epi16(_mm_and_si128(iv2, ch),4); ov1 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov1,4), ov1),cb); ov1 = _mm_packus_epi16(ov1, _mm_srli_si128(ov1,2));
    ov2 = _mm_and_si128(iv3, cl);                   ov2 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov2,4), ov2),cb); ov2 = _mm_packus_epi16(ov2, _mm_srli_si128(ov2,2));
    ov3 =  mm_srli_epi16(_mm_and_si128(iv3, ch),4); ov3 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov3,4), ov3),cb); ov3 = _mm_packus_epi16(ov3, _mm_srli_si128(ov3,2));
    STL(p,ov0,4); STL(p,ov1,5);STL(p,ov2,6);STL(p,ov3,7);
          #if ESIZE > 4
    ov0 = _mm_and_si128(iv4, cl);                   ov0 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov0,4), ov0),cb); ov0 = _mm_packus_epi16(ov0, _mm_srli_si128(ov0,2));
    ov1 =  mm_srli_epi16(_mm_and_si128(iv4, ch),4); ov1 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov1,4), ov1),cb); ov1 = _mm_packus_epi16(ov1, _mm_srli_si128(ov1,2));
    ov2 = _mm_and_si128(iv5, cl);                   ov2 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov2,4), ov2),cb); ov2 = _mm_packus_epi16(ov2, _mm_srli_si128(ov2,2));
    ov3 =  mm_srli_epi16(_mm_and_si128(iv5, ch),4); ov3 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov3,4), ov3),cb); ov3 = _mm_packus_epi16(ov3, _mm_srli_si128(ov3,2));
    STL(p,ov0,8); STL(p,ov1,9);STL(p,ov2,10);STL(p,ov3,11);

    ov4 = _mm_and_si128(iv6, cl);                   ov4 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov4,4), ov4),cb); ov4 = _mm_packus_epi16(ov4, _mm_srli_si128(ov4,2));
    ov5 =  mm_srli_epi16(_mm_and_si128(iv6, ch),4); ov5 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov5,4), ov5),cb); ov5 = _mm_packus_epi16(ov5, _mm_srli_si128(ov5,2));
    ov6 = _mm_and_si128(iv7, cl);                   ov6 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov6,4), ov6),cb); ov6 = _mm_packus_epi16(ov6, _mm_srli_si128(ov6,2));
    ov7 =  mm_srli_epi16(_mm_and_si128(iv7, ch),4); ov7 = _mm_and_si128(_mm_or_si128(mm_srli_epi16(ov7,4), ov7),cb); ov7 = _mm_packus_epi16(ov7, _mm_srli_si128(ov7,2));
    STL(p,ov4,12); STL(p,ov5,13); STL(p,ov6,14); STL(p,ov7,15);
          #endif
        #endif
      #endif
  }
  T2(tpenc,ESIZE)(in+v, n-v, out+v);
}

void T3(TP, dec128v, ESIZE)(unsigned char *in, unsigned n, unsigned char *out) {
  unsigned           v = n&~(ESIZE*32-1);
  unsigned      stride = v/STRIDE;
  unsigned char *op,*ip;

    #if STRIDE > ESIZE
  __m128i cl = _mm_set1_epi8(0x0f), ch=_mm_set1_epi8(0xf0), cb = _mm_set1_epi16(0xff);
    #endif
  VINI128;
  for(op = out,ip = in; op != out+v; op+=ESIZE*16,ip += ESIZE*16/STRIDE) {
    unsigned char *p=ip;                                                        PREFETCH(ip+(ESIZE*16/STRIDE)*ESIZE,0);
    __m128i iv0, iv1, ov0, ov1;
	  #if ESIZE >= 2
    __m128i iv2, iv3, ov2, ov3;
	    #if ESIZE > 4
    __m128i iv4, iv5, iv6, iv7, ov4, ov5, ov6, ov7;
	    #endif
	#endif

      #if STRIDE > ESIZE //------------ Nibble transpose -------------------
    ov0 = _mm_loadl_epi64((__m128i *)    p   );
    ov1 = _mm_loadl_epi64((__m128i *)SID(p,1));
    ov2 = _mm_loadl_epi64((__m128i *)SID(p,2));
    ov3 = _mm_loadl_epi64((__m128i *)SID(p,3));

    ov0 = _mm_unpacklo_epi8(ov0, mm_srli_epi16(ov0,4)); ov0 = _mm_and_si128(ov0, cl); // 0,1->0
    ov1 = _mm_unpacklo_epi8(ov1, mm_srli_epi16(ov1,4)); ov1 = _mm_and_si128(ov1, cl);
    iv0 = _mm_or_si128(mm_slli_epi16(ov1,4), ov0);

    ov2 = _mm_unpacklo_epi8(ov2, mm_srli_epi16(ov2,4)); ov2 = _mm_and_si128(ov2, cl); // 2,3->1
    ov3 = _mm_unpacklo_epi8(ov3, mm_srli_epi16(ov3,4)); ov3 = _mm_and_si128(ov3, cl);
    iv1 = _mm_or_si128(mm_slli_epi16(ov3,4), ov2);
        #if ESIZE > 2
    ov0 = _mm_loadl_epi64((__m128i *)SID(p,4));
    ov1 = _mm_loadl_epi64((__m128i *)SID(p,5));
    ov2 = _mm_loadl_epi64((__m128i *)SID(p,6));
    ov3 = _mm_loadl_epi64((__m128i *)SID(p,7));

    ov0 = _mm_unpacklo_epi8(ov0, mm_srli_epi16(ov0,4)); ov0 = _mm_and_si128(ov0, cl); // 0,1->2
    ov1 = _mm_unpacklo_epi8(ov1, mm_srli_epi16(ov1,4)); ov1 = _mm_and_si128(ov1, cl);
    iv2 = _mm_or_si128(mm_slli_epi16(ov1,4), ov0);

    ov2 = _mm_unpacklo_epi8(ov2, mm_srli_epi16(ov2,4)); ov2 = _mm_and_si128(ov2, cl); // 2,3->3
    ov3 = _mm_unpacklo_epi8(ov3, mm_srli_epi16(ov3,4)); ov3 = _mm_and_si128(ov3, cl);
    iv3 = _mm_or_si128(mm_slli_epi16(ov3,4), ov2);
        #endif
        #if ESIZE > 4
    ov0 = _mm_loadl_epi64((__m128i *)SID(p,8));
    ov1 = _mm_loadl_epi64((__m128i *)SID(p,9));
    ov2 = _mm_loadl_epi64((__m128i *)SID(p,10));
    ov3 = _mm_loadl_epi64((__m128i *)SID(p,11));

    ov0 = _mm_unpacklo_epi8(ov0, mm_srli_epi16(ov0,4)); ov0 = _mm_and_si128(ov0, cl); // 0,1->4
    ov1 = _mm_unpacklo_epi8(ov1, mm_srli_epi16(ov1,4)); ov1 = _mm_and_si128(ov1, cl);
    iv4 = _mm_or_si128(mm_slli_epi16(ov1,4), ov0);

    ov2 = _mm_unpacklo_epi8(ov2, mm_srli_epi16(ov2,4)); ov2 = _mm_and_si128(ov2, cl); // 2,3->5
    ov3 = _mm_unpacklo_epi8(ov3, mm_srli_epi16(ov3,4));
    ov3 = _mm_and_si128(ov3, cl);
    iv5 = _mm_or_si128(mm_slli_epi16(ov3,4), ov2);

    ov0 = _mm_loadl_epi64((__m128i *)SID(p,12));
    ov1 = _mm_loadl_epi64((__m128i *)SID(p,13));
    ov2 = _mm_loadl_epi64((__m128i *)SID(p,14));
    ov3 = _mm_loadl_epi64((__m128i *)SID(p,15));

    ov0 = _mm_unpacklo_epi8(ov0, mm_srli_epi16(ov0,4)); ov0 = _mm_and_si128(ov0, cl); // 0,1->6
    ov1 = _mm_unpacklo_epi8(ov1, mm_srli_epi16(ov1,4)); ov1 = _mm_and_si128(ov1, cl);
    iv6 = _mm_or_si128(mm_slli_epi16(ov1,4), ov0);

    ov2 = _mm_unpacklo_epi8(ov2, mm_srli_epi16(ov2,4)); ov2 = _mm_and_si128(ov2, cl); // 2,3->7
    ov3 = _mm_unpacklo_epi8(ov3, mm_srli_epi16(ov3,4)); ov3 = _mm_and_si128(ov3, cl);
    iv7 = _mm_or_si128(mm_slli_epi16(ov3,4), ov2);
        #endif
      #else // --------------------------- Byte transpose -------------------
    iv0 = _mm_loadu_si128((__m128i *)    p   );
    iv1 = _mm_loadu_si128((__m128i *)SID(p,1));
        #if ESIZE > 2
    iv2 = _mm_loadu_si128((__m128i *)SID(p,2));
    iv3 = _mm_loadu_si128((__m128i *)SID(p,3));
          #if ESIZE > 4
    iv4 = _mm_loadu_si128((__m128i *)SID(p,4));
    iv5 = _mm_loadu_si128((__m128i *)SID(p,5));
    iv6 = _mm_loadu_si128((__m128i *)SID(p,6));
    iv7 = _mm_loadu_si128((__m128i *)SID(p,7));
          #endif
        #endif
      #endif
      #if ESIZE == 2
        #ifdef __ARM_NEON
    uint8x16x2_t w; w.val[0] = (uint8x16_t)iv0;
                    w.val[1] = (uint8x16_t)iv1; vst2q_u8(op, w);
        #else
    ov0 = _mm_unpacklo_epi8(iv0, iv1); ov1 = _mm_unpackhi_epi8(iv0, iv1);//i(0,1)->o(0,1)
    VD128(ov0,vs); VD128(ov1,vs); 
    ST128(op,ov0); ST128(op+16,ov1);
        #endif
      #elif ESIZE == 4
        #ifdef __ARM_NEON
    uint8x16x4_t w; w.val[0] = (uint8x16_t)iv0;
                    w.val[1] = (uint8x16_t)iv1;
                    w.val[2] = (uint8x16_t)iv2;
                    w.val[3] = (uint8x16_t)iv3; vst4q_u8(op,w);
        #else
    ov0 = _mm_unpacklo_epi8( iv0, iv1); ov1 = _mm_unpackhi_epi8(iv0, iv1); //i(0,1)->o(0,1)
    ov2 = _mm_unpacklo_epi8( iv2, iv3); ov3 = _mm_unpackhi_epi8(iv2, iv3); //i(2,3)->o(2,3)

    iv0 = _mm_unpacklo_epi16(ov0, ov2); iv1 = _mm_unpackhi_epi16(ov0, ov2);//o(0,2)->i(0,1)
    iv2 = _mm_unpacklo_epi16(ov1, ov3); iv3 = _mm_unpackhi_epi16(ov1, ov3);//o(1,3)->i(2,3)
    VDQ128(iv0,iv1,iv2,iv3,vs); 
    ST128(op, iv0); ST128(op+16,iv1); ST128(op+32,iv2); ST128(op+48,iv3);
        #endif
      #else
    ov0 = _mm_unpacklo_epi8( iv0, iv1); ov1 = _mm_unpackhi_epi8( iv0, iv1);//i(0,1)->o(0,1)
    ov2 = _mm_unpacklo_epi8( iv2, iv3); ov3 = _mm_unpackhi_epi8( iv2, iv3);//i(2,3)->o(2,3)
    
    ov4 = _mm_unpacklo_epi8( iv4, iv5); ov5 = _mm_unpackhi_epi8( iv4, iv5);//i(4,5)->o(4,5)
    ov6 = _mm_unpacklo_epi8( iv6, iv7); ov7 = _mm_unpackhi_epi8( iv6, iv7);//i(6,7)->o(6,7)

    iv0 = _mm_unpacklo_epi16(ov0, ov2); iv1 = _mm_unpackhi_epi16(ov0, ov2);
    iv2 = _mm_unpacklo_epi16(ov1, ov3); iv3 = _mm_unpackhi_epi16(ov1, ov3);
    
    iv4 = _mm_unpacklo_epi16(ov4, ov6); iv5 = _mm_unpackhi_epi16(ov4, ov6);
    iv6 = _mm_unpacklo_epi16(ov5, ov7); iv7 = _mm_unpackhi_epi16(ov5, ov7);

    ov0 = _mm_unpacklo_epi32(iv0, iv4); ov1 = _mm_unpackhi_epi32(iv0, iv4);
    ov2 = _mm_unpacklo_epi32(iv1, iv5); ov3 = _mm_unpackhi_epi32(iv1, iv5);
    
    ov4 = _mm_unpacklo_epi32(iv2, iv6); ov5 = _mm_unpackhi_epi32(iv2, iv6);
    ov6 = _mm_unpacklo_epi32(iv3, iv7); ov7 = _mm_unpackhi_epi32(iv3, iv7);

    VDQ128(ov0,ov1,ov2,ov3,vs); 
    ST128(op, ov0); ST128(op+16, ov1); ST128(op+32, ov2); ST128(op+48, ov3);
    VDQ128(ov4,ov5,ov6,ov7,vs); 
    ST128(op+64, ov4); ST128(op+80, ov5); ST128(op+96, ov6); ST128(op+112,ov7);
      #endif
  }
  T2(tpdec,ESIZE)(in+v, n-v, out+v);
}
    #endif // SSE3

    //--------------------------------------- plain -------------------------------------------------------------------
    #if STRIDE == ESIZE // bytes only, no nibble version
      #if (ESIZE == 2 || ESIZE == 4 || ESIZE == 8) && !defined(ISDELTA)
#define uint_t T3(uint, USIZE, _t)

#define ODX2 (x + y * nx)
#define O2D(_i_) (x + (y+_i_) * nx)
void T2(tp2denc,ESIZE)(unsigned char *in, unsigned nx, unsigned ny, unsigned char *out) {
  unsigned x,y;
  uint_t *op = (uint_t *)out, *ip = (uint_t *)in;

  for(  x = 0; x < nx; x++)
    for(y = 0; y < ny; y++)
      op[ODX2] = *ip++;
}

void T2(tp2ddec,ESIZE)(unsigned char *in, unsigned nx, unsigned ny, unsigned char *out) {
  unsigned x, y;
  uint_t   *op = (uint_t *)out, *ip = (uint_t *)in;

  for(  x = 0; x < nx; x++)
    for(y=0; y != ny; y++)
      *op++ = ip[ODX2];
}
#undef ODX2

#define ODX3 (x + y * nx + z * ny * nx)
void T2(tp3denc,ESIZE)(unsigned char *in, unsigned nx, unsigned ny, unsigned nz, unsigned char *out) {
  unsigned x, y, z;
  uint_t   *op = (uint_t *)out, *ip = (uint_t *)in;

  for(    x = 0; x < nx; x++)
    for(  y = 0; y < ny; y++)
      for(z = 0; z < nz; z++)
        op[ODX3] = *ip++;
}

void T2(tp3ddec,ESIZE)(unsigned char *in, unsigned nx, unsigned ny, unsigned nz, unsigned char *out) {
  unsigned x,y,z;
  uint_t   *op = (uint_t *)out, *ip = (uint_t *)in;

  for(x = 0; x < nx; ++x)
    for(y = 0; y < ny; ++y)
      for(z = 0; z < nz; ++z)
        *op++ = ip[ODX3];
}
#undef ODX3

#define ODX4 (w + x * nw + y * nx * nw + z * nx * ny * nw)
void T2(tp4denc,ESIZE)(unsigned char *in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *out) {
  unsigned w,x,y,z;
  uint_t *op = (uint_t *)out, *ip = (uint_t *)in;

  for(      w = 0; w < nw; w++)
    for(    x = 0; x < nx; x++)
      for(  y = 0; y < ny; y++)
        for(z = 0; z < nz; z++)
          op[ODX4] = *ip++;
}

void T2(tp4ddec,ESIZE)(unsigned char *in, unsigned nw, unsigned nx, unsigned ny, unsigned nz, unsigned char *out) {
  unsigned w,x,y,z;
  uint_t *op = (uint_t *)out, *ip = (uint_t *)in;

  for(      w = 0; w < nw; ++w)
    for(    x = 0; x < nx; ++x)
      for(  y = 0; y < ny; ++y)
        for(z = 0; z < nz; ++z)
          *op++= ip[ODX4];
}
#undef ODX4
      #endif // ISDELTA

void T3(TP, enc, ESIZE)(unsigned char *in, unsigned n, unsigned char *out) {
  unsigned char *op,*ip,*e;
  unsigned stride = n/STRIDE;

    #if powof2(ESIZE)
  e = in+(n&~(ESIZE-1));
    #else
  e = in+stride*ESIZE;
    #endif

  for(ip = in,op = out; ip < e; op++, ip+=ESIZE) { unsigned char *p = op;
    p[0]      = ip[ 0];
    *SIE(p, 1) = ip[ 1];
      #if ESIZE > 2
    *SIE(p, 2) = ip[ 2];
        #if ESIZE > 3
    *SIE(p, 3) = ip[ 3];
          #if ESIZE > 4
    uint32_t u = ctou32(p);
    *SIE(p, 4) = ip[ 4];
    *SIE(p, 5) = ip[ 5];
    *SIE(p, 6) = ip[ 6];
    *SIE(p, 7) = ip[ 7];
            #if ESIZE > 8
    *SIE(p, 8) = ip[ 8];
    *SIE(p, 9) = ip[ 9];
    *SIE(p,10) = ip[10];
    *SIE(p,11) = ip[11];
    *SIE(p,12) = ip[12];
    *SIE(p,13) = ip[13];
    *SIE(p,14) = ip[14];
    *SIE(p,15) = ip[15];
            #endif
          #endif
        #endif
      #endif
  }
  for(op = out+stride*ESIZE;ip < in+n;)
    *op++ = *ip++;
}

void T3(TP, dec, ESIZE)(unsigned char *in, unsigned n, unsigned char *out) {
  unsigned char *op,*ip,*e;
  unsigned      stride = n/STRIDE;

    #if powof2(ESIZE)
  e = out+(n&~(ESIZE-1));
    #else
  e = out+stride*ESIZE;
    #endif
  for(op = out,ip = in; op < e; ip++,op += ESIZE) { unsigned char *p = ip;
    op[ 0] = *p;
    op[ 1] = *SID(p,1);
      #if ESIZE > 2
    op[ 2] = *SID(p,2);
        #if ESIZE > 3
    op[ 3] = *SID(p,3);
          #if ESIZE > 4
    op[ 4] = *SID(p,4);
    op[ 5] = *SID(p,5);
    op[ 6] = *SID(p,6);
    op[ 7] = *SID(p,7);
            #if ESIZE > 8
    op[ 8] = *SID(p,8);
    op[ 9] = *SID(p,9);
    op[10] = *SID(p,10);
    op[11] = *SID(p,11);
    op[12] = *SID(p,12);
    op[13] = *SID(p,13);
    op[14] = *SID(p,14);
    op[15] = *SID(p,15);
            #endif
          #endif
        #endif
      #endif
  }
  for(ip = in+stride*ESIZE; op < out+n; )
    *op++ = *ip++;
}
    #endif // STRIDE = ESIZE

  #endif // avx2
#endif // template

