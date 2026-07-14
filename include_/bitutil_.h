/**
    Copyright (C) powturbo 2013-2026
    GPL v2 License

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
//     "Integer Compression: max.bits, delta, zigzag, xor"
#pragma once

  #ifdef __ARM_NEON
#define PREFETCH(_ip_,_rw_)
  #else
#define PREFETCH(_ip_,_rw_) //__builtin_prefetch(_ip_,_rw_)
  #endif

#ifdef __AVX2__ //***************************************** AVX2 ***********************************************************************************************************
static ALWAYS_INLINE __m256i mm256_srli_epi8(__m256i v, int imm) { return _mm256_and_si256(_mm256_srli_epi16(v, imm), _mm256_set1_epi8(0xff >> imm) ); }

static ALWAYS_INLINE __m256i mm256_cmpgt_epu32(__m256i a, __m256i b) {  // unsigned compare
  const __m256i bias = _mm256_set1_epi32((int)0x80000000);
  return _mm256_cmpgt_epi32(_mm256_xor_si256(a, bias), _mm256_xor_si256(b, bias));
}

static ALWAYS_INLINE __m256i mm256_bsr_epi32(__m256i v) { // bsr 
  __m256i vm = _mm256_srli_epi32(v, 8),
           e = _mm256_srli_epi32(_mm256_castps_si256(_mm256_cvtepi32_ps(_mm256_andnot_si256(vm, v))), 23);
           e = _mm256_subs_epu16(e, _mm256_set1_epi32(126));
           e = _mm256_min_epi16(e, _mm256_set1_epi32(32));
  return e;
}

#define mm256_cvt32_epu64(_v_,_vl_) { /* 32 -> 64 */\
  _vl_ = _mm256_cvtepu32_epi64(_mm256_castsi256_si128(_v_));\
   _v_ = _mm256_cvtepu32_epi64(_mm256_extracti128_si256(_v_, 1));\
}

static ALWAYS_INLINE __m256i mm256_cvt64_epu32(__m256i _vh_, __m256i _vl_) { // 64 -> 32
  return _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(_vl_), _mm256_castsi256_ps(_vh_), _MM_SHUFFLE(2, 0, 2, 0))), _MM_SHUFFLE(3, 1, 2, 0));
}

static ALWAYS_INLINE uint32_t mm256_hor_epi32(__m256i v) { 
  __m128i v128 = _mm_or_si128(_mm256_castsi256_si128(v), _mm256_extracti128_si256(v, 1));
          v128 = _mm_or_si128(v128, _mm_srli_si128(v128, 8));
          v128 = _mm_or_si128(v128, _mm_srli_si128(v128, 4));
  return (uint32_t)_mm_cvtsi128_si32(v128);
}

static ALWAYS_INLINE uint64_t mm256_hor_epi64(__m256i x) { __m128i or128 = _mm_or_si128(_mm256_castsi256_si128(x), _mm256_extracti128_si256(x, 1)); 
  return _mm_cvtsi128_si64(_mm_or_si128(or128, _mm_unpackhi_epi64(or128, or128)));
}

#define mm256_srai_epi64_63(v) _mm256_srai_epi32(_mm256_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 1, 1)), 31)
static ALWAYS_INLINE __m256i mm256_zzage_epi16(__m256i v) { return _mm256_xor_si256(_mm256_slli_epi16(v,1), _mm256_srai_epi16(   v,15)); }
static ALWAYS_INLINE __m256i mm256_zzage_epi32(__m256i v) { return _mm256_xor_si256(_mm256_slli_epi32(v,1), _mm256_srai_epi32(   v,31)); }
static ALWAYS_INLINE __m256i mm256_zzage_epi64(__m256i v) { return _mm256_xor_si256(_mm256_slli_epi64(v,1),  mm256_srai_epi64_63(v)); }

#define MM256_ZZAGD_EPI8( v) { const __m256i _cz = _mm256_setzero_si256(); __m256i _va = _mm256_avg_epu8(v, _cz); v = _mm256_blendv_epi8(_va, _mm256_sub_epi8(_cz, _va), _mm256_slli_epi16(v, 7)); }
#define MM256_ZZAGD_EPI16(v) _mm256_xor_si256(_mm256_srli_epi16(v,1), _mm256_srai_epi16(   _mm256_slli_epi16(v,15),15) )
#define MM256_ZZAGD_EPI32(v) _mm256_xor_si256(_mm256_srli_epi32(v,1), _mm256_srai_epi32(   _mm256_slli_epi32(v,31),31) )
#define MM256_ZZAGD_EPI64(v) _mm256_xor_si256(_mm256_srli_epi64(v,1),  mm256_srai_epi64_63(_mm256_slli_epi64(v,63)) )

static ALWAYS_INLINE __m256i mm256_zzagd_epi8( __m256i v) { MM256_ZZAGD_EPI8(v); return v; }
static ALWAYS_INLINE __m256i mm256_zzagd_epi16(__m256i v) { return MM256_ZZAGD_EPI16(v); }
static ALWAYS_INLINE __m256i mm256_zzagd_epi32(__m256i v) { return MM256_ZZAGD_EPI32(v); }
static ALWAYS_INLINE __m256i mm256_zzagd_epi64(__m256i v) { return MM256_ZZAGD_EPI64(v); }

static ALWAYS_INLINE __m256i mm256_delta_epi16(__m256i v, __m256i vs) { return _mm256_sub_epi16(v, _mm256_alignr_epi8(v, _mm256_permute2x128_si256(vs, v, 0x21), 14)); }
static ALWAYS_INLINE __m256i mm256_delta_epi32(__m256i v, __m256i vs) { const __m256i _idx = _mm256_setr_epi32(7,0,1,2,3,4,5,6); return _mm256_sub_epi32(v, _mm256_permutevar8x32_epi32(_mm256_blend_epi32(v,vs,0x80), _idx)); }
static ALWAYS_INLINE __m256i mm256_delta_epi64(__m256i v, __m256i vs) { return _mm256_sub_epi64(v, _mm256_alignr_epi8(v, _mm256_permute2x128_si256(vs, v, _MM_SHUFFLE(0, 2, 0, 1)),  8)); }

#define _MM256_SCAN_EPI8(_v_,_vs_,_ho_) {\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 1));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 2));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 4));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 8));\
  __m256i _vx = _mm256_shuffle_epi8(_mm256_permute2x128_si256(_v_,  _v_,  0x08), _mm256_set1_epi8(0x0f)),\
          _vp = _mm256_shuffle_epi8(_mm256_permute2x128_si256(_vs_, _vs_, 0x11), _mm256_set1_epi8(0x0f));\
  _vs_ = _v_ = _ho_(_v_, _ho_(_vx, _vp));\
}

#define _MM256_SCAN_EPI16(_v_,_vs_,_ho_) {\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 2));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 4));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 8));\
  __m256i _vx = _mm256_shuffle_epi8(_mm256_permute2x128_si256(_v_,  _v_,  0x08), _mm256_set1_epi16(0x0f0e)),\
          _vp = _mm256_shuffle_epi8(_mm256_permute2x128_si256(_vs_, _vs_, 0x11), _mm256_set1_epi16(0x0f0e));\
  _vs_ = _v_ = _ho_(_v_, _ho_(_vx, _vp));\
}

#define _MM256_SCAN_EPI16C(_v_,_vs_,_ho_) { const __m256i _cv =  _mm256_set_epi8(15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14, 15,14);\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 2));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 4));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 8));\
  __m256i   _vs = _mm256_shuffle_epi8(_mm256_permute2x128_si256(_v_, _v_, 0x00), _cv);\
  _v_ = _ho_(_v_,_mm256_blend_epi32(_mm256_setzero_si256(),_vs, 0xF0));\
  __m256i _vx = _mm256_shuffle_epi8(_mm256_broadcastsi128_si256(_mm256_castsi256_si128(_vs_)), _cv);\
  _vs_ = _v_ = _ho_(_v_, _vx);\
}

#define _MM256_SCAN_EPI32(_v_,_vs_,_ho_) {\
  _vs_ = _mm256_permutevar8x32_epi32(_vs_, _mm256_set1_epi32(7));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 4));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 8));\
  _v_ = _ho_(_v_, _mm256_inserti128_si256(_mm256_setzero_si256(), _mm_shuffle_epi32( _mm256_castsi256_si128(_v_), _MM_SHUFFLE(3,3,3,3)) , 1));\
  _vs_ = _v_ = _ho_(_v_, _vs_);\
}

#define _MM256_SCAN_EPI64xxx(_v_,_vs_,_ho_) {\
  _v_ = _ho_(_v_, _mm256_alignr_epi8(_v_, _mm256_permute2x128_si256(_v_, _v_, _MM_SHUFFLE(0, 0, 2, 0)), 8));\
  _vs_ = _v_ = _ho_(_mm256_permute4x64_epi64(_vs_, _MM_SHUFFLE(3, 3, 3, 3)), _ho_(_mm256_permute2x128_si256(_v_, _v_, _MM_SHUFFLE(0, 0, 2, 0)), _v_) );\
}

#define _MM256_SCAN_EPI64(_v_,_vs_,_ho_) {\
  _vs_ = _mm256_permute4x64_epi64(_vs_, _MM_SHUFFLE(3,3,3,3));\
  _v_ = _ho_(_v_, _mm256_slli_si256(_v_, 8));\
  _v_ = _ho_(_v_, _mm256_inserti128_si256(_mm256_setzero_si256(), _mm_shuffle_epi32( _mm256_castsi256_si128(_v_), _MM_SHUFFLE(3,2,3,2)) , 1));\
  _vs_ = _v_ = _ho_(_v_, _vs_);\
}

#define MM256_SCAN_EPI8( _v_,_vs_) _MM256_SCAN_EPI8( _v_,_vs_,_mm256_add_epi8)
#define MM256_SCAN_EPI16(_v_,_vs_) _MM256_SCAN_EPI16(_v_,_vs_,_mm256_add_epi16)
#define MM256_SCAN_EPI32(_v_,_vs_) _MM256_SCAN_EPI32(_v_,_vs_,_mm256_add_epi32)
#define MM256_SCAN_EPI64(_v_,_vs_) _MM256_SCAN_EPI64(_v_,_vs_,_mm256_add_epi64)
static ALWAYS_INLINE __m256i mm256_scan_epi8( __m256i v, __m256i vs) { MM256_SCAN_EPI8( v,vs); return vs; }
static ALWAYS_INLINE __m256i mm256_scan_epi16(__m256i v, __m256i vs) { MM256_SCAN_EPI16(v,vs); return vs; }
static ALWAYS_INLINE __m256i mm256_scan_epi32(__m256i v, __m256i vs) { MM256_SCAN_EPI32(v,vs); return vs; }
static ALWAYS_INLINE __m256i mm256_scan_epi64(__m256i v, __m256i vs) { MM256_SCAN_EPI64(v,vs); return vs; }

// ---- xor ----------
static ALWAYS_INLINE __m256i mm256_xore_epi16(__m256i v, __m256i vs) { return _mm256_xor_si256(v, _mm256_alignr_epi8(v, _mm256_permute2x128_si256(vs, v, 0x21), 14)); }
static ALWAYS_INLINE __m256i mm256_xore_epi32(__m256i v, __m256i vs) { return _mm256_xor_si256(v, _mm256_alignr_epi8(v, _mm256_permute2f128_si256(vs, v, _MM_SHUFFLE(0, 2, 0, 1)), 12)); }
static ALWAYS_INLINE __m256i mm256_xore_epi64(__m256i v, __m256i vs) { return _mm256_xor_si256(v, _mm256_alignr_epi8(v, _mm256_permute2f128_si256(vs, v, _MM_SHUFFLE(0, 2, 0, 1)),  8)); }

#define MM256_XORD_EPI8( _v_,_vs_) _MM256_SCAN_EPI8( _v_,_vs_,_mm256_xor_si256)
#define MM256_XORD_EPI16(_v_,_vs_) _MM256_SCAN_EPI16(_v_,_vs_,_mm256_xor_si256)
#define MM256_XORD_EPI32(_v_,_vs_) _MM256_SCAN_EPI32(_v_,_vs_,_mm256_xor_si256)
#define MM256_XORD_EPI64(_v_,_vs_) _MM256_SCAN_EPI64(_v_,_vs_,_mm256_xor_si256)
static ALWAYS_INLINE __m256i mm256_xord_epi8( __m256i v, __m256i vs) { MM256_XORD_EPI8( v,vs); return vs; }
static ALWAYS_INLINE __m256i mm256_xord_epi16(__m256i v, __m256i vs) { MM256_XORD_EPI16(v,vs); return vs; }
static ALWAYS_INLINE __m256i mm256_xord_epi32(__m256i v, __m256i vs) { MM256_XORD_EPI32(v,vs); return vs; }
static ALWAYS_INLINE __m256i mm256_xord_epi64(__m256i v, __m256i vs) { MM256_SCAN_EPI64(v,vs); return vs; }

