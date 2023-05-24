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
// TurboRC: Range Coder 
#include <stdint.h>
#include "include_/conf.h"
//----------------------------- Logging/statistics macros (ex. counting number of 0/1 bits ) --------------------------
  #ifdef RC_LOG 
#define RC_BE(_x_) rc_c[_x_]++  
#define RC_BD(_x_) rc_c[_x_]++
#define RC_BG      unsigned long long rc_c[2];
#define RC_LOGINI  rc_c[0] = rc_c[1] = 0
  #else 
#define RC_BE(_x_)
#define RC_BD(_x_)
#define RC_BG
#define RC_LOGINI
  #endif

//----------------------------- Setting RC_SIZE, RC_IO, RC_BITS ------------------------------------------------------
  #ifndef RC_SIZE
#define RC_SIZE 64
  #endif  
  
  #if   RC_SIZE == 128                  // 128 bits range coder
#define rcrange_t __uint128_t
    #ifndef RC_IO                      
#define RC_IO 64
    #elif RC_IO != 8 && RC_IO != 16 && RC_IO != 32 && RC_IO != 64
#error "RC_IO must be 8,16,32 or 64"
    #endif
  #elif   RC_SIZE == 64                  // 64 bits range coder
#define rcrange_t uint64_t
    #ifndef RC_IO                      // Range coder I/O mode: 8, 16 or 32 bits  (default 32 bits for RC_SIZE=64, 8 bits for RC_SIZE=32)
#define RC_IO 32
    #elif RC_IO != 8 && RC_IO != 16 && RC_IO != 32
#error "RC_IO must be 8,16 or 32"
    #endif
  #elif RC_SIZE == 32                  // 32 bits range coder
#define rcrange_t unsigned
    #ifndef RC_IO
#define RC_IO 8
    #elif RC_IO != 8 && RC_IO != 16
#error "RC_IO must be 8 or 16"
    #endif
  #endif

  #ifndef RC_BITS                      // Optimal range coder precision 
    #if RC_IO == 32  
       #ifdef RC_MULTISYMBOL
#define RC_BITS 15
       #else
#define RC_BITS 14
       #endif  
    #elif RC_IO == 16 
#define RC_BITS 13
    #elif RC_IO == 8
#define RC_BITS 15
    #else
#error "RC_IO must be 8,16 or 32"
    #endif
  #endif

//--------------------------- Input/Output --------------------------------------------------------
  #ifdef RC_BSWAP
#define _RC_BSWAP(a) T3(bswap, _, RC_IO)(a) 
  #else
#define _RC_BSWAP(a) (a)
  #endif

#define rcout_t T3(uint,RC_IO,_t)
#define RCPUT(_x_,_op_) { T2(ctou, RC_IO)(_op_) = _RC_BSWAP(_x_); _op_ += (RC_IO/8); }
#define RCGET(_ip_) ((rcout_t)_RC_BSWAP(T2(ctou, RC_IO)(_ip_))), _ip_ += (RC_IO/8)
//#define RCGETX(_ip_) ((rcout_t)_RC_BSWAP(T2(ctou, RC_IO)(_ip_)))&(-_c), _ip_ += (RC_IO/8)&(-_c)

//--------------------------- Renormalization -----------------------------------------------------
  #if RC_IO == 8
#define _LOOP while
  #else
#define _LOOP if
  #endif

#define RC_S(_rcrange_) (sizeof(_rcrange_)*8  - RC_IO)

#define _rccarry_(_rcilow_,_rclow_, _op_)           if(unlikely((_rcilow_) > (_rclow_))) { rcout_t *_pca = (rcout_t *)_op_; while(unlikely(!++*--_pca)); }

#define _rcenorm_(_rcrange_,_rclow_,_rcilow_, _op_)\
  _LOOP(unlikely(_rcrange_ < ((rcrange_t)1<<RC_S(_rcrange_)))) { \
    _rccarry_(_rcilow_,_rclow_, _op_);\
	RCPUT((rcout_t)(_rclow_ >> RC_S(_rcrange_)),_op_); _rclow_ <<= RC_IO; _rcrange_ <<= RC_IO; _rcilow_ = _rclow_;\
  }

