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
// TurboRC: Range Coder - misc. functions internal header
  #ifdef __APPLE__
#include <sys/malloc.h>
  #else
#include <malloc.h>
  #endif

  #ifdef __AVX2__  													// SIMD includes
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
//#include "sse_neon.h"
  #endif

  #ifdef __ARM_NEON                                                 // memory prefetch
#define PREFETCH(_ip_,_rw_)
  #else
#define PREFETCH(_ip_,_rw_) __builtin_prefetch(_ip_,_rw_)
  #endif

//----------- BWT -------------------
#define BWT_RDONLY  (1<<30)  // input is read only, no overwrite
#define BWT_BWT16   (1<<29) // 16 bits bwt
#define BWT_PREP8   (1<<28)   // preprocessor output 8-16 bits 
#define BWT_LZP     (1<<27)  // Force lzp
#define BWT_VERBOSE (1<<26)  // verbose

//-------------------------- mtf: move to front (8 bits) -----------------------------------------------------------
  #ifdef __AVX2__ // Get position of existing c
#define MEMGET8(_in_,_ip_,_cv_,_c_) do { for(;;) { unsigned m = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i*)_ip_), _cv_)); if(m) { _ip_ += ctz32(m); break; } _ip_ += 32;} while(*_ip_ != _c_) _ip_++; } while(0)
  #elif defined(__SSE__)
#define MEMGET8(_in_,_ip_,_cv_,_c_) do { for(;;) { uint16_t m =    _mm_movemask_epi8(   _mm_cmpeq_epi8(   _mm_loadu_si128((__m128i*)_ip_), _cv_)); if(m) { _ip_ += ctz16(m); break; } _ip_ += 16; } while(*_ip_ != _c_) _ip_++; } while(0)
  #else
#define MEMGET8(_in_,_ip_,_cv_,_c_) while(*_ip_ != _c_) _ip_++;		   
  #endif
  
  #ifdef __AVX2__ // move to front. declaration c2r (symbol c to rank): uint8_t _c2r2[256+32], *c2r = _c2r+32
#define MTF8(_c2r_, _p_, _c_) do { for(q = _p_; q > _c2r_; ) { q -= 32; _mm256_storeu_si256(q+1,_mm256_loadu_si256((__m256i*)q)); } _c2r_[0] = _c_; } while(0)
  #elif defined(__SSE__) 
#define MTF8(_c2r_, _p_, _c_) do { for(q = _p_; q > _c2r_; ) { q -= 16; _mm_storeu_si128(q+1,   _mm_loadu_si128((__m128i*)q));    } _c2r_[0] = _c_; } while(0)
  #else
#define MTF8(_c2r_, _p_, _c_) do { for(q = _p_; q > _c2r_; ) { q -= 16; ctou64(q+1+8) = ctou64(q+8); ctou64(q+1) = ctou64(q); } _c2r_[0] = _c_; } while(0)
  #endif 

//---- reverse mtf (8 bits) 
#define O 16     // declaration: uint8_t _r2c_[256+32], *r2c = _r2s+32
#define MTFD1(_r2c_,_u_) _r2c_[O] = _r2c_[O+1], _r2c_[O+1] = _u_
#if O == 16
/*#define MTFD(_r2c_,_k_,_u_) if(_k_ <= O) { uint8_t *_mtf = &_r2c_[_k_]; ctou64(_mtf+0)=ctou64(_mtf+0+1); ctou64(_mtf+8)=ctou64(_mtf+8+1); _r2c_[O] = _u_; } \
	else { ctou64(&_r2c_[O+0])=ctou64(&_r2c_[O+1]);ctou64(&_r2c_[O+8])=ctou64(&_r2c_[O+9]); uint8_t *_p; for(_p = _r2c_+O+16; _p != _r2c_+O+_k_; ++_p) _p[0] = _p[1]; *_p = _u_; /*memcpy(_r2c_+0, _r2c_+1, _k_); _r2c_[O+_k_] = _u_;*/  /*__builtin_prefetch(ip +512, 0);* /}
*/	
#define MTFD(_r2c_,_k_,_u_)\
  if(_k_ <= O) { unsigned char *_c = &_r2c_[_k_]; ctou64(&_c[ 0]) = ctou64(&_c[0+1]); ctou64(&_c[ 8]) = ctou64(&_c[8+1]); _c[O] = _u_; }\
  else { ctou64(&_r2c_[O+0]) = ctou64(&_r2c_[O+1]); ctou64(&_r2c_[O+8]) = ctou64(&_r2c_[O+9]); unsigned char *_p; for(_p = _r2c_+O+16; _p != _r2c_+O+_k_; ++_p) _p[0] = _p[1]; *_p = _u_; }
	#else
