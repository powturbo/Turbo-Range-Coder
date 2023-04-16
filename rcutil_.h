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

  #ifdef __AVX2__  								// SIMD includes
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
#include "include_/sse_neon.h"
  #endif
#include "include_/conf.h"

  #ifdef __ARM_NEON                                                 // memory prefetch
#define PREFETCH(_ip_,_rw_)
  #else
#define PREFETCH(_ip_,_rw_) __builtin_prefetch(_ip_,_rw_)
  #endif

//-------------------------- mtf: move to front (8 bits) -----------------------------------------------------------
  #ifdef __AVX2__ // Get position of existing c
#define MEMGET8(_in_,_ip_,_cv_,_c_) do { for(;;) { unsigned m = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i*)_ip_), _cv_)); if(m) { _ip_ += ctz32(m); break; } _ip_ += 32;} while(*_ip_ != _c_) _ip_++; } while(0)
  #elif defined(__SSE__)
#define MEMGET8(_in_,_ip_,_cv_,_c_) do { for(;;) { uint16_t m =    _mm_movemask_epi8(   _mm_cmpeq_epi8(   _mm_loadu_si128((__m128i*)_ip_), _cv_)); if(m) { _ip_ += ctz16(m); break; } _ip_ += 16;} while(*_ip_ != _c_) _ip_++; } while(0)
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
//#define EMA(  _n_,_x_,_a_,_y_) (((_x_)*_a_ + ((1<<(_n_)) -_a_)*(_y_) ) >>(_n_)) // Exponential moving average EMA2=1->2 EMA4=2->4 EMA8=3->8,...
#define EMA( _n_,_x_,_a_,_y_) (((_x_)*_a_ + ((1ull<<(_n_)) -_a_)*(_y_) + (1ull<<((_n_)-2)) ) >>(_n_)) // Exponential moving average + rounding EMA2=1->2 EMA4=2->4 EMA8=3->8,...
#define RICEK(_x_)             __bsr32((_x_)+1)                                // Rice parameter

#define OVERFLOW( _in_,_inlen_,_out_, _op_, _goto_) if( _op_                >= _out_+(_inlen_*255)/256-8) { memcpy(_out_,_in_,_inlen_); _op_ = _out_+_inlen_; _goto_; }
#define OVERFLOWR(_in_,_inlen_,_out_, _op_, _goto_) if((_out_+_inlen_-_op_) >=       (_inlen_*255)/256-8) { memcpy(_out_,_in_,_inlen_); _op_ = _out_+_inlen_; _goto_; }

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

static inline uint64_t       zigzagenc64(int64_t  x)       { return x << 1 ^ x >> 63;  }
static inline  int64_t       zigzagdec64(uint64_t x)       { return x >> 1 ^ -(x & 1); }

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
#define chash(_c_, _hbits_) bzhi32(tmhash32(_c_),_hbits_)
//------------- hash table : open adressing with linear probing --------------------------------------------------

#define HNEW(_htab_, _hbits_, _hmask_, _c_, _h_)             { _h_ = chash(_c_,_hbits_); while(_htab_[_h_]) _h_ = (_h+1)&_hmask_; }// search unused slot
																	// add item with key c and value v to the array a with the hash slot h
#define CADD(    _a_, _htab_, _n_, _c_, _h_, _v_)            { _a_[_n_].c = _c_; /*_a_[_n_].v = _v_;*/ _a_[_n_++].cnt = 1; _htab_[_h_] = _n_; }
																	// get index i in a for a previously added key c
#define CGET(    _a_, _htab_,_hbits_,_hmask_, _c_, _i_)      { unsigned _h = chash(_c_,_hbits_); while(_a_[_i_ = _htab_[_h]-1].c != _c_) _h = (_h+1)&_hmask_; }
																	// return index i of key c if found, otherwise the first available empty hash slot h
#define CFIND(   _a_, _htab_,_hbits_,_hmask_, _c_, _i_, _h_) { _h_ = chash(_c_,_hbits_); while((_i_=_htab_[_h_]) && _a_[_i_-1].c != _c_) _h_ = (_h_+1)&(_hmask_); --_i_; }
                                                                    // rehash all items in a
#define CREHASH( _a_, _htab_,_hbits_,_hmask_, _n_)           { unsigned _i; memset(_htab_, 0, (1<<_hbits_)*sizeof(_htab_[0])); for(_i = 0; _i < _n_; _i++) { unsigned _h; HNEW(_htab_,_hbits_,_hmask_,_a_[_i].c, _h); _htab_[_h] = _i+1;}  }
																	// rehash all items in, except when count is zero (deleted items)
#define CREHASHN(_a_, _htab_,_hbits_,_hmask_, _n_)           { unsigned _i; memset(_htab_, 0, (1<<_hbits_)*sizeof(_htab_[0])); for(_i = 0; _i < _n_; _i++) if(_a_[_i].cnt) { unsigned _h; HNEW(_htab_,_hbits_,_hmask_,_a_[_i].c, _h); _htab_[_h] = _i+1; } }

#pragma pack(1)
typedef struct { uint32_t cnt; uint16_t c; } _PACKED sym16_t;  // c=key,  cnt=count
typedef struct { uint32_t cnt; uint32_t c; } _PACKED sym32_t;  
#pragma pack() 

#ifdef __cplusplus
extern "C" {
#endif
void *vmalloc(size_t size);
void vfree(void *address);

unsigned histcalc8( unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt);  // Histogram construction
unsigned histrcalc8(unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt);
void memrev(unsigned char a[], unsigned n);  // reverse bytes in memory buffer
size_t bitenc(unsigned char *__restrict in, size_t inlen,  unsigned char *__restrict out);
size_t bitdec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out);

#ifdef __cplusplus
}
#endif