// --- zigzag delta ---------
#define MM256_SCANZ_EPI8( v, vs) { v = mm256_zzagd_epi8(v);  MM256_SCAN_EPI8( v, vs); }
#define MM256_SCANZ_EPI16(v, vs) { v = mm256_zzagd_epi16(v); MM256_SCAN_EPI16(v, vs); }
#define MM256_SCANZ_EPI32(v, vs) { v = mm256_zzagd_epi32(v); MM256_SCAN_EPI32(v, vs); }
#define MM256_SCANZ_EPI64(v, vs) { v = mm256_zzagd_epi64(v); MM256_SCAN_EPI64(v, vs); }

// --- delta 1 -------------
#define MM256_SCAN1_EPI8( v, vs)  { const __m256i _cv = _mm256_set_epi8(32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1); MM256_SCAN_EPI8( v, vs); vs = v = _mm256_add_epi8( v, _cv); }
#define MM256_SCAN1_EPI16(v, vs)  { const __m256i _cv = _mm256_set_epi16(                                               16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1); MM256_SCAN_EPI16(v, vs); vs = v = _mm256_add_epi16(v, _cv); }
#define MM256_SCAN1_EPI32(v, vs)  { const __m256i _cv = _mm256_set_epi32(                                                                        8, 7, 6, 5, 4, 3, 2, 1); MM256_SCAN_EPI32(v, vs); vs = v = _mm256_add_epi32(v, _cv); }

#define MM256_SCAN1z_EPI8( v, vs) { const __m256i _cv = _mm256_set_epi8(32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1), csv = _mm256_set1_epi8( 32); v = _mm256_add_epi8( vs, _cv); vs = _mm256_add_epi8( vs, csv);}
#define MM256_SCAN1z_EPI16(v, vs) { const __m256i _cv = _mm256_set_epi16(                                               16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1), csv = _mm256_set1_epi16(16); v = _mm256_add_epi16(vs, _cv); vs = _mm256_add_epi16(vs, csv);}
#define MM256_SCAN1z_EPI32(v, vs) { const __m256i _cv = _mm256_set_epi32(                                                                        8, 7, 6, 5, 4, 3, 2, 1), csv = _mm256_set1_epi32( 8); v = _mm256_add_epi32(vs, _cv); vs = _mm256_add_epi32(vs, csv);}

// --- delta N --------------
static ALWAYS_INLINE __m256i mm256_scani_epi16(__m256i v, __m256i vs, __m256i vi) { return _mm256_add_epi16(mm256_scan_epi16(v, vs), vi); }
static ALWAYS_INLINE __m256i mm256_scani_epi32(__m256i v, __m256i vs, __m256i vi) { return _mm256_add_epi32(mm256_scan_epi32(v, vs), vi); }

//--------------------------------------------------------- Quad v0,v1,v2,v3 ----------------------------
#define MM256_ZZAGEQ_EPI32(v0, v1, v2, v3) { \
  __m256i _v0 = _mm256_srai_epi32(v0,31),\
          _v1 = _mm256_srai_epi32(v1,31),\
          _v2 = _mm256_srai_epi32(v2,31),\
          _v3 = _mm256_srai_epi32(v3,31);\
  v0 = _mm256_slli_epi32(v0,1);\
  v1 = _mm256_slli_epi32(v1,1);\
  v2 = _mm256_slli_epi32(v2,1);\
  v3 = _mm256_slli_epi32(v3,1); \
  v0 = _mm256_xor_si256(v0, _v0);\
  v1 = _mm256_xor_si256(v1, _v1);\
  v2 = _mm256_xor_si256(v2, _v2);\
  v3 = _mm256_xor_si256(v3, _v3);\
}

#define MM256_ZZAGDQ_EPI16(v0, v1, v2, v3) { \
  __m256i _v0 = _mm256_slli_epi16(v0, 15),\
          _v1 = _mm256_slli_epi16(v1, 15),\
          _v2 = _mm256_slli_epi16(v2, 15),\
          _v3 = _mm256_slli_epi16(v3, 15);\
   v0 = _mm256_srli_epi16(v0, 1);\
   v1 = _mm256_srli_epi16(v1, 1);\
   v2 = _mm256_srli_epi16(v2, 1);\
   v3 = _mm256_srli_epi16(v3, 1);\
  _v0 = _mm256_srai_epi16(_v0, 15);\
  _v1 = _mm256_srai_epi16(_v1, 15);\
  _v2 = _mm256_srai_epi16(_v2, 15);\
  _v3 = _mm256_srai_epi16(_v3, 15);\
   v0 = _mm256_xor_si256(v0, _v0);\
   v1 = _mm256_xor_si256(v1, _v1);\
   v2 = _mm256_xor_si256(v2, _v2);\
   v3 = _mm256_xor_si256(v3, _v3);\
}

#define MM256_ZZAGDQ_EPI32(v0, v1, v2, v3) { \
  __m256i _v0 = _mm256_slli_epi32(v0, 31),\
          _v1 = _mm256_slli_epi32(v1, 31),\
          _v2 = _mm256_slli_epi32(v2, 31),\
          _v3 = _mm256_slli_epi32(v3, 31);\
   v0 = _mm256_srli_epi32(v0, 1);\
   v1 = _mm256_srli_epi32(v1, 1);\
   v2 = _mm256_srli_epi32(v2, 1);\
   v3 = _mm256_srli_epi32(v3, 1);\
  _v0 = _mm256_srai_epi32(_v0, 31);\
  _v1 = _mm256_srai_epi32(_v1, 31);\
  _v2 = _mm256_srai_epi32(_v2, 31);\
  _v3 = _mm256_srai_epi32(_v3, 31);\
   v0 = _mm256_xor_si256(v0, _v0);\
   v1 = _mm256_xor_si256(v1, _v1);\
   v2 = _mm256_xor_si256(v2, _v2);\
   v3 = _mm256_xor_si256(v3, _v3);\
}

#define MM256_ZZAGDQ_EPI64(v0, v1, v2, v3) { \
  __m256i _v0 = _mm256_slli_epi64(v0, 63),\
          _v1 = _mm256_slli_epi64(v1, 63),\
          _v2 = _mm256_slli_epi64(v2, 63),\
          _v3 = _mm256_slli_epi64(v3, 63);\
   v0 = _mm256_srli_epi64(v0, 1);\
   v1 = _mm256_srli_epi64(v1, 1);\
   v2 = _mm256_srli_epi64(v2, 1);\
   v3 = _mm256_srli_epi64(v3, 1);\
  _v0 =  mm256_srai_epi64_63(_v0);\
  _v1 =  mm256_srai_epi64_63(_v1);\
  _v2 =  mm256_srai_epi64_63(_v2);\
  _v3 =  mm256_srai_epi64_63(_v3);\
   v0 = _mm256_xor_si256(v0, _v0);\
   v1 = _mm256_xor_si256(v1, _v1);\
   v2 = _mm256_xor_si256(v2, _v2);\
   v3 = _mm256_xor_si256(v3, _v3);\
}

#define MM256_DELTAQ_EPI32(v0, v1, v2, v3, vs) {\
  __m256i _v0 = _mm256_permute2x128_si256(vs, v0, 0x21),\
          _v1 = _mm256_permute2x128_si256(v0, v1, 0x21),\
          _v2 = _mm256_permute2x128_si256(v1, v2, 0x21),\
          _v3 = _mm256_permute2x128_si256(v2, v3, 0x21); vs = v3;\
  v0 = _mm256_sub_epi32(v0, _mm256_alignr_epi8(v0, _v0, 12)); \
  v1 = _mm256_sub_epi32(v1, _mm256_alignr_epi8(v1, _v1, 12));\
  v2 = _mm256_sub_epi32(v2, _mm256_alignr_epi8(v2, _v2, 12));\
  v3 = _mm256_sub_epi32(v3, _mm256_alignr_epi8(v3, _v3, 12));\
}

#define MM256_DELTA1Q_EPI32(v0, v1, v2, v3, vs) {\
  const __m256i _cv = _mm256_set1_epi32(1);\
  __m256i _v0 = _mm256_permute2x128_si256(vs, v0, 0x21),\
          _v1 = _mm256_permute2x128_si256(v0, v1, 0x21),\
          _v2 = _mm256_permute2x128_si256(v1, v2, 0x21),\
          _v3 = _mm256_permute2x128_si256(v2, v3, 0x21); vs = v3;\
  v0 = _mm256_sub_epi32(_mm256_sub_epi32(v0, _mm256_alignr_epi8(v0, _v0, 12)), _cv);\
  v1 = _mm256_sub_epi32(_mm256_sub_epi32(v1, _mm256_alignr_epi8(v1, _v1, 12)), _cv);\
  v2 = _mm256_sub_epi32(_mm256_sub_epi32(v2, _mm256_alignr_epi8(v2, _v2, 12)), _cv);\
  v3 = _mm256_sub_epi32(_mm256_sub_epi32(v3, _mm256_alignr_epi8(v3, _v3, 12)), _cv);\
}

#define MM256_DELTAZQ_EPI32(v0, v1, v2, v3, vs) { MM256_DELTAQ_EPI32(v0, v1, v2, v3, vs); MM256_ZZAGEQ_EPI32(v0, v1, v2, v3); } 

#define _MM256_SCANQ_EPI16(v0, v1, v2, v3, vs, _ho_) {\
  const __m256i       zv = _mm256_setzero_si256(), pidx = _mm256_set1_epi32(7),\
                bcast_hi = _mm256_set1_epi32(0x03020302);\
  const __m128i bcast_lo = _mm_set1_epi32(0x0F0E0F0E);\
  v0 = _ho_(v0, _mm256_slli_si256(v0, 2));\
  v1 = _ho_(v1, _mm256_slli_si256(v1, 2));\
  v2 = _ho_(v2, _mm256_slli_si256(v2, 2));\
  v3 = _ho_(v3, _mm256_slli_si256(v3, 2));\
  v0 = _ho_(v0, _mm256_slli_si256(v0, 4));\
  v1 = _ho_(v1, _mm256_slli_si256(v1, 4));\
  v2 = _ho_(v2, _mm256_slli_si256(v2, 4));\
  v3 = _ho_(v3, _mm256_slli_si256(v3, 4));\
  v0 = _ho_(v0, _mm256_slli_si256(v0, 8));\
  v1 = _ho_(v1, _mm256_slli_si256(v1, 8));\
  v2 = _ho_(v2, _mm256_slli_si256(v2, 8));\
  v3 = _ho_(v3, _mm256_slli_si256(v3, 8));\
  __m128i lo0 = _mm_shuffle_epi8(_mm256_castsi256_si128(v0), bcast_lo),\
          lo1 = _mm_shuffle_epi8(_mm256_castsi256_si128(v1), bcast_lo),\
          lo2 = _mm_shuffle_epi8(_mm256_castsi256_si128(v2), bcast_lo),\
          lo3 = _mm_shuffle_epi8(_mm256_castsi256_si128(v3), bcast_lo);\
  v0 = _ho_(v0, _mm256_inserti128_si256(zv, lo0, 1));\
  v1 = _ho_(v1, _mm256_inserti128_si256(zv, lo1, 1));\
  v2 = _ho_(v2, _mm256_inserti128_si256(zv, lo2, 1));\
  v3 = _ho_(v3, _mm256_inserti128_si256(zv, lo3, 1));\
  __m256i s0 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(v0, pidx), bcast_hi),\
          s1 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(v1, pidx), bcast_hi),\
          s2 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(v2, pidx), bcast_hi),\
          s3 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(v3, pidx), bcast_hi),\
          c1 = _ho_(vs, s0),\
          c2 = _ho_(c1, s1),\
          c3 = _ho_(c2, s2);\
  v0 = _ho_(v0, vs);\
  v1 = _ho_(v1, c1);\
  v2 = _ho_(v2, c2);\
  v3 = _ho_(v3, c3);\
  vs = _ho_(c3, s3);\
}