#define _rcdnorm_(_rcrange_, rccode,_ip_) _LOOP(_rcrange_ < ((rcrange_t)1<<RC_S(_rcrange_)))  { rccode <<= RC_IO; rccode |= RCGET(_ip_); _rcrange_ <<= RC_IO; }
/*#define _rcdnorm_(_rcrange_, rccode,_ip_) {\
  unsigned _c = _rcrange_ < ((rcrange_t)1<<RC_S(_rcrange_)); \
    rccode <<= RC_IO&(-_c); \
    rccode |= RCGETX(_ip_); \
    _rcrange_ <<= RC_IO&(-_c); \
}*/

//------------------------- Initialization encode ----------------------------------------------------
#define rcencdef(_rcrange_,_rclow_,_rcilow_) RC_BG; rcrange_t _rcrange_,_rclow_,_rcilow_
#define rceinit( _rcrange_,_rclow_,_rcilow_) { _rclow_ = _rcilow_ = 0; _rcrange_ = (rcrange_t)-1; RC_LOGINI; }
#define rcencdec(_rcrange_,_rclow_,_rcilow_) rcencdef(_rcrange_,_rclow_,_rcilow_); rceinit(_rcrange_,_rclow_,_rcilow_)

  #ifdef RC_MACROS
#define rceflush(_rcrange_,_rclow_,_rcilow_, op) {\
  _rcenorm_(_rcrange_,_rclow_,_rcilow_, op);\
  if(_rcrange_ > ((rcrange_t)1<<(sizeof(_rcrange_)*8-RC_IO+1)) ) {\
    _rccarry_(_rcilow_, _rclow_ += (rcrange_t)1 << RC_S(_rcrange_), op);\
    RCPUT( (rcout_t)(_rclow_ >> RC_S(_rcrange_)), op );\
  } else {\
    _rccarry_(_rcilow_, _rclow_ += (rcrange_t)1<<  (RC_S(_rcrange_)-RC_IO), op);\
    RCPUT((rcout_t)(_rclow_ >>  RC_S(_rcrange_)),        op);\
    RCPUT((rcout_t)(_rclow_ >> (RC_S(_rcrange_)-RC_IO)), op);\
  }\
}
  #else
    #ifndef _TURBORC_H_
#define _TURBORC_H_
static void _rceflush_(rcrange_t rcrange, rcrange_t rclow, rcrange_t rcilow, unsigned char **_op) {
  unsigned char *op = *_op;
  _rcenorm_(rcrange,rclow,rcilow, op);               
  if(rcrange > ((rcrange_t)1<<(sizeof(rcrange)*8-RC_IO+1)) ) {                               
    _rccarry_(rcilow, rclow += (rcrange_t)1 << RC_S(rcrange), op);                                                         
    RCPUT( (rcout_t)(rclow >> RC_S(rcrange)), op );                             
  } else {                                                 
    _rccarry_(rcilow, rclow += (rcrange_t)1<<  (RC_S(rcrange)-RC_IO), op);                         
    RCPUT((rcout_t)(rclow >>  RC_S(rcrange)),        op);   
    RCPUT((rcout_t)(rclow >> (RC_S(rcrange)-RC_IO)), op);  
  } 
  *_op = op;
}
#define rceflush(_rcrange_,_rclow_,_rcilow_, _op_) _rceflush_(_rcrange_,_rclow_,_rcilow_, &_op_)
    #endif
  #endif

//---------------------------------- Initialization decode --------------------------------------------
#define rcdecdef(_rcrange_, rccode) RC_BG; rcrange_t _rcrange_,rccode

#define rcdinit(_rcrange_,rccode,_ip_) { int _rci;RC_LOGINI;\
  _rcrange_ = (rcrange_t)-1;\
  rccode  = 0;\
  for(_rci = 0; _rci < (RC_SIZE/RC_IO); _rci++ ) {\
    rccode = (rccode << RC_IO) | RCGET(_ip_);\
  }\
}
#define rcdecdec(_rcrange_, rccode, _ip_) rcdecdef(_rcrange_, rccode); rcdinit(_rcrange_, rccode, _ip_)

//********************************** Multisymbol (byte-wise/nibble,...) range coder ****************************************************************
  #ifdef RC_MULTISYMBOL
    #ifdef DIV_BITS  // (= RC_SIZE-RC_BITS) -------------- division w/ reciprocal multiplication (lookup table) ---------------
      #if RC_SIZE > 32
#error "RC_SIZE must be 32 for using reciprocal multiplication"
      #endif