#define MTFD(_r2c_,_k_,_u_) if(_k_ <= O) { unsigned char *_c = &_r2c_[_k_]; ctou64(&_c[0])=ctou64(&_c[0+1]); _r2c_[O] = _u_; } \
	else { ctou64(&_r2c_[O+0])=ctou64(&_r2c_[O+1]); unsigned char *_p; for(_p = _r2c_+O+8; _p != _r2c_+O+_k_; ++_p) _p[0] = _p[1]; *_p = _u_; /*memcpy(_r2c_+0, _r2c_+1, _k_); _r2c_[O+_k_] = _u_;*/  /*__builtin_prefetch(ip +512, 0);*/		}
#endif	
//#define MTFD(_r2c_, r, _c_) { unsigned s = 0; do _r2c_[O+s] = _r2c_[O+s + 1]; while(++s < r); }

//------------------------- run length determination ----------------------------------------------------------
  #ifdef __AVX2__                                                   // declaration
#define MEMDEC8(_cv_, _c_) __m256i _cv_ = _mm256_set1_epi8(_c_);
  #elif defined(__SSE__)
#define MEMDEC8(_cv_, _c_) __m128i _cv_ = _mm_set1_epi8(_c_);
  #else
#define MEMDEC8(_cv_, _c_)	  
  #endif
  
  #ifdef __AVX2__ 													// runs in reverse order (R->L)
#define MEMRUNR8(_in_,ip,_cv_,_c_,_goto_) do {\
for(; ip >= _in_+sizeof(_cv_);) { unsigned msk; ip -= 32; msk = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i*)ip), _cv_));\
  if(msk != 0xffffffffu) { ip += 32 - clz32(~msk); _goto_; }\
}\
for(; ip > _in_ && ip[-1] == _c_; ip--); } while(0)
  #elif defined(__SSE__)
#define MEMRUNR8(_in_,ip,_cv_,_c_,_goto_) do {\
for(; ip >= _in_+sizeof(_cv_);) { unsigned msk; ip -= 16;  msk =   _mm_movemask_epi8(   _mm_cmpeq_epi8(   _mm_loadu_si128((__m128i*)ip), _cv_));\
  if(msk != 0xffffu    ) { ip += 32 - clz32(msk ^ 0xffff); _goto_; }\
}\
for(; ip > _in_ && ip[-1] == _c_; ip--); } while(0)
  #else
#define MEMRUNR8(_in_,ip,_cv_,_c_,_goto_) for(; ip > _in_ && ip[-1] == _c_; ip--);  
  #endif

#define BROADCAST8_64(_c_)  (~UINT64_C(0)/255 * (_c_))	  
#define SZ64 if((_z = (ctou64(_ip_) ^ _cu_))) { _ip_ += ctz64(_z)>>3; _goto_; } _ip += 8;

#define _MEMRUNR8(_ie_,_ip_,_c_,_goto_) do { uint64_t _cu = BROADCAST8_64(_c_);\
  for(; _ip_ < _ie_-8;) { uint64_t _z; SZ64; SZ64; SZ64; SZ64; }\
  while(_ip_ < _ie_ && *_ip_ == _c_) _ip_++;\
} while(0)

//-- 8/16 bits run length determination 
#define BROADCAST64(_c_, _bits_)  (~UINT64_C(0)/((1<<_bits_)-1) * (_c_))	  
#define RUN_64(_ip_,_bits_, _goto_) if((_z = (ctou64(_ip_) ^ cu64))) { _ip_ += ctz64(_z)/_bits_; _goto_; } _ip_ += 64/_bits_;

static ALWAYS_INLINE size_t memrun8(uint8_t const *in, uint8_t const *in_) { uint8_t *ip = in, c = *ip; 
  uint64_t cu64 = BROADCAST64(c, 8);
  while(ip+32 < in_) { uint64_t _z; RUN_64(ip, 8, goto a); RUN_64(ip, 8, goto a); RUN_64(ip, 8, goto a); RUN_64(ip, 8, goto a); }
  while(ip+ 8 < in_) { uint64_t _z; RUN_64(ip, 8, goto a); }
  while(ip < in_ && *ip == c) ip++;
  a: return ip - in; 
}

static ALWAYS_INLINE size_t memrun16(uint16_t const *in, uint16_t const *in_) { uint16_t *ip = in, c = *ip; 
  uint64_t cu64 = BROADCAST64(c, 16);
  while(ip+16 < in_) { uint64_t _z; RUN_64(ip, 16, goto a); RUN_64(ip, 16, goto a); RUN_64(ip, 16, goto a); RUN_64(ip, 16, goto a); }
  while(ip < in_ && *ip == c) ip++;
  a: return ip - in; 
}
	