#define _MM256_SCANQ_EPI32(v0, v1, v2, v3, vs, _ho_) {\
  const __m256i zv = _mm256_setzero_si256(), pidx = _mm256_set1_epi32(7);\
  v0 = _ho_(v0, _mm256_slli_si256(v0, 4));\
  v1 = _ho_(v1, _mm256_slli_si256(v1, 4));\
  v2 = _ho_(v2, _mm256_slli_si256(v2, 4));\
  v3 = _ho_(v3, _mm256_slli_si256(v3, 4));\
  v0 = _ho_(v0, _mm256_slli_si256(v0, 8));\
  v1 = _ho_(v1, _mm256_slli_si256(v1, 8));\
  v2 = _ho_(v2, _mm256_slli_si256(v2, 8));\
  v3 = _ho_(v3, _mm256_slli_si256(v3, 8));\
  __m128i lo0 = _mm_shuffle_epi32(_mm256_castsi256_si128(v0), 0xff),\
          lo1 = _mm_shuffle_epi32(_mm256_castsi256_si128(v1), 0xff),\
          lo2 = _mm_shuffle_epi32(_mm256_castsi256_si128(v2), 0xff),\
          lo3 = _mm_shuffle_epi32(_mm256_castsi256_si128(v3), 0xff);\
  v0 = _ho_(v0, _mm256_inserti128_si256(zv, lo0, 1));\
  v1 = _ho_(v1, _mm256_inserti128_si256(zv, lo1, 1));\
  v2 = _ho_(v2, _mm256_inserti128_si256(zv, lo2, 1));\
  v3 = _ho_(v3, _mm256_inserti128_si256(zv, lo3, 1));\
  __m256i s0 = _mm256_permutevar8x32_epi32(v0, pidx),\
          s1 = _mm256_permutevar8x32_epi32(v1, pidx),\
          s2 = _mm256_permutevar8x32_epi32(v2, pidx),\
          s3 = _mm256_permutevar8x32_epi32(v3, pidx),\
          c1 = _ho_(vs, s0),\
          c2 = _ho_(c1, s1),\
          c3 = _ho_(c2, s2);\
  v0 = _ho_(v0, vs);\
  v1 = _ho_(v1, c1);\
  v2 = _ho_(v2, c2);\
  v3 = _ho_(v3, c3);\
  vs = _ho_(c3, s3);\
}

#define _MM256_SCANQ_EPI64(v0, v1, v2, v3, vs, _ho_) { \
  v0 = _ho_(v0, _mm256_slli_si256(v0, 8)); \
  v1 = _ho_(v1, _mm256_slli_si256(v1, 8)); \
  v2 = _ho_(v2, _mm256_slli_si256(v2, 8)); \
  v3 = _ho_(v3, _mm256_slli_si256(v3, 8)); \
  v0 = _ho_(v0, _mm256_inserti128_si256(_mm256_setzero_si256(), _mm_shuffle_epi32(_mm256_castsi256_si128(v0), _MM_SHUFFLE(3,2,3,2)), 1)); \
  v1 = _ho_(v1, _mm256_inserti128_si256(_mm256_setzero_si256(), _mm_shuffle_epi32(_mm256_castsi256_si128(v1), _MM_SHUFFLE(3,2,3,2)), 1)); \
  v2 = _ho_(v2, _mm256_inserti128_si256(_mm256_setzero_si256(), _mm_shuffle_epi32(_mm256_castsi256_si128(v2), _MM_SHUFFLE(3,2,3,2)), 1)); \
  v3 = _ho_(v3, _mm256_inserti128_si256(_mm256_setzero_si256(), _mm_shuffle_epi32(_mm256_castsi256_si128(v3), _MM_SHUFFLE(3,2,3,2)), 1)); \
  __m256i _v0 = _mm256_permute4x64_epi64(v0, _MM_SHUFFLE(3,3,3,3)), \
          _v1 = _mm256_permute4x64_epi64(v1, _MM_SHUFFLE(3,3,3,3)), \
          _v2 = _mm256_permute4x64_epi64(v2, _MM_SHUFFLE(3,3,3,3)); \
  vs = _mm256_permute4x64_epi64(vs, _MM_SHUFFLE(3,3,3,3)); \
  v0 = _ho_(v0, vs); \
  vs = _ho_(vs, _v0); \
  v1 = _ho_(v1, vs); \
  vs = _ho_(vs, _v1); \
  v2 = _ho_(v2, vs); \
  vs = _ho_(vs, _v2); \
  vs = v3 = _ho_(v3, vs); \
}

#define MM256_SCANQ_EPI16(v0, v1, v2, v3, vs) _MM256_SCANQ_EPI16(v0, v1, v2, v3, vs, _mm256_add_epi16)
#define MM256_SCANQ_EPI32(v0, v1, v2, v3, vs) _MM256_SCANQ_EPI32(v0, v1, v2, v3, vs, _mm256_add_epi32)
#define MM256_SCANQ_EPI64(v0, v1, v2, v3, vs) _MM256_SCANQ_EPI64(v0, v1, v2, v3, vs, _mm256_add_epi64)

#define MM256_XORDQ_EPI16(v0, v1, v2, v3, vs) _MM256_SCANQ_EPI16(v0, v1, v2, v3, vs, _mm256_xor_si256)
#define MM256_XORDQ_EPI32(v0, v1, v2, v3, vs) _MM256_SCANQ_EPI32(v0, v1, v2, v3, vs, _mm256_xor_si256)
#define MM256_XORDQ_EPI64(v0, v1, v2, v3, vs) _MM256_SCANQ_EPI64(v0, v1, v2, v3, vs, _mm256_xor_si256)

#define MM256_SCANZQ_EPI16(v0, v1, v2, v3, vs) { MM256_ZZAGDQ_EPI16(v0, v1, v2, v3); MM256_SCANQ_EPI16(v0, v1, v2, v3, vs); }
#define MM256_SCANZQ_EPI32(v0, v1, v2, v3, vs) { MM256_ZZAGDQ_EPI32(v0, v1, v2, v3); MM256_SCANQ_EPI32(v0, v1, v2, v3, vs); }
#define MM256_SCANZQ_EPI64(v0, v1, v2, v3, vs) { MM256_ZZAGDQ_EPI64(v0, v1, v2, v3); MM256_SCANQ_EPI64(v0, v1, v2, v3, vs); }

#define _MM256_SCAN1Q_EPI16(v0, v1, v2, v3, vs) {\
  const __m256i _cv0 = _mm256_set_epi16(16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1),\
                _cv1 = _mm256_set_epi16(32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17),\
                _cv2 = _mm256_set_epi16(48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33),\
                _cv3 = _mm256_set_epi16(64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49),\
				_cvs = _mm256_set1_epi16(64);\
  v0 = _mm256_add_epi16(v0, _cv0);\
  v1 = _mm256_add_epi16(v1, _cv1);\
  v2 = _mm256_add_epi16(v2, _cv2);\
  v3 = _mm256_add_epi16(v3, _cv3);\
  vs = _mm256_add_epi16(vs, _cvs);\
}

#define _MM256_SCAN1Q_EPI32(v0, v1, v2, v3, vs) {\
  const __m256i _cv0 = _mm256_set_epi32( 8, 7,  6,  5,  4,  3,  2, 1),\
                _cv1 = _mm256_set_epi32(16, 15, 14, 13, 12, 11, 10, 9),\
                _cv2 = _mm256_set_epi32(24, 23, 22, 21, 20, 19, 18, 17),\
                _cv3 = _mm256_set_epi32(32, 31, 30, 29, 28, 27, 26, 25),\
				_cvs = _mm256_set1_epi32(32);\
  v0 = _mm256_add_epi32(v0, _cv0);\
  v1 = _mm256_add_epi32(v1, _cv1);\
  v2 = _mm256_add_epi32(v2, _cv2);\
  v3 = _mm256_add_epi32(v3, _cv3);\
  vs = _mm256_add_epi32(vs, _cvs);\
}

#define MM256_SCAN1Q_EPI16( v0, v1, v2, v3, vs) { MM256_SCANQ_EPI16(v0, v1, v2, v3, vs); _MM256_SCAN1Q_EPI16(v0, v1, v2, v3, vs); }
#define MM256_SCAN1Q_EPI32( v0, v1, v2, v3, vs) { MM256_SCANQ_EPI32(v0, v1, v2, v3, vs); _MM256_SCAN1Q_EPI32(v0, v1, v2, v3, vs); }

#define MM256_SCAN1zQ_EPI16(v0, v1, v2, v3, vs) {\
  const __m256i _cv0 = _mm256_set_epi16(16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1),\
                _cv1 = _mm256_set_epi16(32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17),\
                _cv2 = _mm256_set_epi16(48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33),\
                _cv3 = _mm256_set_epi16(64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49),\
				_cvs = _mm256_set1_epi16(64);\
  v0 = _mm256_add_epi16(vs, _cv0);\
  v1 = _mm256_add_epi16(vs, _cv1);\
  v2 = _mm256_add_epi16(vs, _cv2);\
  v3 = _mm256_add_epi16(vs, _cv3);\
  vs = _mm256_add_epi16(vs, _cvs);\
}

#define MM256_SCAN1zQ_EPI32(v0, v1, v2, v3, vs) {\
  const __m256i _cv0 = _mm256_set_epi32( 8,  7,  6,  5,  4,  3,  2,  1),\
                _cv1 = _mm256_set_epi32(16, 15, 14, 13, 12, 11, 10,  9),\
                _cv2 = _mm256_set_epi32(24, 23, 22, 21, 20, 19, 18, 17),\
                _cv3 = _mm256_set_epi32(32, 31, 30, 29, 28, 27, 26, 25),\
				_cvs = _mm256_set1_epi32(32);\
  v0 = _mm256_add_epi32(vs, _cv0);\
  v1 = _mm256_add_epi32(vs, _cv1);\
  v2 = _mm256_add_epi32(vs, _cv2);\
  v3 = _mm256_add_epi32(vs, _cv3);\
  vs = _mm256_add_epi32(vs, _cvs);\
}

#define MM256_SCANIQ_EPI32(v0, v1, v2, v3, vs, vi) {\
  __m256i const zv = _mm256_setzero_si256(), perm_idx = _mm256_set1_epi32(7);\
  vs = _mm256_permutevar8x32_epi32(vs, perm_idx);\
  v0 = _mm256_add_epi32(v0, _mm256_slli_si256(v0, 4));\
  v1 = _mm256_add_epi32(v1, _mm256_slli_si256(v1, 4));\
  v2 = _mm256_add_epi32(v2, _mm256_slli_si256(v2, 4));\
  v3 = _mm256_add_epi32(v3, _mm256_slli_si256(v3, 4));\
\
  v0 = _mm256_add_epi32(v0, _mm256_slli_si256(v0, 8));\
  v1 = _mm256_add_epi32(v1, _mm256_slli_si256(v1, 8));\
  v2 = _mm256_add_epi32(v2, _mm256_slli_si256(v2, 8));\
  v3 = _mm256_add_epi32(v3, _mm256_slli_si256(v3, 8));\
\
  v0 = _mm256_add_epi32(v0, _mm256_shuffle_epi32(_mm256_permute2x128_si256(v0, v0, 0x08), 0xFF));\
  v1 = _mm256_add_epi32(v1, _mm256_shuffle_epi32(_mm256_permute2x128_si256(v1, v1, 0x08), 0xFF));\
  v2 = _mm256_add_epi32(v2, _mm256_shuffle_epi32(_mm256_permute2x128_si256(v2, v2, 0x08), 0xFF));\
  v3 = _mm256_add_epi32(v3, _mm256_shuffle_epi32(_mm256_permute2x128_si256(v3, v3, 0x08), 0xFF));\
\
  v0 = _mm256_add_epi32(v0, vs);      v0 = _mm256_add_epi32(v0, vi); vs = _mm256_permutevar8x32_epi32(v0, perm_idx);\
  v1 = _mm256_add_epi32(v1, vs);      v1 = _mm256_add_epi32(v1, vi); vs = _mm256_permutevar8x32_epi32(v1, perm_idx);\
  v2 = _mm256_add_epi32(v2, vs);      v2 = _mm256_add_epi32(v2, vi); vs = _mm256_permutevar8x32_epi32(v2, perm_idx);\
  v3 = _mm256_add_epi32(v3, vs); vs = v3 = _mm256_add_epi32(v3, vi);\
}
static ALWAYS_INLINE void mm256_storeuq_si256(__m256i *op, __m256i v0, __m256i v1, __m256i v2, __m256i v3) {
  _mm256_storeu_si256((__m256i*)         op,     v0);     
  _mm256_storeu_si256((__m256i*)((char *)op+32), v1);
  _mm256_storeu_si256((__m256i*)((char *)op+64), v2);
  _mm256_storeu_si256((__m256i*)((char *)op+96), v3);
}
#endif

//*************************************************** SIMD 128 bits *****************************************************************
#if defined(__SSSE3__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)
  #ifdef __SSE4_1__
#define MM_HOZ_EPI16(v,_ho_) {\
  v = _ho_(v, _mm_srli_si128(v, 8));\
  v = _ho_(v, _mm_srli_si128(v, 4));\
  v = _ho_(v, _mm_srli_si128(v, 2));\
  return (uint16_t)_mm_extract_epi16(v, 0);\
}
  #else