#pragma pack(1) 
struct _div32 { unsigned m; unsigned char s; } _PACKED; //divisor RC_BITS=15 -> (1<<(32-15))*5 = 640k lookup table
#pragma pack() 

#define powof2(n)              !((n)&((n)-1))
#define DIVS32(_d_)            (__bsr32(_d_) - powof2(_d_))
#define DIVM32(_d_,_s_)        (((1ull << (_s_ + 32)) + _d_-1) / _d_)

#define DIVDIV32(_x_, _m_,_s_) ((((_x_) * (unsigned long long)(_m_)) >> 32) >> (_s_))
#define DIVMOD32(_x_, _m_,_s_) ((_x_) -  DIVDIV32(_x_, _m_,_s_))

#define DIVTDEF32(_n_)         struct _div32 _div32lut[(1<<(_n_))+1]; // LUT
#define DIVTINI32(v, n)        { unsigned i; for(v[1].m = ~0u, v[1].s = 0,i = 2; i <= (1<<n); i++) { unsigned s = v[i].s = DIVS32(i); v[i].m = DIVM32(i,s); } }
#define DIVTDIV32(_x_, _d_)    DIVDIV32(_x_, _div32lut[_d_].m, _div32lut[_d_].s)  // Division

  #ifndef _DIVTDEF32
DIVTDEF32(DIV_BITS);
#define _DIVTDEF32
  #else
extern struct _div32 _div32lut[];
  #endif 

static int _div32ini;
static void div32init(void) { if(!_div32ini) { DIVTINI32(_div32lut, DIV_BITS); _div32ini++; } }
    #else   //------------------- hardware division ------------------------------------------------------------------------
#define div32init()
#define DIVT(__n)
#define DIVTINI(__n)
#define DIVTDIV32(_x_, _y_) ((_x_) / (_y_))
    #endif
    
                  //------------ (adaptive) multisymbol range coder ---------------------
#define _rcadprob(_rcrange_, rccode, _total_) rccode/(_rcrange_ = DIVTDIV32(_rcrange_, _total_))  

#define _rcaenc(_rcrange_,_rclow_,_rcilow_, _cum_, _cnt_, _total_,_op_) {\
  _rclow_   += (_rcrange_ = DIVTDIV32(_rcrange_, _total_)) * (_cum_);\
  _rcrange_ *= (_cnt_); \
  _rcenorm_(_rcrange_,_rclow_,_rcilow_, _op_);\
}

#define _rcadupdate(_rcrange_, rccode, _cum_, _cnt_,_ip_) {\
  rccode  -= _rcrange_*(_cum_);\
  _rcrange_ *= (_cnt_);\
  _rcdnorm_(_rcrange_,rccode,_ip_);\
}

//#################################### cdf multisymbol range coder (see usage example in turborccdf.c) ##################################
//########## Encode 
#define _rccdfenc_(_rcrange_,_rclow_, _cdf0_, _cdf1_,_op_) { _rclow_ += (_rcrange_ >>= RC_BITS)*(_cdf0_); _rcrange_ *= (_cdf1_ - _cdf0_); }
//########## Decode
#define _rccdf(_rcrange_, rccode, _cdfp_) { _rcrange_ >>= RC_BITS; _cdfp_ = DIVTDIV32(rccode, _rcrange_); }  //usage cdf decoding with division

#define _rccdfupdate_(_rcrange_, rccode, _cdf0_, _cdf1_) {\
  rcrange_t _rp  = (_cdf0_) * _rcrange_;      \
  _rcrange_      = _rcrange_ * (_cdf1_) - _rp;\
  rccode        -= _rp;                       \
}
#define _rccdfrange(_rcrange_) (_rcrange_ >>= RC_BITS) // usage in linear/binray symbol search in cdf (CDF cum must be power of 2) 
  
//## Encode + renorm
#define _rccdfenc(_rcrange_,_rclow_,_rcilow_, _cdf0_, _cdf1_,_op_) { _rccdfenc_(_rcrange_,_rclow_, _cdf0_, _cdf1_,_op_); _rcenorm_(_rcrange_,_rclow_,_rcilow_, _op_); }
//## Decode + renorm
#define _rccdfupdate(_rcrange_, rccode, _cdf0_, _cdf1_,_ip_) { _rccdfupdate_(_rcrange_, rccode, _cdf0_, _cdf1_); _rcdnorm_(_rcrange_, rccode, _ip_); }

