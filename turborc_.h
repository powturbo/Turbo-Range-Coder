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
#include <stdint.h>
#include <assert.h>
#include "conf.h"
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

//--------------------------- Renormalization -----------------------------------------------------
  #if RC_IO == 8
#define _LOOP while
  #else
#define _LOOP if
  #endif

#define RC_S(rcrange) (sizeof(rcrange)*8-RC_IO)
#define _rcenorm_(rcrange, rclow, _op_) _LOOP(unlikely(rcrange < ((rcrange_t)1<<RC_S(rcrange)))) { rcrange <<= RC_IO; RCPUT((rcout_t)(rclow >> RC_S(rcrange)),_op_); rclow <<= RC_IO; }
#define _rcdnorm_(rcrange, rccode,_ip_) _LOOP(rcrange          < ((rcrange_t)1<<RC_S(rcrange)))  { rcrange <<= RC_IO; rccode = (rccode << RC_IO) | RCGET(_ip_); }
#define _rccarry_(_ilow_, _rclow_, _op_) if(unlikely((_ilow_) > (_rclow_))) { rcout_t *_pca = (rcout_t *)_op_; while(unlikely(!++*--_pca)); }

//------------------------- Initialization encode ----------------------------------------------------
#define rcencdef(rcrange,rclow) RC_BG; rcrange_t rcrange,rclow;
#define rceinit(rcrange,rclow) { rclow = 0; rcrange = (rcrange_t)-1; RC_LOGINI; }
#define rcencdec(rcrange,rclow) rcencdef(rcrange,rclow);rceinit(rcrange,rclow)

  #ifdef RC_MACROS
#define rceflush(rcrange, rclow, op) {\
  _rcenorm_(rcrange, rclow, op);\
  rcrange_t ilow = rclow;\
  if(rcrange > ((rcrange_t)1<<(sizeof(rcrange)*8-RC_IO+1)) ) {\
    _rccarry_(ilow, rclow += (rcrange_t)1 << RC_S(rcrange), op);\
    RCPUT( (rcout_t)(rclow >> RC_S(rcrange)), op );\
  } else {\
    _rccarry_(ilow, rclow += (rcrange_t)1<<  (RC_S(rcrange)-RC_IO), op);\
    RCPUT((rcout_t)(rclow >>  RC_S(rcrange)),        op);\
    RCPUT((rcout_t)(rclow >> (RC_S(rcrange)-RC_IO)), op);\
  }\
}
  #else
    #ifndef _TURBORC_H_
#define _TURBORC_H_
static void _rceflush_(rcrange_t rcrange, rcrange_t rclow, unsigned char **_op) {
  unsigned char *op = *_op;
  _rcenorm_(rcrange, rclow, op);
  rcrange_t ilow = rclow;               
  if(rcrange > ((rcrange_t)1<<(sizeof(rcrange)*8-RC_IO+1)) ) {                               
    _rccarry_(ilow, rclow += (rcrange_t)1 << RC_S(rcrange), op);                                                         
    RCPUT( (rcout_t)(rclow >> RC_S(rcrange)), op );                             
  } else {                                                 
    _rccarry_(ilow, rclow += (rcrange_t)1<<  (RC_S(rcrange)-RC_IO), op);                         
    RCPUT((rcout_t)(rclow >>  RC_S(rcrange)),        op);   
    RCPUT((rcout_t)(rclow >> (RC_S(rcrange)-RC_IO)), op);  
  } 
  *_op = op;
}
#define rceflush(rcrange, rclow,_op_) _rceflush_(rcrange, rclow, &_op_)
    #endif
  #endif

//---------------------------------- Initialization decode --------------------------------------------
#define rcdecdef(rcrange, rccode) RC_BG; rcrange_t rcrange,rccode

#define rcdinit(rcrange,rccode,_ip_) { int _rci;RC_LOGINI;\
  rcrange = (rcrange_t)-1;\
  rccode  = 0;\
  for(_rci = 0; _rci < (RC_SIZE/RC_IO); _rci++ ) {\
    rccode = (rccode << RC_IO) | RCGET(_ip_);\
  }\
}
#define rcdecdec(rcrange, rccode, _ip_) rcdecdef(rcrange, rccode);rcdinit(rcrange, rccode, _ip_)

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

DIVTDEF32(DIV_BITS);
static int _div32ini;
static void div32init(void) { if(!_div32ini) { DIVTINI32(_div32lut, DIV_BITS); _div32ini++; } }
    #else   //------------------- hardware division ------------------------------------------------------------------------