#define MM_HOZ_EPI16(v,_ho_) {\
  v = _ho_(v, _mm_srli_si128(v, 8));\
  v = _ho_(v, _mm_srli_si128(v, 4));\
  v = _ho_(v, _mm_srli_si128(v, 2));\
  return (unsigned short)_mm_cvtsi128_si32(v);\
}
  #endif

#define MM_HOZ_EPI32(v,_ho_) {\
  v = _ho_(v, _mm_srli_si128(v, 8));\
  v = _ho_(v, _mm_srli_si128(v, 4));\
  return (unsigned)_mm_cvtsi128_si32(v);\
}

static ALWAYS_INLINE uint16_t mm_hor_epi16( __m128i v) { MM_HOZ_EPI16(v,_mm_or_si128); }
static ALWAYS_INLINE uint32_t mm_hor_epi32( __m128i v) { MM_HOZ_EPI32(v,_mm_or_si128); }
static ALWAYS_INLINE uint64_t mm_hor_epi64( __m128i v) { v = _mm_or_si128( v, _mm_srli_si128(v, 8)); return (uint64_t      )_mm_cvtsi128_si64(v); }

static ALWAYS_INLINE uint8_t   mm_cvtsi128_si8 (__m128i v) { return (uint8_t )_mm_cvtsi128_si32(v); }
static ALWAYS_INLINE uint16_t  mm_cvtsi128_si16(__m128i v) { return (uint16_t)_mm_cvtsi128_si32(v); }
#define mm_cvtsi128_si32(_v_) _mm_cvtsi128_si32(_v_)

static ALWAYS_INLINE __m128i mm_bsr_epi16(__m128i v) {
  __m128  flo  = _mm_cvtepi32_ps(_mm_unpacklo_epi16(v, _mm_setzero_si128()));
  __m128  fhi  = _mm_cvtepi32_ps(_mm_unpackhi_epi16(v, _mm_setzero_si128()));
  __m128i ilo  = mm_srli_epi32(_mm_castps_si128(flo), 23);
  __m128i ihi  = mm_srli_epi32(_mm_castps_si128(fhi), 23);
  __m128i bias = _mm_set1_epi32(126);
           ilo = _mm_sub_epi32(ilo, bias);
           ihi = _mm_sub_epi32(ihi, bias);
  return _mm_max_epi16(_mm_packs_epi32(ilo, ihi), _mm_setzero_si128());
}

static ALWAYS_INLINE __m128i mm_bsr_epi32(__m128i v) {
  __m128i vm =  mm_srli_epi32(v, 8),
           e =  mm_srli_epi32(_mm_castps_si128(_mm_cvtepi32_ps(_mm_andnot_si128(vm, v))), 23);
           e = _mm_subs_epu16(e, _mm_set1_epi32(126));
           e = _mm_min_epi16(e, _mm_set1_epi32(32));
  return e;
}

// --- 32 <-> 64 ----
#if defined(__SSE4_1__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)
#define mm_cvt32_epu64(_v_, _vl_) { _vl_ = _mm_cvtepu32_epi64(_v_); _v_  = _mm_cvtepu32_epi64(_mm_srli_si128(_v_, 8)); }
#else
#define mm_cvt32_epu64(_v_, _vl_) { const __m128i _zv = _mm_setzero_si128(); _vl_ = _mm_unpacklo_epi32(_v_, _zv);  _v_  = _mm_unpackhi_epi32(_v_, _zv); }
#endif

static ALWAYS_INLINE __m128i mm_cvt64_epu32(__m128i _vh_, __m128i _vl_) {
  return _mm_unpacklo_epi64(mm_shuffle_2020_epi32(_vl_), mm_shuffle_2020_epi32(_vh_));
}

  #if defined(__SSE4_1__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)
static ALWAYS_INLINE __m128i mm_zzagd_epi8( __m128i v) { const __m128i _cz = _mm_setzero_si128();
  __m128i _va = _mm_avg_epu8(v, _cz);
  return _mm_blendv_epi8(_va, _mm_sub_epi8(_cz, _va), mm_slli_epi16(v, 7));
}
  #elif defined(__SSSE3__)
static ALWAYS_INLINE __m128i mm_zzagd_epi8(__m128i v) { 
  return _mm_xor_si128(_mm_and_si128(mm_srli_epi16(v, 1), _mm_set1_epi8(0x7F)), _mm_cmpgt_epi8(_mm_setzero_si128(), mm_slli_epi16(v, 7)));
}
  #endif
  
static ALWAYS_INLINE __m128i mm_zzage_epi16(__m128i v) { return _mm_xor_si128( mm_slli_epi16(v,1),  mm_srai_epi16(   v,15)); }
static ALWAYS_INLINE __m128i mm_zzage_epi32(__m128i v) { return _mm_xor_si128( mm_slli_epi32(v,1),  mm_srai_epi32(   v,31)); }
static ALWAYS_INLINE __m128i mm_zzage_epi64(__m128i v) { return _mm_xor_si128( mm_slli_epi64(v,1),  mm_srai_epi64_63(v)); }

static ALWAYS_INLINE __m128i mm_zzagd_epi16(__m128i v) { return _mm_xor_si128( mm_srli_epi16(v,1),  mm_srai_epi16(   mm_slli_epi16(v,15),15)); }
static ALWAYS_INLINE __m128i mm_zzagd_epi32(__m128i v) { return _mm_xor_si128( mm_srli_epi32(v,1),  mm_srai_epi32(   mm_slli_epi32(v,31),31)); }
static ALWAYS_INLINE __m128i mm_zzagd_epi64(__m128i v) { return _mm_xor_si128( mm_srli_epi64(v,1),  mm_srai_epi64_63(mm_slli_epi64(v,63)   )); }

//--- delta/xor encode/decode ---
#if defined(__SSSE3__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)
static ALWAYS_INLINE __m128i mm_delta_epi16(__m128i v, __m128i vs) { return _mm_sub_epi16(v, _mm_alignr_epi8(v, vs, 14)); }
static ALWAYS_INLINE __m128i mm_delta_epi32(__m128i v, __m128i vs) { return _mm_sub_epi32(v, _mm_alignr_epi8(v, vs, 12)); }
static ALWAYS_INLINE __m128i mm_delta_epi64(__m128i v, __m128i vs) { return _mm_sub_epi64(v, _mm_alignr_epi8(v, vs,  8)); }
static ALWAYS_INLINE __m128i mm_xore_epi16( __m128i v, __m128i vs) { return _mm_xor_si128(v, _mm_alignr_epi8(v, vs, 14)); }
static ALWAYS_INLINE __m128i mm_xore_epi32( __m128i v, __m128i vs) { return _mm_xor_si128(v, _mm_alignr_epi8(v, vs, 12)); }
static ALWAYS_INLINE __m128i mm_xore_epi64( __m128i v, __m128i vs) { return _mm_xor_si128(v, _mm_alignr_epi8(v, vs,  8)); }
#else
static ALWAYS_INLINE __m128i mm_delta_epi16(__m128i v, __m128i vs) { return _mm_sub_epi16(v, _mm_or_si128(_mm_srli_si128(vs, 14), mm_slli_si128(v, 2))); }
static ALWAYS_INLINE __m128i mm_xore_epi16( __m128i v, __m128i vs) { return _mm_xor_si128(v, _mm_or_si128(_mm_srli_si128(vs, 14), mm_slli_si128(v, 2))); }
static ALWAYS_INLINE __m128i mm_delta_epi32(__m128i v, __m128i vs) { return _mm_sub_epi32(v, _mm_or_si128(_mm_srli_si128(vs, 12), mm_slli_si128(v, 4))); }
static ALWAYS_INLINE __m128i mm_xore_epi32( __m128i v, __m128i vs) { return _mm_xor_si128(v, _mm_or_si128(_mm_srli_si128(vs, 12), mm_slli_si128(v, 4))); }
#endif

#define MM_DDELTA_EPI16(v, vs, vd) { __m128i _v = mm_delta_epi16(v, vs); vs = v; v = mm_delta_epi16(_v, vd); vd = _v; }
static ALWAYS_INLINE __m128i mm_ddelta_epi16( __m128i *v, __m128i vs, __m128i *vd) {  MM_DDELTA_EPI16(*v, vs, *vd); return vs; }

#define MM_DDELTAZ_EPI16(v, vs, vd) { MM_DDELTA_EPI16(v, vs, vd); v = mm_zzage_epi16(v); }

#define MM_DSCAN_EPI16(v, vs, vd)  { vd = mm_scan_epi16(v, vd); vs = v = mm_scan_epi16(vd, vs); }
#define MM_DSCANZ_EPI16(v, vs, vd) { v = mm_zzagd_epi16(v); MM_DSCAN_EPI16(v, vs, vd); }

#define MM_DDELTA_EPI32(v, vs, vd) { __m128i _v = mm_delta_epi32(v, vs); vs = v; v = mm_delta_epi32(_v, vd); vd = _v; }
static ALWAYS_INLINE __m128i mm_ddelta_epi32( __m128i *v, __m128i vs, __m128i *vd) {  MM_DDELTA_EPI32(*v, vs, *vd); return vs; }

#define MM_DDELTAZ_EPI32(v, vs, vd) { MM_DDELTA_EPI32(v, vs, vd); v = mm_zzage_epi32(v); }

#define MM_DSCAN_EPI32(v, vs, vd)  { vd = mm_scan_epi32(v, vd); vs = v = mm_scan_epi32(vd, vs); }
#define MM_DSCANZ_EPI32(v, vs, vd) { v = mm_zzagd_epi32(v); MM_DSCAN_EPI32(v, vs, vd); }

#define _MM_SCAN_EPI8(_v_,_vs_,_ho_) {\
  _v_  = _ho_(_v_, mm_slli_si128(_v_, 1));\
  _v_  = _ho_(_v_, mm_slli_si128(_v_, 2));\
  _v_  = _ho_(_v_, mm_slli_si128(_v_, 4)); \
  _vs_ = _v_  = _ho_(_ho_(_v_, mm_slli_si128(_v_, 8)), _mm_shuffle_epi8(_vs_, _mm_set1_epi8(15)));\
}

#define _MM_SCAN_EPI16(_v_,_vs_,_ho_) {\
  _v_  = _ho_(     _v_, mm_slli_si128(_v_, 2));\
  _v_  = _ho_(     _v_, mm_slli_si128(_v_, 4));\
  _vs_ = _v_  = _ho_(_ho_(_v_, mm_slli_si128(_v_, 8)), _mm_shuffle_epi8(_vs_, _mm_set1_epi16(0x0f0e)));\
}

#define _MM_SCAN_EPI32(_v_,_vs_,_ho_) {\
  _v_ = _ho_(_v_, mm_slli_si128(_v_, 4));\
  _vs_ = _v_ = _ho_(mm_shuffle_nnnn_epi32(_vs_, 3), _ho_(mm_slli_si128(_v_, 8), _v_));\
}

#define _MM_SCAN_EPI64(_v_,_vs_,_ho_) {\
  _v_ = _ho_(_v_, mm_slli_si128(_v_, 8));\
  _vs_ = _v_ = _ho_(_mm_shuffle_epi8(_vs_, _mm_set_epi8(15,14,13,12,11,10,9,8, 15,14,13,12,11,10,9,8)), _v_);\
}

#define MM_SCAN_EPI8( _v_,_vs_) _MM_SCAN_EPI8( _v_,_vs_,_mm_add_epi8 )
#define MM_SCAN_EPI16(_v_,_vs_) _MM_SCAN_EPI16(_v_,_vs_,_mm_add_epi16)
#define MM_SCAN_EPI32(_v_,_vs_) _MM_SCAN_EPI32(_v_,_vs_,_mm_add_epi32)
#define MM_SCAN_EPI64(_v_,_vs_) _MM_SCAN_EPI64(_v_,_vs_,_mm_add_epi64)
static ALWAYS_INLINE __m128i mm_scan_epi8( __m128i v, __m128i vs) { MM_SCAN_EPI8( v,vs); return vs; }
static ALWAYS_INLINE __m128i mm_scan_epi16(__m128i v, __m128i vs) { MM_SCAN_EPI16(v,vs); return vs; }
static ALWAYS_INLINE __m128i mm_scan_epi32(__m128i v, __m128i vs) { MM_SCAN_EPI32(v,vs); return vs; }
static ALWAYS_INLINE __m128i mm_scan_epi64(__m128i v, __m128i vs) { MM_SCAN_EPI64(v,vs); return vs; }
static ALWAYS_INLINE __m128i mm_scani_epi16(__m128i v, __m128i vs, __m128i vi) { return _mm_add_epi16(mm_scan_epi16(v, vs), vi); }
static ALWAYS_INLINE __m128i mm_scani_epi32(__m128i v, __m128i vs, __m128i vi) { return _mm_add_epi32(mm_scan_epi32(v, vs), vi); }