//--------------------------- CDF encode / decode ------------------------------------------------------------------
// CDF: cdf[0]=0, cdf[i]=cdf[i-1]+cnt[i-1], cdf[cdfn+1]=total
// convert a counter predictor cnt[CDFN] to cdf :
// cdf_t cdf[CDFN+1]; cdf[0] = 0; for(int i = 0; i < CDFN; i++) cdf[i+1] = cdf[i]+cnt[i];

//---------- CDF encode ------------------
#define cdfenc(_rcrange_,_rclow_,_rcilow_, cdf, _x_, op) _rccdfenc(_rcrange_,_rclow_,_rcilow_, cdf[_x_], cdf[_x_+1], op)

//---------- CDF decode w/ linear search for small cdfs (ex. 16 symbols)
// linear symbol search
#define _cdflget(_rcrange_,rccode, _cdf_, _x_) { _x_ = 0; while(_cdf_[_x_+1]*_rcrange_ <= rccode) ++_x_; }

  #if defined(__AVX512F__) && RC_SIZE == 32
#define mm512_cmpgt_epu32_mask(_a_, _b_) _mm512_cmpgt_epi32_mask(\
  _mm512_xor_si512(_a_, _mm512_set1_epi32(0x80000000)), \
  _mm512_xor_si512(_b_, _mm512_set1_epi32(0x80000000)))

#define _cdflget16(_rcrange_,rccode, _cdf_, _x_) {\
  __m256i  _v = _mm256_loadu_si256((__m256i const *)_cdf_);\
  __m512i _v0 = _mm512_cvtepu16_epi32(_v);\
          _v0 = _mm512_mullo_epi32(_v0, _mm512_set1_epi32(_rcrange_));\
  unsigned _m =  mm512_cmpgt_epu32_mask(_v0, _mm512_set1_epi32(rccode));\
  _x_ =  ctz(_m+(1<<16)) - 1;\
}
  #elif defined(__AVX2__) && RC_SIZE == 32
// avx2 symbol search  
#define mm256_cmpgt_epu32(_a_, _b_) _mm256_cmpgt_epi32(_mm256_xor_si256(_a_, _mm256_set1_epi32(0x80000000)), _mm256_xor_si256(_b_, _mm256_set1_epi32(0x80000000)))

#define _cdflget16(_rcrange_,rccode, _cdf_, _x_) {\
  __m256i  _v = _mm256_loadu_si256((__m256i const *)_cdf_);\
  __m256i _v0 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(_v));\
          _v0 = _mm256_mullo_epi32(_v0, _mm256_set1_epi32(_rcrange_));\
  __m256i _v1 = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_v,1));\
          _v1 = _mm256_mullo_epi32(_v1, _mm256_set1_epi32(_rcrange_));\
          _v0 =  mm256_cmpgt_epu32(_v0, _mm256_set1_epi32(rccode));\
          _v1 =  mm256_cmpgt_epu32(_v1, _mm256_set1_epi32(rccode));  /*unsigned _m = _mm256_movemask_epi8(_mm256_permute4x64_epi64(_mm256_packs_epi32(_v0, _v1),0xd8));_x_ = (ctz32(_m)>>1)-1;*/\
  unsigned _m = _mm256_movemask_ps(_mm256_castsi256_ps(_v1))<<8 | _mm256_movemask_ps(_mm256_castsi256_ps(_v0)); _x_ = ctz32(_m+(1<<16))-1;\
}
  #else