#define div32init()
#define DIVT(__n)
#define DIVTINI(__n)
#define DIVTDIV32(_x_, _y_) ((_x_) / (_y_))
    #endif
    
                  //------------ (adaptive) multisymbol range coder ---------------------
#define _rcadprob(rcrange, rccode, _total_) rccode/(rcrange = DIVTDIV32(rcrange, _total_))  

#define _rcaenc(rcrange, rclow, _cum_, _cnt_, _total_,_op_) { \
  rcrange_t ilow = rclow;\
  rclow += (rcrange = DIVTDIV32(rcrange, _total_)) * (_cum_); \
  _rccarry_(ilow, rclow, _op_); \
  rcrange *= (_cnt_); \
  _rcenorm_(rcrange,rclow,_op_);\
}

#define _rcadupdate(rcrange, rccode, _cum_, _cnt_,_ip_) {\
  rccode  -= rcrange*(_cum_);\
  rcrange *= (_cnt_);\
  _rcdnorm_(rcrange,rccode,_ip_);\
}

//#################################### cdf multisymbol range coder (see usage example in turborccdf.c) ##################################
//########## Encode 
#define _rccdfenc_(rcrange,rclow, _cdf0_, _cdf1_,_op_) {\
  rcrange_t ilow = rclow;                   \
  rclow += (rcrange >>= RC_BITS)*(_cdf0_);  \
  rcrange *= (_cdf1_ - _cdf0_);             \
  _rccarry_(ilow, rclow, _op_);             \
}
//########## Decode
#define _rccdf(rcrange, rccode, _cdfp_) { rcrange >>= RC_BITS; _cdfp_ = DIVTDIV32(rccode, rcrange); }  //usage cdf decoding with division

#define _rccdfupdate_(rcrange, rccode, _cdf0_, _cdf1_) {\
  rcrange_t _rp  = (_cdf0_) * rcrange;      \
  rcrange        = rcrange * (_cdf1_) - _rp;\
  rccode        -= _rp;                     \
}
#define _rccdfrange(rcrange) (rcrange >>= RC_BITS) // usage in linear/binray symbol search in cdf (CDF cum must be power of 2) 
  
//## Encode + renorm
#define _rccdfenc(rcrange, rclow, _cdf0_, _cdf1_,_op_) { _rccdfenc_(rcrange, rclow, _cdf0_, _cdf1_,_op_); _rcenorm_(rcrange,rclow,_op_); }
//## Decode + renorm
#define _rccdfupdate(rcrange, rccode, _cdf0_, _cdf1_,_ip_) { _rccdfupdate_(rcrange, rccode, _cdf0_, _cdf1_); _rcdnorm_(rcrange, rccode, _ip_); }

//--------------------------- CDF encode / decode ------------------------------------------------------------------
// CDF: cdf[0]=0, cdf[i]=cdf[i-1]+cnt[i-1], cdf[cdfn+1]=total
// convert a counter predictor cnt[CDFN] to cdf :
// cdf_t cdf[CDFN+1]; cdf[0] = 0; for(int i = 0; i < CDFN; i++) cdf[i+1] = cdf[i]+cnt[i];

//---------- CDF encode ------------------
#define cdfenc(rcrange,rclow, cdf, _x_, op) _rccdfenc(rcrange,rclow, cdf[_x_], cdf[_x_+1], op)

//---------- CDF decode w/ linear search for small cdfs (ex. 16 symbols)
// linear symbol search
#define _cdflget(rcrange,rccode, _cdf_, _x_) { _x_ = 0; while(_cdf_[_x_+1]*rcrange <= rccode) ++_x_; }

  #if defined(__AVX2__) && RC_SIZE == 32
// avx2 symbol search  
#define mm256_cmpgt_epu32(_a_, _b_) _mm256_cmpgt_epi32(_mm256_xor_si256(_a_, _mm256_set1_epi32(0x80000000)), _mm256_xor_si256(_b_, _mm256_set1_epi32(0x80000000)))

#define _cdflget16(rcrange,rccode, _cdf_, _x_) {\
  __m256i  _v = _mm256_loadu_si256((__m256i const *)_cdf_);\
  __m256i _v0 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(_v));\
  __m256i _v1 = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_v,1));\
          _v0 = _mm256_mullo_epi32(_v0, _mm256_set1_epi32(rcrange));\
          _v1 = _mm256_mullo_epi32(_v1, _mm256_set1_epi32(rcrange));\
          _v0 =  mm256_cmpgt_epu32(_v0, _mm256_set1_epi32(rccode));\
          _v1 =  mm256_cmpgt_epu32(_v1, _mm256_set1_epi32(rccode));  /*unsigned _m = _mm256_movemask_epi8(_mm256_permute4x64_epi64(_mm256_packs_epi32(_v0, _v1),0xd8));_x_ = (ctz32(_m)>>1)-1;*/\
  unsigned _m = _mm256_movemask_ps(_mm256_castsi256_ps(_v1))<<8 | _mm256_movemask_ps(_mm256_castsi256_ps(_v0)); _x_ = ctz32(_m+(1<<16))-1;\
}
  #else