//-------------- misc --------------------------------------------------------------------------------------------------
#define EMA1( _x_)          (_x_)  							// Exponential moving average
#define EMA2( _x_,_y_)      (((_x_) + (_x1_))>>1)
#define EMA4( _x_,_a_,_y_)  (((_x_)*_a_ + (4 -_a_)*(_y_)) >>2)
#define EMA8( _x_,_a_,_y_)  (((_x_)*_a_ + (8 -_a_)*(_y_)) >>3)
#define EMA16(_x_,_a_,_y_)  (((_x_)*_a_ + (16-_a_)*(_y_)) >>4)
#define EMA32(_x_,_a_,_y_)  (((_x_)*_a_ + (32-_a_)*(_y_)) >>5)
#define EMA64(_x_,_a_,_y_)  (((_x_)*_a_ + (64-_a_)*(_y_)) >>6)

#define RICEK(_x_) __bsr32((_x_)+1)                         // Rice parameter

#define OVERFLOW( _in_,_inlen_,_out_, _op_, _goto_) if(_op_ >= _out_+(_inlen_*255)/256-8) { memcpy(_out_,_in_,_inlen_); _op_ = _out_+_inlen_; _goto_; }
#define OVERFLOWR(_in_,_inlen_,_out_, _op_, _goto_) if((_out_+_inlen_-_op_) >= (_inlen_*255)/256-8) { memcpy(_out_,_in_,_inlen_); _op_ = _out_+_inlen_; goto e; }
// store the last bytes without encoding, when inlen is not multiple of array element size
#define INDEC  size_t inlen  = _inlen /sizeof( in[0]); { unsigned char *p_=_in+_inlen,  *_p = _in +(_inlen & ~(sizeof(in[0] )-1)); while(_p < p_) { *op++  = *_p++; } }
#define OUTDEC size_t outlen = _outlen/sizeof(out[0]); { unsigned char *p_=_out+_outlen,*_p = _out+(_outlen& ~(sizeof(out[0])-1)); while(_p < p_) *_p++  = *ip++; }

#define memset_(_op_, _c_, _n_) { unsigned _n = _n_; do *_op_++ = _c_; while(--_n_); } // n > 0
static ALWAYS_INLINE unsigned pow2next(unsigned x) { return x<2?1:(1ull << (__bsr32((x)-1)+1)); }

//------------------------ zigzag encoding -------------------------------------------------------------
static inline unsigned char  zigzagenc8( signed char    x) { return x << 1 ^   x >> 7;  }
static inline          char  zigzagdec8( unsigned char  x) { return x >> 1 ^ -(x &  1); }

static inline unsigned short zigzagenc16(short          x) { return x << 1 ^   x >> 15;  }
static inline          short zigzagdec16(unsigned short x) { return x >> 1 ^ -(x &   1); }

static inline unsigned       zigzagenc32(int      x)       { return x << 1 ^   x >> 31;  }
static inline int            zigzagdec32(unsigned x)       { return x >> 1 ^ -(x &   1); }

#define BITREV
//---- Encode - Big Endian -----------------------
#define bitput_t  					     T3(uint, __WORDSIZE, _t)
#define bitebr_t                         unsigned char
#define bitedef(  _bw_,_br_)             bitput_t _bw_; bitebr_t _br_
#define biteini(  _bw_,_br_)             _bw_ = 0, _br_ = sizeof(bitput_t)*8
#define bitrest(  _bw_, _br_) 			 (sizeof(_bw_)*8+7-_br_)
#define bitput(   _bw_, _br_, _nb_, _x_) _bw_ |= (bitput_t)(_x_) << (_br_ -= (_nb_))

// Encode renorm Right->Left 
#define biteinir(    _bw_,_br_,  _op_)   biteini(_bw_,_br_),(_op_) -= sizeof(_bw_)
#define bitenormr(   _bw_, _br_, _op_)   do { *(bitput_t *)_op_ = _bw_; unsigned _b = (sizeof(_bw_)*8  -_br_)&~7; (_op_) -= _b>>3; _bw_ <<= _b; _br_ += _b;  } while(0)
#define bitflushr(   _bw_, _br_, _op_)   do { *(bitput_t *)_op_ = _bw_; unsigned _b = (sizeof(_bw_)*8+7-_br_)&~7; (_op_) -= _b>>3; (_op_) += sizeof(_bw_); } while(0)

#define _bitenormr64(_bw_,_br_,_op_)
#define _bitenormr32(_bw_,_br_,_op_)     bitenormr( _bw_, _br_, _op_)
#define bitenormr2(  _bw_,_br_,_op_)     T2(_bitenormr,__WORDSIZE)(_bw_,_br_, _op_)