// linear symbol search optimized
#define _cdflget16(_rcrange_,rccode, _cdf_, _x_) {\
  for(;;) { rcrange_t _r1,\
    _r0 = _cdf_[ 1]*_rcrange_;\
    _r1 = _cdf_[ 2]*_rcrange_; if(_r0 > rccode) { _x_ = 0; break; }\
    _r0 = _cdf_[ 3]*_rcrange_; if(_r1 > rccode) { _x_ = 1; break; }\
    _r1 = _cdf_[ 4]*_rcrange_; if(_r0 > rccode) { _x_ = 2; break; }\
    _r0 = _cdf_[ 5]*_rcrange_; if(_r1 > rccode) { _x_ = 3; break; }\
    _r1 = _cdf_[ 6]*_rcrange_; if(_r0 > rccode) { _x_ = 4; break; }\
    _r0 = _cdf_[ 7]*_rcrange_; if(_r1 > rccode) { _x_ = 5; break; }\
    _r1 = _cdf_[ 8]*_rcrange_; if(_r0 > rccode) { _x_ = 6; break; }\
    _r0 = _cdf_[ 9]*_rcrange_; if(_r1 > rccode) { _x_ = 7; break; }\
    _r1 = _cdf_[10]*_rcrange_; if(_r0 > rccode) { _x_ = 8; break; }\
    _r0 = _cdf_[11]*_rcrange_; if(_r1 > rccode) { _x_ = 9; break; }\
    _r1 = _cdf_[12]*_rcrange_; if(_r0 > rccode) { _x_ =10; break; }\
    _r0 = _cdf_[13]*_rcrange_; if(_r1 > rccode) { _x_ =11; break; }\
    _r1 = _cdf_[14]*_rcrange_; if(_r0 > rccode) { _x_ =12; break; }\
    _r0 = _cdf_[15]*_rcrange_; if(_r1 > rccode) { _x_ =13; break; }\
                             if(_r0 > rccode) { _x_ =14; break; }\
    _x_=15; break;\
  }\
}
  #endif

#define cdflget(_rcrange_,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  _rccdfrange(_rcrange_); \
  _cdflget(_rcrange_,rccode, _cdf_, _x_);\
  _rccdfupdate(_rcrange_,rccode, _cdf_[_x_], _cdf_[_x_+1],_ip_);\
}

#define cdflget16(_rcrange_,rccode, _cdf_, _x_, _ip_) {\
  _rccdfrange(_rcrange_);\
  _cdflget16(_rcrange_,rccode, _cdf_, _x_);\
  _rccdfupdate(_rcrange_,rccode, _cdf_[_x_], _cdf_[_x_+1],_ip_);\
}

//---------- CDF decode w/ binary search for large cdfs (ex. 256) ------------------------
#define _cdfbget(_rcrange_,rccode, _cdf_, _cdfnum_, _x_) {\
  _x_ = 0; unsigned _high = _cdfnum_;\
  while(_x_ + 1 < _high) {\
    unsigned _mid = (_x_ + _high) >> 1, \
               _u = _cdf_[_mid]*_rcrange_ > rccode;\
    _x_   = _u ?  _x_:_mid;\
    _high = _u ? _mid:_high;\
  }\
}

#define cdfbget(_rcrange_,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  _rccdfrange(_rcrange_);\
  _cdfbget(_rcrange_,rccode, _cdf_, _cdfn_, _x_);\
  _rccdfupdate(_rcrange_,rccode, _cdf_[_x_], _cdf_[_x_+1],_ip_);\
}

//------ Decode division or reciprocal multiplication ------------------------------------

//linear search
#define _cdfvlget(_cdf_, _cdfnum_, _cdfp, _x_) { _x_ = 0; while(_cdf_[_x_+1] <= _cdfp) ++_x_; }

#define cdfvlget(_rcrange_,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  unsigned _cdfp;\
  _rccdf(_rcrange_, rccode, _cdfp);\
  _cdfvlget(_cdf_, _cdfn_, _cdfp, _x_);\
  _rccdfupdate(_rcrange_,rccode, _cdf_[_x_], _cdf_[_x_+1], _ip_);\
} 

//Binary search
#define _cdfvbget(_cdf_, _cdfnum_, _cdfp, _x_) {\
  _x_ = 0; unsigned _high = _cdfnum_;\
  while(_x_ + 1 < _high) {\
    unsigned _mid = (_x_ + _high) >> 1, \
               _u = _cdf_[_mid] > _cdfp;\
    _x_   = _u ? _x_:_mid;\
    _high = _u ? _mid:_high;\
  }\
}

#define cdfvbget(_rcrange_,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  unsigned _cdfp;\
  _rccdf(_rcrange_, rccode, _cdfp);\
  _cdfvbget(_cdf_, _cdfn_, _cdfp, _x_);\
  _rccdfupdate(_rcrange_,rccode, _cdf_[_x_], _cdf_[_x_+1], _ip_);\
} 

// direct lookup table search for static CDFs (Not yet implemented) 

                 //------------ multisymbol direct bits -----------------
    #if RC_SIZE == 32
#define RC_LIM 15
    #else
#define RC_LIM 24
    #endif