// linear symbol search optimized
#define _cdflget16(rcrange,rccode, _cdf_, _x_) {\
  for(;;) { rcrange_t _r1,\
    _r0 = _cdf_[ 1]*rcrange;\
    _r1 = _cdf_[ 2]*rcrange; if(_r0 > rccode) { _x_ = 0; break; }\
    _r0 = _cdf_[ 3]*rcrange; if(_r1 > rccode) { _x_ = 1; break; }\
    _r1 = _cdf_[ 4]*rcrange; if(_r0 > rccode) { _x_ = 2; break; }\
    _r0 = _cdf_[ 5]*rcrange; if(_r1 > rccode) { _x_ = 3; break; }\
    _r1 = _cdf_[ 6]*rcrange; if(_r0 > rccode) { _x_ = 4; break; }\
    _r0 = _cdf_[ 7]*rcrange; if(_r1 > rccode) { _x_ = 5; break; }\
    _r1 = _cdf_[ 8]*rcrange; if(_r0 > rccode) { _x_ = 6; break; }\
    _r0 = _cdf_[ 9]*rcrange; if(_r1 > rccode) { _x_ = 7; break; }\
    _r1 = _cdf_[10]*rcrange; if(_r0 > rccode) { _x_ = 8; break; }\
    _r0 = _cdf_[11]*rcrange; if(_r1 > rccode) { _x_ = 9; break; }\
    _r1 = _cdf_[12]*rcrange; if(_r0 > rccode) { _x_ =10; break; }\
    _r0 = _cdf_[13]*rcrange; if(_r1 > rccode) { _x_ =11; break; }\
    _r1 = _cdf_[14]*rcrange; if(_r0 > rccode) { _x_ =12; break; }\
    _r0 = _cdf_[15]*rcrange; if(_r1 > rccode) { _x_ =13; break; }\
                             if(_r0 > rccode) { _x_ =14; break; }\
    _x_=15; break;\
  }\
}
  #endif

#define cdflget(rcrange,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  _rccdfrange(rcrange); \
  _cdflget(rcrange,rccode, _cdf_, _x_);\
  _rccdfupdate(rcrange,rccode, _cdf_[_x_], _cdf_[_x_+1],_ip_);\
}

#define cdflget16(rcrange,rccode, _cdf_, _x_, _ip_) {\
  _rccdfrange(rcrange);\
  _cdflget16(rcrange,rccode, _cdf_, _x_);\
  _rccdfupdate(rcrange,rccode, _cdf_[_x_], _cdf_[_x_+1],_ip_);\
}

//---------- CDF decode w/ binary search for large cdfs (ex. 256) ------------------------
#define _cdfbget(rcrange,rccode, _cdf_, _cdfnum_, _x_) {\
  _x_ = 0; unsigned _high = _cdfnum_;\
  while(_x_ + 1 < _high) {\
    unsigned _mid = (_x_ + _high) >> 1, \
               _u = _cdf_[_mid]*rcrange > rccode;\
    _x_   = _u ?  _x_:_mid;\
    _high = _u ? _mid:_high;\
  }\
}

#define cdfbget(rcrange,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  _rccdfrange(rcrange);\
  _cdfbget(rcrange,rccode, _cdf_, _cdfn_, _x_);\
  _rccdfupdate(rcrange,rccode, _cdf_[_x_], _cdf_[_x_+1],_ip_);\
}

//------ Decode division or reciprocal multiplication ------------------------------------

//_inear search
#define _cdfvlget(_cdf_, _cdfnum_, _cdfp, _x_) { _x_ = 0; while(_cdf_[_x_+1] <= _cdfp) ++_x_; }

#define cdfvlget(rcrange,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  unsigned _cdfp;\
  _rccdf(rcrange, rccode, _cdfp);\
  _cdfvlget(_cdf_, _cdfn_, _cdfp, _x_);\
  _rccdfupdate(rcrange,rccode, _cdf_[_x_], _cdf_[_x_+1], _ip_);\
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