//---- Decode - Big Endian ----------------------
#define bitget_t  					     T3(uint, __WORDSIZE, _t)
#define bitdbr_t                         unsigned char
#define bitddef(  _bw_,_br_)             bitget_t _bw_; bitdbr_t _br_
#define bitdini(  _bw_,_br_)             _bw_ = _br_ = 0
#define bitbw(    _bw_,_br_)             (_bw_ << _br_) //  _bw_//
#define bitrmv(   _bw_,_br_,_nb_)        (_br_ += (_nb_)) // _bw_ <<= (_nb_), _br_ += (_nb_)//
#define bitpeek(  _bw_,_br_,_nb_)        (bitbw(_bw_,_br_)>>(sizeof(_bw_)*8-  (_nb_)))
#define bitget(   _bw_,_br_,_nb_,_x_)    _x_ = bitpeek(_bw_, _br_, _nb_), bitrmv(_bw_, _br_, _nb_)
//--- Decode Renorm Right->Left 
#define bitdinir( _bw_,_br_,_ip_)        bitdini(_bw_,_br_),_ip_ -= sizeof(_bw_)
#define bitdnormr(_bw_,_br_,_ip_)        _bw_  = *(bitget_t *)(_ip_ -= _br_>>3), _br_ &= 7 //, _bw_  <<= _br_

//-------------- Turbo VLC : Variable Length Coding for large integers with exponent + mantissa ----------------------------------------------------------
//exponent base for the bit size vlcbits: 1=63 2=123, 3=239 4=463 5=895 6=1727 7=3327
#define VLC_VB6    0  
#define VLC_VB7    4 
#define VLC_VB8   16
#define VLC_VB9   48
#define VLC_VB10 128
#define VLC_VB11 296
#define VLC_VB12 768

#define vlcbits(_vn_)               (5+_vn_)
#define vlcfirst(_vn_)              (1u<<(_vn_+1)) //1,0,4  2,4,8  3,16,16
#define vlcmbits(_expo_, _vn_)      (((_expo_) >> _vn_)-1)
#define _vlcexpo_(_x_, _vn_,_expo_) { unsigned _f = __bsr32(_x_)-_vn_+1; _expo_ = (_f<<_vn_) + bzhi32((_x_)>>(_f-1),_vn_); }
#define vlcexpo_(_x_, _vn_,_expo_)  { unsigned _x = _x_; _vlcexpo_(_x, _vn_,_expo_); }
#ifndef BITIOB_H_
#define BITIOB_H_
static inline int vlcexpo(unsigned x, unsigned vn) { unsigned expo; _vlcexpo_(x, vn, expo); return expo; }
#endif

// return exponent, mantissa + bits length for x the value 
#define vlcenc( _x_, _vn_,_expo_, _mb_, _ma_) { \
  unsigned _x = _x_;\
  _vlcexpo_(_x, _vn_, _expo_);\
  _mb_ = vlcmbits(_expo_, _vn_);\
  _ma_ = bzhi32(_x,_mb_);\
}

// build value from exponent, mantissa + length
#define vlcdec(_expo_, _mb_, _ma_, _vn_) ((((1u << _vn_) + BZHI32(_expo_,_vn_))<<(_mb_)) + (_ma_)) 

// encode the mantissa in bitio (R->L) and store the exponen in u 
#define bitvrput(_bw_,_br_,_ep_, _vn_,_vb_, _u_) do { \
  if((_u_) >= vlcfirst(_vn_)+_vb_) {\
    unsigned _expo, _mb, _ma;\
    vlcenc((_u_)-_vb_, _vn_, _expo, _mb, _ma); \
    bitput(_bw_,_br_, _mb, _ma); bitenormr(_bw_,_br_,_ep_);\
	_u_ = _expo+_vb_;\
  }\
} while(0)

// get mantissa and bitio ((R->L)) decode value from sym 
#define bitvrget( _bw_,_br_,_ip_, _vn_,_vb_,_sym_) \
  if(_sym_ >= vlcfirst(_vn_)+_vb_) { \
    _sym_ -= _vb_; \
	int _mb = vlcmbits(_sym_, _vn_), _ma; \
    bitdnormr(_bw_,_br_,_ip_);\
	bitget(_bw_,_br_, _mb,_ma);\
	_sym_ = vlcdec(_sym_, _mb, _ma, _vn_)+_vb_;\
  }

#ifdef __cplusplus
extern "C" {
#endif
void *vmalloc(size_t size);
void vfree(void *address);

void histcalc8( unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt);
void histrcalc8(unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt);

#ifdef __cplusplus
}
#endif