#define rcbitsenc(_rcrange_,_rclow_, _nb_, _rcx_, _op_) { assert(_nb_ <= RC_LIM); _rcaenc(_rcrange_,_rclow_,_rcilow_, _rcx_, 1, (1<<(_nb_)), _op_); }

#define rcbitsencx(_rcrange_,_rclow_,_rcilow_, _nb_, _sym_, _op_) {\
  unsigned _rcb = _nb_, _rcx1_ = _sym_;\
  if(_rcb > RC_LIM) { _rcaenc(_rcrange_,_rclow_,_rcilow_, (_rcx1_ & ((1 << RC_LIM)-1) ), 1, (1<<RC_LIM), _op_); _rcx1_ >>= RC_LIM; _rcb -= RC_LIM; }\
  _rcaenc(_rcrange_,_rclow_,_rcilow_, _rcx1_, 1, (1<<_rcb), _op_);\
}

#define rcbitsdec(_rcrange_, rccode, _nb_,_ip_, _x_) { assert(_nb_ <= RC_LIM); _x_ = _rcadfreq(_rcrange_, rccode, (1 << _nb_) ); _rcadupdate(_rcrange_, rccode, _rcx1_, 1,_ip_); }

#define rcbitsdecx(_rcrange_, rccode, _nb_,_ip_, _x_) { unsigned _rcx1_, _rcb = _nb_;\
  if(_rcb > RC_LIM) {  _rcx1_ = _rcadfreq(_rcrange_, rccode, (1<<RC_LIM)); _rcadupdate(_rcrange_, rccode, _rcx1_, 1,_ip_); _rcb -= RC_LIM;\
              unsigned _rcx2_ = _rcadfreq(_rcrange_, rccode, (1<<_rcb));   _rcadupdate(_rcrange_, rccode, _rcx2_, 1,_ip_); _rcx1_ = (_rcx2_ << RC_LIM) | _rcx1_;\
  } else {             _rcx1_ = _rcadfreq(_rcrange_, rccode, (1<<_rcb));   _rcadupdate(_rcrange_, rccode, _rcx1_, 1,_ip_); }\
  _x_ = _rcx1_;\
}

  #else           //------------ bitwise direct bits -----------------
#define rcbitsenc(_rcrange_,_rclow_,_rcilow_, _nb_, _sym_,_op_) { unsigned _nb = _nb_;\
  do {\
    _rcenorm_(_rcrange_,_rclow_,_rcilow_, _op_);\
    _rclow_ += (_rcrange_ >>= 1) & ((rcrange_t)0 - (rcrange_t)(((_sym_) >> --_nb) & 1));\
  } while(_nb);\
}

#define rcbitsencx(_rcrange_,_rclow_,_rcilow_, _nb_, _sym_) rcbitsenc(_rcrange_,_rclow_,_rcilow_, _nb_, _sym_)

#define rcbitsdec(_rcrange_, rccode, _nb_, _x_) { \
  _x_ = 0; unsigned _nb = _nb_;\
  do { \
    _rcdnorm_(_rcrange_, rccode, _ip_);\
    rccode -= (_rcrange_ >>= 1);\
    rcrange_t rccode _code = (rcrange_t)0 - (rccode >> (RC_SIZE-1));\
    _x = (_x << 1) | (_code + 1);\
    rccode += _rcrange_ & _code;\
  } while(--_nb);\
}  

#define rcbitsdecx(_rcrange_, rccode, _nb_,_ip_, _x_) rcbitsdec(_rcrange_, rccode, _nb_,_ip_, _x_)
  #endif // MULTISYMBOL

// ******************************** bitwise range coder *************************************************************************
// _mbp_             : predicted probability  
// _mb_              : predictor 
// _mbupd0_/_mbupd1_ : bit 0/1 update functions/macros for predictor _mb_ 
// _prm_             : predictor parameters

//----- Fast flag read + check bit / update predictor (usage see: mb_vint.h) ----------------
#define if_rc0(_rcrange_, rccode, _mbp_,_ip_) unsigned _predp = _mbp_; _rcdnorm_(_rcrange_,rccode,_ip_); rcrange_t _rcx = (_rcrange_ >> RC_BITS) * (_predp); if(rccode >= _rcx)
#define if_rc1(_rcrange_, rccode, _mbp_,_ip_) unsigned _predp = _mbp_; _rcdnorm_(_rcrange_,rccode,_ip_); rcrange_t _rcx = (_rcrange_ >> RC_BITS) * (_predp); if(rccode <  _rcx)
    