#define cdfvbget(rcrange,rccode, _cdf_, _cdfn_, _x_, _ip_) {\
  unsigned _cdfp;\
  _rccdf(rcrange, rccode, _cdfp);\
  _cdfvbget(_cdf_, _cdfn_, _cdfp, _x_);\
  _rccdfupdate(rcrange,rccode, _cdf_[_x_], _cdf_[_x_+1], _ip_);\
} 

// direct lookup table search for static CDFs (Not yet implemented) 

                 //------------ multisymbol direct bits -----------------
    #if RC_SIZE == 32
#define RC_LIM 15
    #else
#define RC_LIM 24
    #endif

#define rcbitsenc(rcrange, rclow, _nb_, _rcx_, _op_) { assert(_nb_ <= RC_LIM); _rcaenc(rcrange, rclow, _rcx_, 1, (1<<(_nb_)), _op_); }

#define rcbitsencx(rcrange, rclow, _nb_, _sym_, _op_) {\
  unsigned _rcb = _nb_, _rcx1_ = _sym_;\
  if(_rcb > RC_LIM) { _rcaenc(rcrange, rclow, (_rcx1_ & ((1 << RC_LIM)-1) ), 1, (1<<RC_LIM), _op_); _rcx1_ >>= RC_LIM; _rcb -= RC_LIM; }\
  _rcaenc(rcrange, rclow, _rcx1_, 1, (1<<_rcb), _op_);\
}

#define rcbitsdec(rcrange, rccode, _nb_,_ip_, _x_) { assert(_nb_ <= RC_LIM); _x_ = _rcadfreq(rcrange, rccode, (1 << _nb_) ); _rcadupdate(rcrange, rccode, _rcx1_, 1,_ip_); }

#define rcbitsdecx(rcrange, rccode, _nb_,_ip_, _x_) { unsigned _rcx1_, _rcb = _nb_;\
  if(_rcb > RC_LIM) {  _rcx1_ = _rcadfreq(rcrange, rccode, (1<<RC_LIM)); _rcadupdate(rcrange, rccode, _rcx1_, 1,_ip_); _rcb -= RC_LIM;\
              unsigned _rcx2_ = _rcadfreq(rcrange, rccode, (1<<_rcb));   _rcadupdate(rcrange, rccode, _rcx2_, 1,_ip_); _rcx1_ = (_rcx2_ << RC_LIM) | _rcx1_;\
  } else {             _rcx1_ = _rcadfreq(rcrange, rccode, (1<<_rcb));   _rcadupdate(rcrange, rccode, _rcx1_, 1,_ip_); }\
  _x_ = _rcx1_;\
}

  #else           //------------ bitwise direct bits -----------------
#define rcbitsenc(rcrange, rclow, _nb_, __sym,_op_) { unsigned _nb = _nb_;\
  do { \
    _rcenorm_(rcrange,rclow,_op_); \
    rcrange_t ilow = rclow; \
    rclow += (rcrange >>= 1) & ((rcrange_t)0 - (rcrange_t)(((__sym) >> --_nb) & 1));\
    _rccarry_(ilow, rclow, _op_);\
  } while(_nb);\
}

#define rcbitsencx(rcrange, rclow, _nb_, _sym_) rcbitsenc(rcrange, rclow, _nb_, _sym_)

#define rcbitsdec(rcrange, rccode, _nb_, _x_) { \
  _x_ = 0; unsigned _nb = _nb_;\
  do { \
    _rcdnorm_(rcrange, rccode, _ip_);\
    rccode -= (rcrange >>= 1);\
    rcrange_t rccode _code = (rcrange_t)0 - (rccode >> (RC_SIZE-1));\
    _x = (_x << 1) | (_code + 1);\
    rccode += rcrange & _code;\
  } while(--_nb);\
}  

#define rcbitsdecx(rcrange, rccode, _nb_,_ip_, _x_) rcbitsdec(rcrange, rccode, _nb_,_ip_, _x_)
  #endif // MULTISYMBOL

// **************************** bitwise range coder *************************************************************************
// _mbp_             : predicted probability  
// _mb_              : predictor 
// _mbupd0_/_mbupd1_ : bit 0/1 update functions for predictor _mb_ 
// _prm_             : predictor parameters

//----- Fast flag read + check bit / update predictor (usage see: mb_vint.h) ----------------
#define if_rc0(rcrange, rccode, _mbp_,_ip_) unsigned _predp = _mbp_; _rcdnorm_(rcrange,rccode,_ip_); rcrange_t _rcx = (rcrange >> RC_BITS) * (_predp); if(rccode >= _rcx)
#define if_rc1(rcrange, rccode, _mbp_,_ip_) unsigned _predp = _mbp_; _rcdnorm_(rcrange,rccode,_ip_); rcrange_t _rcx = (rcrange >> RC_BITS) * (_predp); if(rccode <  _rcx)
    