#define MM_XORD_EPI8( _v_,_vs_) _MM_SCAN_EPI8( _v_,_vs_,_mm_xor_si128)
#define MM_XORD_EPI16(_v_,_vs_) _MM_SCAN_EPI16(_v_,_vs_,_mm_xor_si128)
#define MM_XORD_EPI32(_v_,_vs_) _MM_SCAN_EPI32(_v_,_vs_,_mm_xor_si128)
#define MM_XORD_EPI64(_v_,_vs_) _MM_SCAN_EPI64(_v_,_vs_,_mm_xor_si128)
static ALWAYS_INLINE __m128i mm_xord_epi8( __m128i v, __m128i vs) { MM_XORD_EPI8( v,vs); return vs; }
static ALWAYS_INLINE __m128i mm_xord_epi16(__m128i v, __m128i vs) { MM_XORD_EPI16(v,vs); return vs; }
static ALWAYS_INLINE __m128i mm_xord_epi32(__m128i v, __m128i vs) { MM_XORD_EPI32(v,vs); return vs; }
static ALWAYS_INLINE __m128i mm_xord_epi64(__m128i v, __m128i vs) { MM_XORD_EPI64(v,vs); return vs; }

#define MM_SCANZ_EPI8( v, vs) { v = mm_zzagd_epi8( v); MM_SCAN_EPI8( v, vs); }
#define MM_SCANZ_EPI16(v, vs) { v = mm_zzagd_epi16(v); MM_SCAN_EPI16(v, vs); }
#define MM_SCANZ_EPI32(v, vs) { v = mm_zzagd_epi32(v); MM_SCAN_EPI32(v, vs); }
#define MM_SCANZ_EPI64(v, vs) { v = mm_zzagd_epi64(v); MM_SCAN_EPI64(v, vs); }

// --- delta 1 --------------
#define MM_SCAN1_EPI8( v, vs) { const __m128i _cv = _mm_set_epi8( 16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1); MM_SCAN_EPI8( v, vs); vs = v = _mm_add_epi8( v, _cv); }
#define MM_SCAN1_EPI16(v, vs) { const __m128i _cv = _mm_set_epi16(                         8, 7, 6, 5, 4, 3, 2, 1); MM_SCAN_EPI16(v, vs); vs = v = _mm_add_epi16(v, _cv); }
#define MM_SCAN1_EPI32(v, vs) { const __m128i _cv = _mm_set_epi32(                                     4, 3, 2, 1); MM_SCAN_EPI32(v, vs); vs = v = _mm_add_epi32(v, _cv); }

#define MM_SCAN1z_EPI8( v, vs) { const __m128i _cv = _mm_set_epi8(16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1), _csv = _mm_set1_epi8(16); v  = _mm_add_epi8( vs, _cv); vs = _mm_add_epi8( vs, _csv);}
#define MM_SCAN1z_EPI16(v, vs) { const __m128i _cv = _mm_set_epi16(                        8, 7, 6, 5, 4, 3, 2, 1), _csv = _mm_set1_epi16(8); v  = _mm_add_epi16(vs, _cv); vs = _mm_add_epi16(vs, _csv);}
#define MM_SCAN1z_EPI32(v, vs) { const __m128i _cv = _mm_set_epi32(                                    4, 3, 2, 1), _csv = _mm_set1_epi32(4); v  = _mm_add_epi32(vs, _cv); vs = _mm_add_epi32(vs, _csv);}

//------------------------------------------------- Quad v0,v1,v2,v3 -----------------------------------------------------------------------
#define MM_ZZAGEQ_EPI16(v0, v1, v2, v3) {\
  __m128i _v0 = mm_slli_epi16(v0, 1),\
		  _v1 = mm_slli_epi16(v1, 1),\
		  _v2 = mm_slli_epi16(v2, 1),\
		  _v3 = mm_slli_epi16(v3, 1);\
  v0 = mm_srai_epi16(v0, 15);\
  v1 = mm_srai_epi16(v1, 15);\
  v2 = mm_srai_epi16(v2, 15);\
  v3 = mm_srai_epi16(v3, 15);\
  v0 = _mm_xor_si128(v0, _v0);\
  v1 = _mm_xor_si128(v1, _v1);\
  v2 = _mm_xor_si128(v2, _v2);\
  v3 = _mm_xor_si128(v3, _v3);\
}

#define MM_ZZAGEQ_EPI32(v0, v1, v2, v3) {\
  __m128i _v0 = mm_slli_epi32(v0, 1),\
          _v1 = mm_slli_epi32(v1, 1),\
		  _v2 = mm_slli_epi32(v2, 1),\
		  _v3 = mm_slli_epi32(v3, 1);\
  v0 = mm_srai_epi32(v0, 31);\
  v1 = mm_srai_epi32(v1, 31);\
  v2 = mm_srai_epi32(v2, 31);\
  v3 = mm_srai_epi32(v3, 31);\
  v0 = _mm_xor_si128(v0, _v0);\
  v1 = _mm_xor_si128(v1, _v1);\
  v2 = _mm_xor_si128(v2, _v2);\
  v3 = _mm_xor_si128(v3, _v3);\
}

#define MM_ZZAGDQ_EPI16(v0, v1, v2, v3) {\
  __m128i m0 = mm_slli_epi16(v0, 15),\
          m1 = mm_slli_epi16(v1, 15),\
          m2 = mm_slli_epi16(v2, 15),\
          m3 = mm_slli_epi16(v3, 15),\
          u0 = mm_srli_epi16(v0, 1),\
          u1 = mm_srli_epi16(v1, 1),\
          u2 = mm_srli_epi16(v2, 1),\
          u3 = mm_srli_epi16(v3, 1);\
  m0 = mm_srai_epi16(m0, 15);\
  m1 = mm_srai_epi16(m1, 15);\
  m2 = mm_srai_epi16(m2, 15);\
  m3 = mm_srai_epi16(m3, 15);\
  v0 = _mm_xor_si128(u0, m0);\
  v1 = _mm_xor_si128(u1, m1);\
  v2 = _mm_xor_si128(u2, m2);\
  v3 = _mm_xor_si128(u3, m3);\
}

#define MM_ZZAGDQ_EPI32(v0, v1, v2, v3) {\
  __m128i m0 = mm_slli_epi32(v0, 31),\
          m1 = mm_slli_epi32(v1, 31),\
          m2 = mm_slli_epi32(v2, 31),\
          m3 = mm_slli_epi32(v3, 31);\
          v0 = mm_srli_epi32(v0, 1),\
          v1 = mm_srli_epi32(v1, 1),\
          v2 = mm_srli_epi32(v2, 1),\
          v3 = mm_srli_epi32(v3, 1);\
  m0 = mm_srai_epi32(m0, 31);\
  m1 = mm_srai_epi32(m1, 31);\
  m2 = mm_srai_epi32(m2, 31);\
  m3 = mm_srai_epi32(m3, 31);\
  v0 = _mm_xor_si128(v0, m0);\
  v1 = _mm_xor_si128(v1, m1);\
  v2 = _mm_xor_si128(v2, m2);\
  v3 = _mm_xor_si128(v3, m3);\
}

#define MM_ZZAGDQ_EPI64(v0, v1, v2, v3) {\
  __m128i m0 = mm_slli_epi64(v0,63),\
  m1 = mm_slli_epi64(v1,63),\
  m2 = mm_slli_epi64(v2,63),\
  m3 = mm_slli_epi64(v3,63);\
  v0 = mm_srli_epi64(v0,1);\
  v1 = mm_srli_epi64(v1,1);\
  v2 = mm_srli_epi64(v2,1);\
  v3 = mm_srli_epi64(v3,1);\
  m0 = mm_srai_epi64_63(m0);\
  m1 = mm_srai_epi64_63(m1);\
  m2 = mm_srai_epi64_63(m2);\
  m3 = mm_srai_epi64_63(m3);\
  v0 = _mm_xor_si128( v0, m0);\
  v1 = _mm_xor_si128( v1, m1);\
  v2 = _mm_xor_si128( v2, m2);\
  v3 = _mm_xor_si128( v3, m3);\
}

#define MM_DELTAQ_EPI16(v0, v1, v2, v3, vs) {\
  __m128i _v0 = _mm_alignr_epi8(v0, vs, 14),\
          _v1 = _mm_alignr_epi8(v1, v0, 14),\
          _v2 = _mm_alignr_epi8(v2, v1, 14),\
          _v3 = _mm_alignr_epi8(v3, v2, 14);\
  v0 = _mm_sub_epi16(v0, _v0);\
  v1 = _mm_sub_epi16(v1, _v1);\
  v2 = _mm_sub_epi16(v2, _v2); vs = v3;\
  v3 = _mm_sub_epi16(v3, _v3);\
}

#define MM_DELTAQ_EPI32(v0, v1, v2, v3, vs) {\
  __m128i _v0 = _mm_alignr_epi8(v0, vs, 12),\
          _v1 = _mm_alignr_epi8(v1, v0, 12),\
          _v2 = _mm_alignr_epi8(v2, v1, 12),\
          _v3 = _mm_alignr_epi8(v3, v2, 12);\
  v0 = _mm_sub_epi32(v0, _v0);\
  v1 = _mm_sub_epi32(v1, _v1);\
  v2 = _mm_sub_epi32(v2, _v2); vs = v3;\
  v3 = _mm_sub_epi32(v3, _v3);\
}

#define MM_DELTA1Q_EPI16(v0, v1, v2, v3, vs) {\
  const __m128i _cv = _mm_set1_epi16(1);\
  __m128i _v0 = _mm_alignr_epi8(v0, vs, 14),\
          _v1 = _mm_alignr_epi8(v1, v0, 14),\
          _v2 = _mm_alignr_epi8(v2, v1, 14),\
          _v3 = _mm_alignr_epi8(v3, v2, 14);\
  v0 = _mm_sub_epi16(_mm_sub_epi16(v0, _v0), _cv);\
  v1 = _mm_sub_epi16(_mm_sub_epi16(v1, _v1), _cv);\
  v2 = _mm_sub_epi16(_mm_sub_epi16(v2, _v2), _cv); vs = v3;\
  v3 = _mm_sub_epi16(_mm_sub_epi16(v3, _v3), _cv); \
}

#define MM_DELTA1Q_EPI32(v0, v1, v2, v3, vs) {\
  const __m128i _cv = _mm_set1_epi32(1);\
  __m128i _v0 = _mm_alignr_epi8(v0, vs, 12),\
          _v1 = _mm_alignr_epi8(v1, v0, 12),\
          _v2 = _mm_alignr_epi8(v2, v1, 12),\
          _v3 = _mm_alignr_epi8(v3, v2, 12);\
  v0 = _mm_sub_epi32(_mm_sub_epi32(v0, _v0), _cv);\
  v1 = _mm_sub_epi32(_mm_sub_epi32(v1, _v1), _cv);\
  v2 = _mm_sub_epi32(_mm_sub_epi32(v2, _v2), _cv); vs = v3;\
  v3 = _mm_sub_epi32(_mm_sub_epi32(v3, _v3), _cv); \
}

#define _MM_SCANQ_EPI16(v0, v1, v2, v3, sv, _ho_) {\
  const __m128i vh = _mm_set1_epi16(0x0f0e);\
  v0 = _ho_(v0, mm_slli_si128(v0, 2));\
  v1 = _ho_(v1, mm_slli_si128(v1, 2));\
  v2 = _ho_(v2, mm_slli_si128(v2, 2));\
  v3 = _ho_(v3, mm_slli_si128(v3, 2));\
  v0 = _ho_(v0, mm_slli_si128(v0, 4));\
  v1 = _ho_(v1, mm_slli_si128(v1, 4));\
  v2 = _ho_(v2, mm_slli_si128(v2, 4));\
  v3 = _ho_(v3, mm_slli_si128(v3, 4));\
  v0 = _ho_(v0, mm_slli_si128(v0, 8));\
  v1 = _ho_(v1, mm_slli_si128(v1, 8));\
  v2 = _ho_(v2, mm_slli_si128(v2, 8));\
  v3 = _ho_(v3, mm_slli_si128(v3, 8));\
  __m128i sv0 = _mm_shuffle_epi8(v0, vh),\
          sv1 = _mm_shuffle_epi8(v1, vh),\
          sv2 = _mm_shuffle_epi8(v2, vh),\
           a0 = _mm_shuffle_epi8(sv, vh),\
           a1 = _ho_(a0, sv0),\
           a2 = _ho_(a1, sv1),\
           a3 = _ho_(a2, sv2);\
  v0 = _ho_(v0, a0);\
  v1 = _ho_(v1, a1);\
  v2 = _ho_(v2, a2);\
  v3 = _ho_(v3, a3);\
  sv = v3;\
}