#define rcupdate0(_rcrange_, rccode, _mbupd0_, _mb_,_prm0_,_prm1_) { RC_BD(0); _rcrange_ -= _rcx; rccode -= _rcx; _mbupd0_(_mb_, _predp,_prm0_,_prm1_); }
#define rcupdate1(_rcrange_, rccode, _mbupd1_, _mb_,_prm0_,_prm1_) { RC_BD(1); _rcrange_  = _rcx;                 _mbupd1_(_mb_, _predp,_prm0_,_prm1_); }

//-------------------------------- encode bit (branchless) --------------------------------------------------
#define rcbe_(_rcrange_,_rclow_, _mbp_, _op_, _bit) { \
  rcrange_t   _rcx  = (_rcrange_ >> RC_BITS) * (_mbp_), _bb = -!(_bit);   			RC_BE(_bit_); \
         _rcrange_  = _rcx + (_bb & (_rcrange_ - _rcx - _rcx)); \
         _rclow_   += _bb & _rcx;\
}

//-- encode bit + update predictor 
#define rcbe(_rcrange_,_rclow_, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_,_op_, _bit_) do { \
  rcbe_(_rcrange_,_rclow_, _mbp_, _op_, _bit_);\
  _mbupd_(_mb_,_mbp_, _prm0_,_prm1_,_bit_);\
} while(0)

//--- encode bit + update predictor + renorm 
#define rcbenc(_rcrange_,_rclow_,_rcilow_, _mbp_,_mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_) do {\
  _rcenorm_(_rcrange_,_rclow_,_rcilow_,_op_);\
  rcbe(_rcrange_,_rclow_, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_);\
} while(0)

// encode bit + update context mixing predictor (used in rccm_.c)
#define rcbme(_rcrange_,_rclow_, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_) do { \
  rcbe_(_rcrange_,_rclow_, _mbp_, _op_, _bit_);\
  _mbupd_(_mb_,_mbp_,_prm0_,_prm1_, _bit_, _mb1_,_mb2_,_sse2_);\
} while(0)
	
#define rcbmenc(_rcrange_,_rclow_,_rcilow_, _mbp_,_mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_) do {\
  _rcenorm_(_rcrange_,_rclow_,_rcilow_,_op_);\
  rcbme(_rcrange_,_rclow_, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_);\
} while(0)

//--------------------------------- decode bit (branchless) ------------------------------------------------
#define rcbd_(_rcrange_, rccode, _mbp_, _bit_) { /* https://godbolt.org/z/MorrdMfTs */\
  rcrange_t _rcx = (_rcrange_ >> RC_BITS) * (_mbp_);\
  _rcrange_ -= _rcx;\
  _bit_    = rccode < _rcx?_rcrange_=_rcx, 1:0;\
  rccode  -= (-!_bit_) & _rcx;\
}

//-- decode bit + update predictor 
#define rcbd(_rcrange_, rccode, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_) do {\
  rcrange_t _bit;\
  rcbd_(_rcrange_, rccode, _mbp_, _bit);\
  _x_ += _x_+_bit;\
  _mbupd_(_mb_,_mbp_, _prm0_,_prm1_,_bit); \
} while(0)
	
//--- decode bit + renorm
#define rcbdec( _rcrange_,rccode, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_, _ip_, _x_) do {\
  _rcdnorm_(_rcrange_,rccode,_ip_); \
  rcbd(_rcrange_,rccode,  _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_);\
} while(0)

// decode bit + update context mixing predictor (used in rccm_.c)
#define rcbmd(_rcrange_, rccode, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_, _mb1_, _mb2_, _sse2_) do {\
  rcrange_t _bit;\
  rcbd_(_rcrange_, rccode, _mbp_, _bit);\
  _x_ += _x_+_bit;\
  _mbupd_(_mb_,_mbp_, _prm0_,_prm1_,_bit, _mb1_, _mb2_, _sse2_); \
} while(0)

#define rcbmdec(_rcrange_,rccode, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_, _ip_,_x_, _mb1_,_mb2_,_sse2_) do {\
  _rcdnorm_(_rcrange_,rccode,_ip_); \
  rcbmd(_rcrange_,rccode, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_, _mb1_,_mb2_,_sse2_);\
} while(0)