#define rcupdate0(rcrange, rccode, _mbupd0_, _mb_,_prm0_,_prm1_) { RC_BD(0); rcrange -= _rcx; rccode -= _rcx; _mbupd0_(_mb_, _predp,_prm0_,_prm1_); }
#define rcupdate1(rcrange, rccode, _mbupd1_, _mb_,_prm0_,_prm1_) { RC_BD(1); rcrange  = _rcx;                 _mbupd1_(_mb_, _predp,_prm0_,_prm1_); }

//------------ encode bit (branchless) -----
#define rcbe_(rcrange,rclow, _mbp_, _op_, _bit) {\
  rcrange_t _rcx  = (rcrange >> RC_BITS) * (_mbp_), _ilow = rclow, _b = -!_bit;  			RC_BE(_bit_); \
         rcrange  = _rcx + (_b & (rcrange - _rcx - _rcx)); \
           rclow += _b & _rcx;\
  _rccarry_(_ilow, rclow, _op_);\
}

#define rcbe(rcrange,rclow, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_,_op_, _bit_) do { \
  rcrange_t _bit = (_bit_) != 0;\
  rcbe_(rcrange,rclow, _mbp_, _op_, _bit);\
  _mbupd_(_mb_,_mbp_, _prm0_,_prm1_,_bit);\
} while(0)

#define rcbme(rcrange,rclow, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_) do {\
  rcrange_t _bit = (_bit_) !=0;\
  rcbe_(rcrange,rclow, _mbp_, _op_, _bit);\
  _mbupd_(_mb_,_mbp_,_prm0_,_prm1_, _bit, _mb1_,_mb2_,_sse2_);\
} while(0)
//--- encode bit + renorm 
#define rcbenc( rcrange,rclow, _mbp_,_mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_) do {\
  _rcenorm_(rcrange,rclow,_op_);\
  rcbe(rcrange, rclow, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_);\
} while(0)

#define rcbmenc(rcrange,rclow, _mbp_,_mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_) do {\
  _rcenorm_(rcrange,rclow,_op_);\
  rcbme(rcrange, rclow, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_);\
} while(0)

//----------- decode bit (branchless) -----
/*#define rcbd(rcrange, rccode, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_) do {\
  rcrange_t _rcx = (rcrange >> RC_BITS) * (_mbp_), _bit;\
  rcrange -= _rcx; _bit = rccode < _rcx?rcrange=_rcx, 1:0;\
  _x_ += _x_+_bit;\
  _mbupd_(_mb_,_mbp_, _prm0_,_prm1_,_bit); \
  rccode -= (-!_bit) & _rcx;\
} while(0)*/
	
#define rcbd_(rcrange, rccode, _mbp_, _bit_) {\
  rcrange_t _rcx = (rcrange >> RC_BITS) * (_mbp_);\
  rcrange -= _rcx; _bit_ = rccode < _rcx?rcrange=_rcx, 1:0;\
  rccode -= (-!_bit_) & _rcx;\
}

#define rcbd(rcrange, rccode, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_) do {\
  rcrange_t _bit;\
  rcbd_(rcrange, rccode, _mbp_, _bit);\
  _x_ += _x_+_bit;\
  _mbupd_(_mb_,_mbp_, _prm0_,_prm1_,_bit); \
} while(0)
	
#define rcbmd(rcrange, rccode, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_, _mb1_, _mb2_, _sse2_) do { /*Version with 8 parameters used in context mixing*/\
  rcrange_t _bit;\
  rcbd_(rcrange, rccode, _mbp_, _bit);\
  _x_ += _x_+_bit;\
  _mbupd_(_mb_,_mbp_, _prm0_,_prm1_,_bit, _mb1_, _mb2_, _sse2_); \
} while(0)

//--- decode bit +  renorm
#define rcbdec( rcrange,rccode, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_, _ip_, _x_) do {\
  _rcdnorm_(rcrange,rccode,_ip_); \
  rcbd(rcrange,rccode,  _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_);\
} while(0)

#define rcbmdec(rcrange,rccode, _mbp_, _mbupd_, _mb_,_prm0_,_prm1_, _ip_,_x_, _mb1_,_mb2_,_sse2_) do {\
  _rcdnorm_(rcrange,rccode,_ip_); \
  rcbmd(rcrange,rccode, _mbp_, _mbupd_,_mb_,_prm0_,_prm1_, _x_, _mb1_,_mb2_,_sse2_);\
} while(0)