#define _MM_SCANQ_EPI32(v0, v1, v2, v3, sv, _ho_) {\
   v0 = _ho_(v0, mm_slli_si128(v0, 4));\
  v1 = _ho_(v1, mm_slli_si128(v1, 4));\
  v2 = _ho_(v2, mm_slli_si128(v2, 4));\
  v3 = _ho_(v3, mm_slli_si128(v3, 4));\
  v0 = _ho_(v0, mm_slli_si128(v0, 8));\
  v1 = _ho_(v1, mm_slli_si128(v1, 8));\
  v2 = _ho_(v2, mm_slli_si128(v2, 8));\
  v3 = _ho_(v3, mm_slli_si128(v3, 8));\
  __m128i sv0 = mm_shuffle_nnnn_epi32(v0, 3),\
          sv1 = mm_shuffle_nnnn_epi32(v1, 3),\
          sv2 = mm_shuffle_nnnn_epi32(v2, 3),\
           a0 = mm_shuffle_nnnn_epi32(sv, 3),\
           a1 = _ho_(a0, sv0),\
           a2 = _ho_(a1, sv1),\
           a3 = _ho_(a2, sv2);\
  v0 = _ho_(v0, a0);\
  v1 = _ho_(v1, a1);\
  v2 = _ho_(v2, a2);\
  v3 = _ho_(v3, a3);\
  sv = v3;\
}

#define _MM_SCANQ_EPI64(v0, v1, v2, v3, vs, _ho_) { \
  __m128i _v0 = _mm_slli_si128(v0, 8), \
          _v1 = _mm_slli_si128(v1, 8), \
          _v2 = _mm_slli_si128(v2, 8), \
          _v3 = _mm_slli_si128(v3, 8); \ 
  v0 = _ho_(v0, _v0); \
  v0 = _ho_(_mm_unpackhi_epi64(vs, vs), v0); \
  v1 = _ho_(v1, _v1); \
  v1 = _ho_(_mm_unpackhi_epi64(v0, v0), v1); \
  v2 = _ho_(v2, _v2); \
  v2 = _ho_(_mm_unpackhi_epi64(v1, v1), v2); \
  v3 = _ho_(v3, _v3); \
  vs = v3 = _ho_(_mm_unpackhi_epi64(v2, v2), v3); \
}

#define MM_SCANQ_EPI16(v0, v1, v2, v3, vs) _MM_SCANQ_EPI16(v0, v1, v2, v3, vs, _mm_add_epi16)
#define MM_SCANQ_EPI32(v0, v1, v2, v3, vs) _MM_SCANQ_EPI32(v0, v1, v2, v3, vs, _mm_add_epi32)
#define MM_SCANQ_EPI64(v0, v1, v2, v3, vs) _MM_SCANQ_EPI64(v0, v1, v2, v3, vs, _mm_add_epi64)

#define MM_XORDQ_EPI16(v0, v1, v2, v3, vs) _MM_SCANQ_EPI16(v0, v1, v2, v3, vs, _mm_xor_si128)
#define MM_XORDQ_EPI32(v0, v1, v2, v3, vs) _MM_SCANQ_EPI32(v0, v1, v2, v3, vs, _mm_xor_si128)
#define MM_XORDQ_EPI64(v0, v1, v2, v3, vs) _MM_SCANQ_EPI64(v0, v1, v2, v3, vs, _mm_xor_si128)

#define MM_DELTAZQ_EPI16(v0, v1, v2, v3, vs) { MM_DELTAQ_EPI16(v0, v1, v2, v3, vs); MM_ZZAGEQ_EPI16(v0, v1, v2, v3); }
#define MM_DELTAZQ_EPI32(v0, v1, v2, v3, vs) { MM_DELTAQ_EPI32(v0, v1, v2, v3, vs); MM_ZZAGEQ_EPI32(v0, v1, v2, v3); }

#define MM_SCANZQ_EPI16(v0, v1, v2, v3, vs) { MM_ZZAGDQ_EPI16(v0, v1, v2, v3); MM_SCANQ_EPI16(v0, v1, v2, v3, vs); }
#define MM_SCANZQ_EPI32(v0, v1, v2, v3, vs) { MM_ZZAGDQ_EPI32(v0, v1, v2, v3); MM_SCANQ_EPI32(v0, v1, v2, v3, vs); }
#define MM_SCANZQ_EPI64(v0, v1, v2, v3, vs) { MM_ZZAGDQ_EPI64(v0, v1, v2, v3); MM_SCANQ_EPI64(v0, v1, v2, v3, vs); }

#define MM_SCAN1Q_EPI16(v0, v1, v2, v3, vs) { \
  MM_SCANQ_EPI16(v0, v1, v2, v3, vs); \
  const __m128i _cv0 = _mm_set_epi16( 8, 7,  6,  5,  4,  3,  2, 1),\
                _cv1 = _mm_set_epi16(16, 15, 14, 13, 12, 11, 10, 9),\
                _cv2 = _mm_set_epi16(24, 23, 22, 21, 20, 19, 18, 17),\
                _cv3 = _mm_set_epi16(32, 31, 30, 29, 28, 27, 26, 25);\
  v0 = _mm_add_epi16(v0, _cv0);\
  v1 = _mm_add_epi16(v1, _cv1);\
  v2 = _mm_add_epi16(v2, _cv2);\
  v3 = _mm_add_epi16(v3, _cv3);\
  vs = v3;\
}

#define MM_SCAN1Q_EPI32(v0, v1, v2, v3, vs) { \
  MM_SCANQ_EPI32(v0, v1, v2, v3, vs); \
  const __m128i _cv0 = _mm_set_epi32( 4,  3,  2,  1),\
                _cv1 = _mm_set_epi32( 8,  7,  6,  5),\
                _cv2 = _mm_set_epi32(12, 11, 10,  9),\
                _cv3 = _mm_set_epi32(16, 15, 14, 13);\
  v0 = _mm_add_epi32(v0, _cv0);\
  v1 = _mm_add_epi32(v1, _cv1);\
  v2 = _mm_add_epi32(v2, _cv2);\
  v3 = _mm_add_epi32(v3, _cv3);\
  vs = v3;\
}

#define MM_SCAN1zQ_EPI16(v0, v1, v2, v3, vs) {\
  const __m128i _cv0 = _mm_set_epi16( 8,  7,  6,  5,  4,  3,  2,  1),\
                _cv1 = _mm_set_epi16(16, 15, 14, 13, 12, 11, 10,  9),\
                _cv2 = _mm_set_epi16(24, 23, 22, 21, 20, 19, 18, 17),\
                _cv3 = _mm_set_epi16(32, 31, 30, 29, 28, 27, 26, 25),\
                _csv = _mm_set1_epi16(32);\
  v0 = _mm_add_epi16(vs, _cv0);\
  v1 = _mm_add_epi16(vs, _cv1);\
  v2 = _mm_add_epi16(vs, _cv2);\
  v3 = _mm_add_epi16(vs, _cv3);\
  vs = _mm_add_epi16(vs, _csv);\
}

#define MM_SCAN1zQ_EPI32(v0, v1, v2, v3, vs) {\
  const __m128i _cv0 = _mm_set_epi32( 4,  3,  2,  1),\
                _cv1 = _mm_set_epi32( 8,  7,  6,  5),\
                _cv2 = _mm_set_epi32(12, 11, 10,  9),\
                _cv3 = _mm_set_epi32(16, 15, 14, 13),\
                _csv = _mm_set1_epi32(16);\
  v0 = _mm_add_epi32(vs, _cv0);\
  v1 = _mm_add_epi32(vs, _cv1);\
  v2 = _mm_add_epi32(vs, _cv2);\
  v3 = _mm_add_epi32(vs, _cv3);\
  vs = _mm_add_epi32(vs, _csv);\
}

#define MM_SCANIQ_EPI16(v0, v1, v2, v3, vs, vi) do {\
  const __m128i _cf = _mm_set1_epi16(0x0f0e);\
  v0 = _mm_add_epi16(v0, mm_slli_si128(v0, 2));\
  v1 = _mm_add_epi16(v1, mm_slli_si128(v1, 2));\
  v2 = _mm_add_epi16(v2, mm_slli_si128(v2, 2));\
  v3 = _mm_add_epi16(v3, mm_slli_si128(v3, 2));\
  v0 = _mm_add_epi16(v0, mm_slli_si128(v0, 4));\
  v1 = _mm_add_epi16(v1, mm_slli_si128(v1, 4));\
  v2 = _mm_add_epi16(v2, mm_slli_si128(v2, 4));\
  v3 = _mm_add_epi16(v3, mm_slli_si128(v3, 4));\
  v0 = _mm_add_epi16(_mm_add_epi16(v0, mm_slli_si128(v0, 8)), _mm_shuffle_epi8(vs, _cf));\
  v0 = _mm_add_epi16(v0, vi);\
  v1 = _mm_add_epi16(_mm_add_epi16(v1, mm_slli_si128(v1, 8)), _mm_shuffle_epi8(v0, _cf));\
  v1 = _mm_add_epi16(v1, vi);\
  v2 = _mm_add_epi16(_mm_add_epi16(v2, mm_slli_si128(v2, 8)), _mm_shuffle_epi8(v1, _cf));\
  v2 = _mm_add_epi16(v2, vi);\
  v3 = _mm_add_epi16(_mm_add_epi16(v3, mm_slli_si128(v3, 8)), _mm_shuffle_epi8(v2, _cf));\
  v3 = _mm_add_epi16(v3, vi);\
} while(0)

#define MM_SCANIQ_EPI32(v0, v1, v2, v3, _vs_, _vi_) {\
  v0 = _mm_add_epi32(v0, mm_slli_si128(v0, 4));\
  v1 = _mm_add_epi32(v1, mm_slli_si128(v1, 4));\
  v2 = _mm_add_epi32(v2, mm_slli_si128(v2, 4));\
  v3 = _mm_add_epi32(v3, mm_slli_si128(v3, 4));\
  v0 = _mm_add_epi32(v0, mm_slli_si128(v0, 8));\
  v1 = _mm_add_epi32(v1, mm_slli_si128(v1, 8));\
  v2 = _mm_add_epi32(v2, mm_slli_si128(v2, 8));\
  v3 = _mm_add_epi32(v3, mm_slli_si128(v3, 8));\
  v0 = _mm_add_epi32(v0, _mm_add_epi32(mm_shuffle_nnnn_epi32(_vs_, 3), _vi_));\
  v1 = _mm_add_epi32(v1, _vi_);\
  v2 = _mm_add_epi32(v2, _vi_);\
  v3 = _mm_add_epi32(v3, _vi_);\
  v1 = _mm_add_epi32(v1, mm_shuffle_nnnn_epi32(v0, 3));\
  v2 = _mm_add_epi32(v2, mm_shuffle_nnnn_epi32(v1, 3));\
  v3 = _mm_add_epi32(v3, mm_shuffle_nnnn_epi32(v2, 3));\
}

static ALWAYS_INLINE void mm_storeuq_si128(__m128i *op, __m128i v0, __m128i v1, __m128i v2, __m128i v3) {
  _mm_storeu_si128(op, v0); op = (char*)op+16;
  _mm_storeu_si128(op, v1); op = (char*)op+16;
  _mm_storeu_si128(op, v2); op = (char*)op+16;
  _mm_storeu_si128(op, v3); op = (char*)op+16;
}
#endif // SSE

//--------- memset -----------------------------------------
#define BITFORSET_(_out_, _n_, _start_, _mindelta_) do { unsigned _i;\
  for(_i = 0; _i != (_n_&~3); _i+=4) {\
    _out_[_i+0] = _start_+(_i  )*_mindelta_;\
    _out_[_i+1] = _start_+(_i+1)*_mindelta_;\
    _out_[_i+2] = _start_+(_i+2)*_mindelta_;\
    _out_[_i+3] = _start_+(_i+3)*_mindelta_;\
  }\
  while(_i != _n_)\
    _out_[_i] = _start_+_i*_mindelta_, ++_i;\
} while(0)

//--------- SIMD zero -----------------------------------------
  #ifdef __AVX2__
#define BITZERO32(_out_, _n_, _start_) do {\
  __m256i _vs = _mm256_set1_epi32(_start_), *_op = (__m256i *)(_out_), *out_ = (__m256i *)(_out_ + _n_);\
  do _mm256_storeu_si256(_op, _vs), _op = (char*)_op+32; while(_op < out_);\
} while(0)

#define BITFORZERO32(_out_, _n_, _start_, _mindelta_) do {\
  __m256i _vs = _mm256_set1_epi32(_start_), *_op=(__m256i *)(_out_), *out_ = (__m256i *)(_out_ + _n_), _cv = _mm256_set_epi32(7+_mindelta_,6+_mindelta_,5+_mindelta_,4+_mindelta_,3*_mindelta_,2*_mindelta_,1*_mindelta_,0);\
    _vs = _mm256_add_epi32(_vs, _cv);\
    _cv = _mm256_set1_epi32(4);\
  do { _mm256_storeu_si256(_op, _vs), _op = (char*)_op+32; _vs = _mm256_add_epi32(_vs, _cv); } while(_op < out_);\
} while(0)

#define BITDIZERO32(_out_, _n_, _start_, _mindelta_) do { __m256i _vs = _mm256_set1_epi32(_start_), _cv = _mm256_set_epi32(7+_mindelta_,6+_mindelta_,5+_mindelta_,4+_mindelta_,3+_mindelta_,2+_mindelta_,1+_mindelta_,_mindelta_), *_op=(__m256i *)(_out_), *out_ = (__m256i *)(_out_ + _n_);\
  _vs = _mm256_add_epi32(_vs, _cv); _cv = _mm256_set1_epi32(4*_mindelta_); do { _mm256_storeu_si256(_op, _vs), _op = (char*)_op+32, _vs = _mm256_add_epi32(_vs, _cv); } while(_op < out_);\
} while(0)

  #elif defined(__SSE2__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx) // -------------
// SIMD set value (memset)
#define BITZERO32(_out_, _n_, _v_) do {\
  __m128i _vs = _mm_set1_epi32(_v_), *_op = (__m128i *)(_out_), *out_ = (__m128i *)(_out_ + _n_);\
  do _mm_storeu_si128(_op, _vs), _op = (char*)_op+16; while(_op < out_);\
} while(0)

#define BITFORZERO32(_out_, _n_, _start_, _mindelta_) do {\
  __m128i _vs = _mm_set1_epi32(_start_), *_op=(__m128i *)(_out_), *out_ = (__m128i *)(_out_ + _n_), _cv = _mm_set_epi32(3*_mindelta_,2*_mindelta_,1*_mindelta_,0);\
    _vs = _mm_add_epi32(_vs, _cv);\
    _cv = _mm_set1_epi32(4);\
  do { _mm_storeu_si128(_op, _vs), _op = (char*)_op+16; _vs = _mm_add_epi32(_vs, _cv); } while(_op < out_);\
} while(0)

#define BITDIZERO32(_out_, _n_, _start_, _mindelta_) do { __m128i _vs = _mm_set1_epi32(_start_), _cv = _mm_set_epi32(3+_mindelta_,2+_mindelta_,1+_mindelta_,_mindelta_), *_op=(__m128i *)(_out_), *out_ = (__m128i *)(_out_ + _n_);\
  _vs = _mm_add_epi32(_vs, _cv); _cv = _mm_set1_epi32(4*_mindelta_); do { _mm_storeu_si128(_op, _vs), _op = (char*)_op+16, _vs = _mm_add_epi32(_vs, _cv); } while(_op < out_);\
} while(0)
  #else
#define BITFORZERO32(_out_, _n_, _start_, _mindelta_) BITFORSET_(_out_, _n_, _start_, _mindelta_)
#define BITZERO32(   _out_, _n_, _start_)             BITFORSET_(_out_, _n_, _start_, 0)
  #endif
#define BITZERO16(   _out_, _n_, _start_)             BITFORSET_(_out_, _n_, _start_, 0)

//#define DELTR( _in_, _n_, _start_, _mindelta_,      _out_) { unsigned _v; for(      _v = 0; _v < _n_; _v++) _out_[_v] = _in_[_v] - (_start_) - _v*(_mindelta_) - (_mindelta_); }
//#define DELTRB(_in_, _n_, _start_, _mindelta_, _b_, _out_) { unsigned _v; for(_b_=0,_v = 0; _v < _n_; _v++) _out_[_v] = _in_[_v] - (_start_) - _v*(_mindelta_) - (_mindelta_), _b_ |= _out_[_v]; _b_ = bsr32(_b_); }

//----------------------------------------- bitreverse scalar + SIMD -------------------------------------------
  #if __clang__ && defined __has_builtin
    #if __has_builtin(__builtin_bitreverse64)
#define BUILTIN_BITREVERSE
    #else
#define BUILTIN_BITREVERSE
    #endif
  #endif
  #ifdef BUILTIN_BITREVERSE
#define rbit8(x)  __builtin_bitreverse8( x)
#define rbit16(x) __builtin_bitreverse16(x)
#define rbit32(x) __builtin_bitreverse32(x)
#define rbit64(x) __builtin_bitreverse64(x)
  #else

    #if (__CORTEX_M >= 0x03u) || (__CORTEX_SC >= 300u)
static ALWAYS_INLINE uint32_t _rbit_(uint32_t x) { uint32_t rc; __asm volatile ("rbit %0, %1" : "=r" (rc) : "r" (x) ); }
    #endif
static ALWAYS_INLINE uint8_t rbit8(uint8_t x) {
    #if (__CORTEX_M >= 0x03u) || (__CORTEX_SC >= 300u)
  return _rbit_(x) >> 24;
    #elif 0
  x = (x & 0xaa) >> 1 | (x & 0x55) << 1;
  x = (x & 0xcc) >> 2 | (x & 0x33) << 2;
  return x << 4 | x >> 4;
    #else
  return (x * 0x0202020202ull & 0x010884422010ull) % 1023;
    #endif
}

static ALWAYS_INLINE uint16_t rbit16(uint16_t x) {
    #if (__CORTEX_M >= 0x03u) || (__CORTEX_SC >= 300u)
  return _rbit_(x) >> 16;
    #else
  x = (x & 0xaaaa) >> 1 | (x & 0x5555) << 1;
  x = (x & 0xcccc) >> 2 | (x & 0x3333) << 2;
  x = (x & 0xf0f0) >> 4 | (x & 0x0f0f) << 4;
  return x << 8 | x >> 8;
    #endif
}

static ALWAYS_INLINE uint32_t rbit32(uint32_t x) {
    #if (__CORTEX_M >= 0x03u) || (__CORTEX_SC >= 300u)
  return _rbit_(x);
    #else
  x = ((x & 0xaaaaaaaa) >> 1 | (x & 0x55555555) << 1);
  x = ((x & 0xcccccccc) >> 2 | (x & 0x33333333) << 2);
  x = ((x & 0xf0f0f0f0) >> 4 | (x & 0x0f0f0f0f) << 4);
  x = ((x & 0xff00ff00) >> 8 | (x & 0x00ff00ff) << 8);
  return x << 16 | x >> 16;
    #endif
}
static ALWAYS_INLINE uint64_t rbit64(uint64_t x) {
    #if (__CORTEX_M >= 0x03u) || (__CORTEX_SC >= 300u)
  return (uint64_t)_rbit_(x) << 32 | _rbit_(x >> 32);
    #else
  x = (x & 0xaaaaaaaaaaaaaaaa) >>  1 | (x & 0x5555555555555555) <<  1;
  x = (x & 0xcccccccccccccccc) >>  2 | (x & 0x3333333333333333) <<  2;
  x = (x & 0xf0f0f0f0f0f0f0f0) >>  4 | (x & 0x0f0f0f0f0f0f0f0f) <<  4;
  x = (x & 0xff00ff00ff00ff00) >>  8 | (x & 0x00ff00ff00ff00ff) <<  8;
  x = (x & 0xffff0000ffff0000) >> 16 | (x & 0x0000ffff0000ffff) << 16;
  return x << 32 | x >> 32;
    #endif
}
  #endif

  #if defined(__SSSE3__) || defined(__ARM_NEON) || defined(__riscv_vector) || defined(__loongarch_sx)
static ALWAYS_INLINE __m128i mm_rbit_epi16(__m128i v) { return mm_rbit_epi8(mm_rev_epi16(v)); }
static ALWAYS_INLINE __m128i mm_rbit_epi32(__m128i v) { return mm_rbit_epi8(mm_rev_epi32(v)); }
static ALWAYS_INLINE __m128i mm_rbit_epi64(__m128i v) { return mm_rbit_epi8(mm_rev_epi64(v)); }
//static ALWAYS_INLINE __m128i mm_rbit_si128(__m128i v) { return mm_rbit_epi8(mm_rev_si128(v)); }
  #endif

  #ifdef __AVX2__
static ALWAYS_INLINE __m256i mm256_rbit_epi8(__m256i v) {
  __m256i fv = _mm256_setr_epi8(0, 8, 4,12, 2,10, 6,14, 1, 9, 5,13, 3,11, 7,15, 0, 8, 4,12, 2,10, 6,14, 1, 9, 5,13, 3,11, 7,15), cv0f_8 = _mm256_set1_epi8(0xf);
  __m256i lv = _mm256_shuffle_epi8(fv,_mm256_and_si256(                  v,     cv0f_8));
  __m256i hv = _mm256_shuffle_epi8(fv,_mm256_and_si256(_mm256_srli_epi64(v, 4), cv0f_8));
  return _mm256_or_si256(_mm256_slli_epi64(lv,4), hv);
}

static ALWAYS_INLINE __m256i mm256_rev_epi16(__m256i v) { return _mm256_shuffle_epi8(v, _mm256_setr_epi8( 1, 0, 3, 2, 5, 4, 7, 6,  9, 8,11,10,13,12,15,14,  1, 0, 3, 2, 5, 4, 7, 6,  9, 8,11,10,13,12,15,14)); }
static ALWAYS_INLINE __m256i mm256_rev_epi32(__m256i v) { return _mm256_shuffle_epi8(v, _mm256_setr_epi8( 3, 2, 1, 0, 7, 6, 5, 4, 11,10, 9, 8,15,14,13,12,  3, 2, 1, 0, 7, 6, 5, 4, 11,10, 9, 8,15,14,13,12)); }
static ALWAYS_INLINE __m256i mm256_rev_epi64(__m256i v) { return _mm256_shuffle_epi8(v, _mm256_setr_epi8( 7, 6, 5, 4, 3, 2, 1, 0, 15,14,13,12,11,10, 9, 8,  7, 6, 5, 4, 3, 2, 1, 0, 15,14,13,12,11,10, 9, 8)); }
static ALWAYS_INLINE __m256i mm256_rev_si128(__m256i v) { return _mm256_shuffle_epi8(v, _mm256_setr_epi8(15,14,13,12,11,10, 9, 8,  7, 6, 5, 4, 3, 2, 1, 0, 15,14,13,12,11,10, 9, 8,  7, 6, 5, 4, 3, 2, 1, 0)); }

static ALWAYS_INLINE __m256i mm256_rbit_epi16(__m256i v) { return mm256_rbit_epi8(mm256_rev_epi16(v)); }
static ALWAYS_INLINE __m256i mm256_rbit_epi32(__m256i v) { return mm256_rbit_epi8(mm256_rev_epi32(v)); }
static ALWAYS_INLINE __m256i mm256_rbit_epi64(__m256i v) { return mm256_rbit_epi8(mm256_rev_epi64(v)); }
static ALWAYS_INLINE __m256i mm256_rbit_si128(__m256i v) { return mm256_rbit_epi8(mm256_rev_si128(v)); }
  #endif

//------------------------ negabinary ------------------------------------------------------
#if 1
#define NGB8  0x55
#define NGB16 0x5555
#define NGB32 0x55555555u
#define NGB64 0x5555555555555555ull
#else
#define NGB8  0xaa
#define NGB16 0xaaaa
#define NGB32 0xaaaaaaaau
#define NGB64 0xaaaaaaaaaaaaaaaaull
#endif
static inline uint8_t  nbenc8(  int8_t  x) { return ((uint8_t )x   + NGB8 ) ^ NGB8; }
static inline uint16_t nbenc16( int16_t x) { return ((uint16_t)x   + NGB16) ^ NGB16; }
static inline uint32_t nbenc32( int32_t x) { return ((uint32_t)x   + NGB32) ^ NGB32; }
static inline uint64_t nbenc64( int64_t x) { return ((uint64_t)x   + NGB64) ^ NGB64; }
static inline uint8_t  nbdec8(  uint8_t x) { return ( uint8_t )((x ^ NGB8 ) - NGB8 ); }
static inline uint16_t nbdec16(uint16_t x) { return ( uint16_t)((x ^ NGB16) - NGB16); }
static inline uint32_t nbdec32(uint32_t x) { return ( uint32_t)((x ^ NGB32) - NGB32); }
static inline uint64_t nbdec64(uint64_t x) { return ( uint64_t)((x ^ NGB64) - NGB64); }

#ifdef __AVX2__
static ALWAYS_INLINE __m256i mm256_nbe_epi32(__m256i v) { const __m256i vg = _mm256_set1_epi32( NGB32); return _mm256_xor_si256(_mm256_add_epi32(v,vg),vg); }
static ALWAYS_INLINE __m256i mm256_nbe_epi64(__m256i v) { const __m256i vg = _mm256_set1_epi64x(NGB64); return _mm256_xor_si256(_mm256_add_epi32(v,vg),vg); }
static ALWAYS_INLINE __m256i mm256_nbd_epi32(__m256i v) { const __m256i vg = _mm256_set1_epi32( NGB32); return _mm256_sub_epi32(_mm256_xor_si256(v,vg),vg); }
static ALWAYS_INLINE __m256i mm256_ndd_epi64(__m256i v) { const __m256i vg = _mm256_set1_epi64x(NGB64); return _mm256_sub_epi64(_mm256_xor_si256(v,vg),vg); }

#define MM256_NBEQ_EPI32(v0, v1, v2, v3) {\
  const __m256i vg = _mm256_set1_epi32(NGB32);\
  v0 = _mm256_add_epi32(v0, vg);\
  v1 = _mm256_add_epi32(v1, vg);\
  v2 = _mm256_add_epi32(v2, vg);\
  v3 = _mm256_add_epi32(v3, vg);\
  v0 = _mm256_xor_si256(v0, vg);\
  v1 = _mm256_xor_si256(v1, vg);\
  v2 = _mm256_xor_si256(v2, vg);\
  v3 = _mm256_xor_si256(v3, vg);\
}

static ALWAYS_INLINE void mm256_nbeq_epi32(__m256i *pv0, __m256i *pv1, __m256i *pv2, __m256i *pv3) {
  __m256i v0 = *pv0, v1 = *pv1, v2 = *pv2, v3 = *pv3;
  MM256_NBEQ_EPI32(v0, v1, v2, v3);
  *pv0 = v0; *pv1 = v1; *pv2 = v2; *pv3 = v3;
}

#define MM256_NBDQ_EPI32(v0, v1, v2, v3) {\
  const __m256i vg = _mm256_set1_epi32(NGB32);\
  v0 = _mm256_xor_si256(v0, vg),\
  v1 = _mm256_xor_si256(v1, vg),\
  v2 = _mm256_xor_si256(v2, vg),\
  v3 = _mm256_xor_si256(v3, vg);\
  v0 = _mm256_sub_epi32(v0, vg);\
  v1 = _mm256_sub_epi32(v1, vg);\
  v2 = _mm256_sub_epi32(v2, vg);\
  v3 = _mm256_sub_epi32(v3, vg);\
}

#define MM256_SCANBQ_EPI32(v0, v1, v2, v3, vs) { MM256_NBDQ_EPI32(v0, v1, v2, v3); MM256_SCANQ_EPI32(v0, v1, v2, v3, vs); }
#endif

#ifdef __AVX2__
static void print256x32(unsigned char *s, __m256i var) { unsigned *val = (unsigned*)&var; printf("%s%d,%d,%d,%d,%d,%d,%d,%d\t", s, val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);fflush(stdout);}
static void print128x16(unsigned char *s,__m128i var) {
    unsigned short *val = (unsigned short*) &var;
    printf("%s%x,%x,%x,%x/%x,%x,%x,%x ", s,
           val[7], val[6], val[5], val[4], val[3], val[2],
           val[1], val[0]);
}
static void print256u32(unsigned char *s, __m256i var) { unsigned *val = (unsigned*)&var; printf("%s%d,%d,%d,%d,%d,%d,%d,%d\t", s, val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);fflush(stdout);}
static void print128u16(unsigned char *s,__m128i var) {
    unsigned short *val = (unsigned short*) &var;
    printf("%s%u,%u,%u,%u/%u,%u,%u,%u ", s, val[7], val[6], val[5], val[4], val[3], val[2], val[1], val[0]);
}
#endif
#ifdef __SSE2__
static void print128u8(unsigned char *s,__m128i var) {
    uint8_t *val = (uint8_t*) &var;
    printf("%s%u,%u,%u,%u/%u,%u,%u,%u:%u,%u,%u,%u/%u,%u,%u,%u ", s, val[15], val[14], val[13], val[12], val[11], val[10], val[8], val[8],
	val[7], val[6], val[5], val[4], val[3], val[2], val[1], val[0]);
}

static void print128x32(unsigned char *s,__m128i var) {
    unsigned *val = (unsigned*) &var;
    printf("%s%x,%x,%x,%x ", s, val[3], val[2], val[1], val[0]);
}

static void print128u32(unsigned char *s,__m128i var) {
    unsigned *val = (unsigned*) &var;
    printf("%s%u,%u,%u,%u ", s, val[3], val[2], val[1], val[0]);
}
  #endif

// ------------------ bitio ---------------------------
#define bitdef(     _bw_,_br_)           uint64_t _bw_=0; unsigned _br_=0
#define bitini(     _bw_,_br_)           _bw_=_br_=0

#define bitput(     _bw_,_br_,_nb_,_x_)  (_bw_) += (uint64_t)(_x_) << (_br_), (_br_) += (_nb_)
#define bitenorm(   _bw_,_br_,_op_)      ctou64(_op_) = _bw_; _op_ += ((_br_)>>3), (_bw_) >>=((_br_)&~7), (_br_) &= 7
#define bitflush(   _bw_,_br_,_op_)      ctou64(_op_) = _bw_, _op_ += ((_br_)+7)>>3, _bw_=_br_=0

#define bitbw(      _bw_,_br_)           ((_bw_)>>(_br_))
#define bitrmv(     _bw_,_br_,_nb_)      (_br_) += _nb_

#define bitdnorm(   _bw_,_br_,_ip_)      _bw_ = ctou64((_ip_) += ((_br_)>>3)), (_br_) &= 7
#define bitalign(   _bw_,_br_,_ip_)      ((_ip_) += ((_br_)+7)>>3)

#define BITPEEK32(  _bw_,_br_,_nb_)      BZHI32(bitbw(_bw_,_br_), _nb_)
#define BITGET32(   _bw_,_br_,_nb_,_x_)  _x_ = BITPEEK32(_bw_, _br_, _nb_), bitrmv(_bw_, _br_, _nb_)
#define BITPEEK64(  _bw_,_br_,_nb_)      BZHI64(bitbw(_bw_,_br_), _nb_)
#define BITGET64(   _bw_,_br_,_nb_,_x_)  _x_ = BITPEEK64(_bw_, _br_, _nb_), bitrmv(_bw_, _br_, _nb_)

#define bitpeek57(  _bw_,_br_,_nb_)      bzhi64(bitbw(_bw_,_br_), _nb_)
#define bitget57(   _bw_,_br_,_nb_,_x_)  _x_ = bitpeek57(_bw_, _br_, _nb_), bitrmv(_bw_, _br_, _nb_)
#define bitpeek31(  _bw_,_br_,_nb_)      bzhi32(bitbw(_bw_,_br_), _nb_)
#define bitget31(   _bw_,_br_,_nb_,_x_)  _x_ = bitpeek31(_bw_, _br_, _nb_), bitrmv(_bw_, _br_, _nb_)

#define bitput8( _bw_,_br_,_b_,_x_,_op_) bitput(_bw_,_br_,_b_,_x_)
#define bitput16(_bw_,_br_,_b_,_x_,_op_) bitput(_bw_,_br_,_b_,_x_)
#define bitput32(_bw_,_br_,_b_,_x_,_op_) bitput(_bw_,_br_,_b_,_x_)
#define bitput64(_bw_,_br_,_b_,_x_,_op_) if((_b_)>45) { bitput(_bw_,_br_,(_b_)-32, (_x_)>>32); bitenorm(_bw_,_br_,_op_); bitput(_bw_,_br_,32,(unsigned)(_x_)); } else bitput(_bw_,_br_,_b_,_x_)

#define bitget8( _bw_,_br_,_b_,_x_,_ip_) bitget31(_bw_,_br_,_b_,_x_)
#define bitget16(_bw_,_br_,_b_,_x_,_ip_) bitget31(_bw_,_br_,_b_,_x_)
#define bitget32(_bw_,_br_,_b_,_x_,_ip_) bitget57(_bw_,_br_,_b_,_x_)
#define bitget64(_bw_,_br_,_b_,_x_,_ip_) if((_b_)>45) { unsigned _v; bitget57(_bw_,_br_,(_b_)-32,_x_); bitdnorm(_bw_,_br_,_ip_); BITGET64(_bw_,_br_,32,_v); _x_ = _x_<<32|_v; } else bitget57(_bw_,_br_,_b_,_x_)

//---- Floating point to Integer decomposition ---------------------------------
// seeeeeeee21098765432109876543210 (s:sign, e:exponent, 0-9:mantissa)
#define MANTF32    23
#define MANTF64    52

#define BITFENC(_u_, _sgn_, _expo_, _mant_,      _mantbits_, _one_) _sgn_ = _u_ >> (sizeof(_u_)*8-1); _expo_ = ((_u_ >> (_mantbits_)) & ( (_one_<<(sizeof(_u_)*8 - 1 - _mantbits_)) -1)); _mant_ = _u_ & ((_one_<<_mantbits_)-1);
#define BITFDEC(     _sgn_, _expo_, _mant_, _u_, _mantbits_)        _u_ = (_sgn_) << (sizeof(_u_)*8-1) | (_expo_) << _mantbits_ | (_mant_)

//#define APOSNEG

//------------- hash functions -------------------------------------------------------
#define HASH32(_x_) (((_x_) * 123456791)) //0x1af42f
#define HASH(_c_, _hbits_)   (HASH32(_c_) >> (32 - (_hbits_)))

static ALWAYS_INLINE uint32_t tmhash32(uint32_t x) { // https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key/12996028
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

//https://gist.github.com/psema4/bee2614208944f08f5c4640ff582c611
static ALWAYS_INLINE unsigned squirrel32(unsigned x) {	// get1d(), get2d() and get3d() each return an integer
  unsigned seed = 0, h = x;
  h *= 0xB5297A4D;
  h += seed;
  h ^= (h >> 8);
  h += 0x68E31DA4;
  h ^= (h << 8);
  h *= 0x1B56C4E9;
  h ^= (h >> 8);
  return h;
}

#if 1
static uint64_t ALWAYS_INLINE _hash64(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}
#else
static uint64_t ALWAYS_INLINE xorshift(const uint64_t n,int i){  return n^(n>>i); }
static uint64_t ALWAYS_INLINE _hash64(const uint64_t n){
  uint64_t p = 0x5555555555555555ull; // pattern of alternating 0 and 1
  uint64_t c = 17316035218449499591ull;// random uneven integer constant;
  return c*xorshift(p*xorshift(n,32),32);
}
#endif

#define chash32(_c_, _hbits_) bzhi32(tmhash32(_c_),_hbits_)
#define chash64(_c_, _hbits_) bzhi64(_hash64(_c_),_hbits_)
//------------- hash table : open adressing with linear probing --------------------------------------------------

#define HNEW(_usize_,_htab_, _hbits_, _hmask_, _c_, _h_)           { _h_ = T2(chash,_usize_)(_c_,_hbits_); while(_htab_[_h_]) _h_ = (_h+1)&_hmask_; }// search unused slot
																	 // add item with key c and value v to the array a with the hash slot h
#define CADD(    _a_, _htab_, _n_, _c_, _h_, _v_)            	   { _a_[_n_].c = _c_; _a_[_n_].cnt = 1; _htab_[_h_] = ++_n_; }
																	 // get index i in a for a previously added key c
#define CGET(_usize_, _a_, _htab_,_hbits_,_hmask_, _c_, _i_)       { unsigned _h = T2(chash,_usize_)(_c_,_hbits_); while(_a_[_i_ = _htab_[_h]-1].c != _c_) _h = (_h+1)&_hmask_; }
																	 // return index i of key c if found, otherwise the first available empty hash slot h
#define CFIND(_usize_, _a_, _htab_,_hbits_,_hmask_, _c_, _i_, _h_) { _h_ = T2(chash,_usize_)(_c_,_hbits_); while((_i_=_htab_[_h_]) && _a_[_i_-1].c != _c_) _h_ = (_h_+1)&(_hmask_); --_i_; }
                                                                     // rehash all items in a
#define CREHASH(_usize_, _a_, _htab_,_hbits_,_hmask_, _n_)         { unsigned _i; memset(_htab_, 0, (1<<_hbits_)*sizeof(_htab_[0])); for(_i = 0; _i < _n_; _i++) { unsigned _h; HNEW(_usize_,_htab_,_hbits_,_hmask_,_a_[_i].c, _h); _htab_[_h] = _i+1;}  }
																	 // rehash all items in with count > 0 
#define CREHASHN(_usize_,_a_, _htab_,_hbits_,_hmask_, _n_)         { unsigned _i; memset(_htab_, 0, (1<<_hbits_)*sizeof(_htab_[0])); for(_i = 0; _i < _n_; _i++) if(_a_[_i].cnt) { unsigned _h; HNEW(_usize_,_htab_,_hbits_,_hmask_,_a_[_i].c, _h); _htab_[_h] = _i+1; } }

