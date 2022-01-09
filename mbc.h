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
// TurboRC: Range Coder - Predictor Encode + Decode single bit + Context mixing 

// Encode with predictor
#define mbu_e(  rcrange,rclow,  _mbp_, _mb_,_prm0_,_prm1_,_op_, _bit_)                                   rcbe(  rcrange,rclow,  _mbp_, mbu_update,_mb_,_prm0_,_prm1_,_op_, _bit_)
// Encode with predictor + renorm																									  
#define mbu_enc(rcrange,rclow,  _mb_,_prm0_,_prm1_,_op_, _bit_) do { unsigned _mbp = mbu_p(_mb_,_prm0_); rcbenc(rcrange,rclow,  _mbp,  mbu_update,_mb_,_prm0_,_prm1_,_op_, _bit_); } while(0)
	
// Decode with predictor
#define mbu_d(  rcrange,rccode, _mbp_, _mb_,_prm0_,_prm1_, _x_)                                          rcbd(  rcrange,rccode, _mbp_, mbu_update, _mb_,_prm0_,_prm1_, _x_)
// Decode with predictor + renorm
#define mbu_dec(rcrange,rccode, _mb_,_prm0_,_prm1_,_ip_, _x_) do { unsigned _mbp = mbu_p(_mb_,_prm0_); rcbdec(rcrange,rccode, _mbp,  mbu_update, _mb_,_prm0_,_prm1_,_ip_, _x_); } while(0)

  #ifdef RC_MACROS
#define mbu_init0(_mb_) { mbu_init(_mb_, mbu_probinit()); }
#define mbu_init1(_mb_, _n_) { int _i; for(_i = 0; _i < _n_; _i++) mbu_init(&_mb_[_i], mbu_probinit()); } 
  #else
    #ifndef _MBC_H_
#define _MBC_H_	
// Init predictor value
static inline mbu_init0(mbu *mb) { mbu_init(mb, mbu_probinit()); } 

// Init order 0 predictor (1D array) 
static void mbu_init1(mbu *mb, unsigned n) {
  int i; 
  for(i = 0; i < n; i++)
    mbu_init(&mb[i], mbu_probinit()); 
} 
    #endif
  #endif

// Init order 1 predictor (2D array mb[a0][a1])
#define mbu_init2(_mb_, _a0_, _a1_) {                  \
  int _ix,_is,_iy;                                     \
  for(_ix = 0; _ix < (_a0_); _ix++)                    \
    for(_is = 0; _is < (_a1_); _is++)                  \
      mbu_init(&(_mb_)[_ix][_is], mbu_probinit());     \
}  

// Init order 2 predictor (3D array mb[a0][a1][a2])
#define mbu_init3(_mb_, _a0_, _a1_, _a2_) {                \
  int _iw,_ix,_is,_iy;                                     \
  for(_iw = 0; _iw < (_a0_); _iw++)                        \
    for(_ix = 0; _ix < (_a1_); _ix++)                      \
      for(_is = 0; _is < (_a2_); _is++)                    \
        mbu_init(&(_mb_)[_iw][_ix][_is], mbu_probinit());  \
}

#define MBU_DEF0(_mb_)           mbu _mb_
#define MBU_DEF1(_mb_,_b0_)      mbu _mb_[_b0_] 
#define MBU_DEF2(_mb_,_b1_,_b0_) mbu _mb_[_b1_][_b0_]

#define MBU_DEC0(_mb_)           MBU_DEF0(_mb_);           mbu_init0(_mb_)
#define MBU_DEC1(_mb_,_b0_)      MBU_DEF1(_mb_,_b0_);      mbu_init1(_mb_, _b0_)
#define MBU_DEC2(_mb_,_b1_,_b0_) MBU_DEF2(_mb_,_b1_,_b0_); mbu_init2(_mb_, _b1_, _b0_)

#define mbu_initp2(_mb_, _a0_, _a1_) {                  \
  unsigned _ix,_iy;                                     \
  for(  _ix = 0; _ix < (_a0_); _ix++)                   \
    for(_iy = 0; _iy < (_a1_); _iy++) {                 \
      mbu *_mp = &(_mb_)[(_a1_) * _ix + _iy]; mbu_init(_mp, mbu_probinit());\
	}\
}  

#define MBU_NEW2( _mb_,_b0_,_b1_) mbu *_mb_ = malloc((_b0_)*(_b1_)*sizeof(_mb_[0])); if(!_mb_) die("malloc failed\n")
#define MBU_NEWI2(_mb_,_b0_,_b1_) MBU_NEW2(_mb_,_b0_,_b1_); mbu_initp2(_mb_, _b0_, _b1_)

//------------------------------ context mixing with SSE ------------------------------------------------------
//SSE2: https://encode.su/threads/3303-Is-this-how-to-practically-implement-Arithmetic-Coding?p=63506&viewfull=1#post63506
#define mbu_updates( _mb_, _mbp_, _prm0_, _prm1_, _bit_) *(_mb_) = (_mbp_) - ((((_mbp_) - (-_bit_ & (1<<RC_BITS))) >> _prm0_) + _bit_)
#define mbu_updates0(_mb_, _mbp_, _prm0_, _prm1_) *(_mb_) = (_mbp_) - ((_mbp_) >> _prm0_)
#define mbu_updates1(_mb_, _mbp_, _prm0_, _prm1_) *(_mb_) = (_mbp_) - ( (((_mbp_) - (1<<RC_BITS)) >> _prm0_) + 1)

#define RESCALE 4

//---- o0+o1+o2 (1,2) ----------------------------------------
#define W20 1
#define W21 2
#define W22 (16-W21-W20)

#define mbum2_p(_p_, _p1_, _p2_, _sse_, _mbp_) {\
  int _p = ( W20*(_p_) + W21*(_p1_) + W22*(_p2_) ) >> RESCALE;\
  _sse_  = &_sse_[_p >> (RC_BITS-4)];\
  _mbp_  = (_p + 3*(_sse_[0] + (((_sse_[1] - _sse_[0]) * (_p & ((1<<(RC_BITS-4))-1))) >> (RC_BITS-4)))) >> (18 - RC_BITS);\
}

  #if   RC_PRDID == 1
#define PRM200  4
#define PRM201  2
#define PRM202  3
#define PRMS20  5

  #elif RC_PRDID == 2 
#define PRM200  3
#define PRM201  2
#define PRM202  2
#define PRMS20  6 

#define PRM210  4
#define PRM211  2
#define PRM212  7
#define PRMS21  6
  #elif RC_PRDID == 3 
#define PRM200  fsm
#define PRM201  fsm
#define PRM202  fsm
#define PRMS20  5
  #else
#error "predictor not supported"  
  #endif

#define mbum2_update0(_mb_,_mbp_, _prm0_, _prm1_, _mb1_,_mb2_,_sse2_) {\
  mbu_update0(  _mb_,      _mbp,      PRM200, PRM210);\
  mbu_update0(  _mb1_,     _mbp1,     PRM201, PRM211);\
  mbu_update0(  _mb2_,     _mbp2,     PRM202, PRM212);\
  mbu_updates0( _sse2_,    _sse2_[0], PRMS20, PRMS21);\
  mbu_updates0((_sse2_+1), _sse2_[1], PRMS20, PRMS21);\
}
   
#define mbum2_update1(_mb_,_mbp_, _prm0_,_prm1_, _mb1_,_mb2_,_sse2_) {\
  mbu_update1(  _mb_,      _mbp,      PRM200, PRM210);\
  mbu_update1(  _mb1_,     _mbp1,     PRM201, PRM211);\
  mbu_update1(  _mb2_,     _mbp2,     PRM202, PRM212);\
  mbu_updates1( _sse2_,    _sse2_[0], PRMS20, PRMS21);\
  mbu_updates1((_sse2_+1), _sse2_[1], PRMS20, PRMS21);\
}

#define mbum2_update(_mb_,_mbp_, _prm0_,_prm1_, _bit_, _mb1_,_mb2_,_sse2_) {\
  mbu_update(  _mb_,      _mbp,      PRM200, PRM210, _bit_);\
  mbu_update(  _mb1_,     _mbp1,     PRM201, PRM211, _bit_);\
  mbu_update(  _mb2_,     _mbp2,     PRM202, PRM212, _bit_);\
  mbu_updates( _sse2_,    _sse2_[0], PRMS20, PRMS21, _bit_);\
  mbu_updates((_sse2_+1), _sse2_[1], PRMS20, PRMS21, _bit_);\
}

//----------------------  o0+o1 (2,4,6) -------------------------------------------------
#define WB20 7
#define WB21 7
#define WB22 (16-WB21-WB20)

#define mbur_p(_p_, _p1_, _p2_, _sse_, _mbp_) {\
  int _p          = ( WB20*(_p_) + WB21*(_p1_) + WB22*(_p2_) ) >> RESCALE; \
  _sse_           = &_sse_[_p >> (RC_BITS-4)];\
  const int x1    = _sse_[0];\
  const int _ssep = x1 + (((_sse_[1] - x1) * (_p & ((1<<(RC_BITS-4))-1))) >> (RC_BITS-4));\
  _mbp_ = (_p + 3*_ssep) >> (18 - RC_BITS);\
}

#define W0 1
#define W1 (16-W0)

#define mbum_p(_p_, _p1_, _p2_, _sse_, _mbp_) {\
  int _p          = ( W0*(_p_) + W1*(_p1_) /*+ W2*(_p2_)*/ ) >> RESCALE; \
  _sse_           = &_sse_[_p >> (RC_BITS-4)];\
  const int x1    = _sse_[0];\
  const int _ssep = x1 + (((_sse_[1] - x1) * (_p & ((1<<(RC_BITS-4))-1))) >> (RC_BITS-4));\
  _mbp_ = (_p + 3*_ssep) >> (18 - RC_BITS);\
}

  #if   RC_PRDID == 1 //RC_PRD_S
#define PRM100  2
#define PRM101  4
#define PRMS10  6
  #elif RC_PRDID == 2 //RC_PRD_SS
#define PRM100  3
#define PRM101  4
#define PRMS10  7

#define PRM110  3
#define PRM111  4
#define PRMS11  7
  #elif RC_PRDID == 3 //RC_PRD_SF
#define PRM100  fsm
#define PRM101  fsm
#define PRMS10  5
  #else
#error "predictor not supported"  
  #endif

#define mbum_update0(_mb_,_mbp_, _prm0_,_prm1_, _mb1_,_mb2_,_sse2_) {\
  mbu_update0(  _mb_,      _mbp,      PRM100,  PRM110);\
  mbu_update0(  _mb1_,     _mbp1,     PRM101,  PRM111);\
  mbu_updates0( _sse2_,    _sse2_[0], PRMS10,  PRMS11);\
  mbu_updates0((_sse2_+1), _sse2_[1], PRMS10,  PRMS11);\
}
   
#define mbum_update1(_mb_,_mbp_, _prm0_,_prm1_, _mb1_,_mb2_,_sse2_) {\
  mbu_update1(  _mb_,      _mbp,      PRM100,  PRM110);\
  mbu_update1(  _mb1_,     _mbp1,     PRM101,  PRM111);\
  mbu_updates1( _sse2_,    _sse2_[0], PRMS10,  PRMS11);\
  mbu_updates1((_sse2_+1), _sse2_[1], PRMS10,  PRMS11);\
}

#define mbum_update(_mb_,_mbp_, _prm0_,_prm1_, _bit_, _mb1_,_mb2_,_sse2_) {\
  mbu_update(  _mb_,      _mbp,      PRM100,  PRM110, _bit_);\
  mbu_update(  _mb1_,     _mbp1,     PRM101,  PRM111, _bit_);\
  mbu_updates( _sse2_,    _sse2_[0], PRMS10,  PRMS11, _bit_);\
  mbu_updates((_sse2_+1), _sse2_[1], PRMS10,  PRMS11, _bit_);\
}

#define mbum_enc(rcrange,rclow,  _mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_) do {\
  unsigned _mbxp,\
           _mbp  = mbu_p(_mb_,_prm0_),\
		   _mbp1  = mbu_p(_mb1_,_prm0_);\
  mbum_p(_mbp,_mbp1,0,_sse2_, _mbxp);\
  rcbmenc(rcrange,rclow,  _mbxp, mbum_update,_mb_,_prm0_,_prm1_, _op_,_bit_, _mb1_,_mb2_, _sse2_);\
} while(0)

#define mbum_dec(rcrange,rccode, _mb_,_prm0_,_prm1_,_ip_, _x_, _mb1_, _mb2_, _sse2_) do {\
  unsigned _mbxp,\
           _mbp = mbu_p(_mb_,_prm0_),\
		   _mbp1 = mbu_p(_mb1_,_prm0_);\
  mbum_p(_mbp,_mbp1,0,_sse2_, _mbxp);\
  rcbmdec(rcrange,rccode, _mbxp, mbum_update,_mb_,_prm0_,_prm1_, _ip_,  _x_, _mb1_,_mb2_,_sse2_);\
} while(0)
	
#define mbum2_enc(rcrange,rclow, _mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_) do {\
  unsigned _mbxp,\
           _mbp  = mbu_p(_mb_,_prm0_),\
		   _mbp1 = mbu_p(_mb1_,_prm0_),\
		   _mbp2 = mbu_p(_mb2_,_prm0_);\
  mbum2_p(_mbp,_mbp1,_mbp2,_sse2_, _mbxp);\
  rcbmenc(rcrange,rclow,  _mbxp,  mbum2_update,_mb_,_prm0_,_prm1_, _op_,_bit_, _mb1_,_mb2_, _sse2_);\
} while(0)

#define mbum2_dec(rcrange,rccode, _mb_,_prm0_,_prm1_,_ip_, _x_, _mb1_, _mb2_, _sse2_) do {\
  unsigned _mbxp, _mbp = mbu_p(_mb_,_prm0_),\
           _mbp1 = mbu_p(_mb1_,_prm0_),\
		   _mbp2 = mbu_p(_mb2_,_prm0_);\
  mbum2_p(_mbp,_mbp1,_mbp2,_sse2_, _mbxp);\
  rcbmdec(rcrange,rccode, _mbxp,  mbum2_update,_mb_,_prm0_,_prm1_, _ip_,  _x_, _mb1_,_mb2_,_sse2_);\
} while(0)

//---------------- o0+o1 run aware for sekewed data for ex. BWT-data --------------------------------------------------------------
  #if   RC_PRDID == 1 //RC_PRD_S
#define PRR100  2
#define PRR101  4
#define PRRS10  6

  #elif RC_PRDID == 2 //RC_PRD_SS  2,4,5 + 2, 5, 5
#define PRR100  1
#define PRR101  3
#define PRRS10  6

#define PRR110  3
#define PRR111  7
#define PRRS11  6
  #elif RC_PRDID == 3 //RC_PRD_SF
#define PRR100  fsm
#define PRR101  fsm
#define PRRS10  5
  #else
#error "predictor not supported"  
  #endif

#define mbur_update0(_mb_,_mbp_, _prm0_,_prm1_, _mb1_,_mb2_,_sse2_) {\
  mbu_update0(  _mb_,      _mbp,      PRR100,  PRR110);\
  mbu_update0(  _mb1_,     _mbp1,     PRR101,  PRR111);\
  mbu_updates0( _sse2_,    _sse2_[0], PRRS10,  PRRS11);\
  mbu_updates0((_sse2_+1), _sse2_[1], PRRS10,  PRRS11);\
}
   
#define mbur_update1(_mb_,_mbp_, _prm0_,_prm1_, _mb1_,_mb2_,_sse2_) {\
  mbu_update1(  _mb_,      _mbp,      PRR100,  PRR110);\
  mbu_update1(  _mb1_,     _mbp1,     PRR101,  PRR111);\
  mbu_updates1( _sse2_,    _sse2_[0], PRRS10,  PRRS11);\
  mbu_updates1((_sse2_+1), _sse2_[1], PRRS10,  PRRS11);\
}

#define mbur_update(_mb_,_mbp_, _prm0_,_prm1_, _bit_, _mb1_,_mb2_,_sse2_) {\
  mbu_update(  _mb_,      _mbp,      PRR100,  PRR110, _bit_);\
  mbu_update(  _mb1_,     _mbp1,     PRR101,  PRR111, _bit_);\
  mbu_updates( _sse2_,    _sse2_[0], PRRS10,  PRRS11, _bit_);\
  mbu_updates((_sse2_+1), _sse2_[1], PRRS10,  PRRS11, _bit_);\
}

#define mbur_enc(rcrange,rclow,  _mb_,_prm0_,_prm1_,_op_, _bit_, _mb1_, _mb2_, _sse2_) do {\
  unsigned _mbxp,\
           _mbp  = mbu_p(_mb_,_prm0_),\
		   _mbp1 = mbu_p(_mb1_,_prm0_),\
		   _mbp2 = mbu_p(_mb2_,_prm0_);\
  mbur_p(_mbp,_mbp1,_mbp2,_sse2_, _mbxp);\
  rcbmenc(rcrange,rclow,  _mbxp,  mbur_update,_mb_,_prm0_,_prm1_, _op_,_bit_, _mb1_,_mb2_, _sse2_);\
} while(0)

#define mbur_dec(rcrange,rccode, _mb_,_prm0_,_prm1_,_ip_, _x_, _mb1_, _mb2_, _sse2_) do {\
  unsigned _mbxp,\
           _mbp  = mbu_p(_mb_,_prm0_),\
           _mbp1 = mbu_p(_mb1_,_prm0_),\
           _mbp2 = mbu_p(_mb2_,_prm0_);\
  mbur_p(_mbp,_mbp1,_mbp2,_sse2_, _mbxp);\
  rcbmdec(rcrange,rccode, _mbxp,  mbur_update,_mb_,_prm0_,_prm1_, _ip_,  _x_, _mb1_,_mb2_,_sse2_);\
} while(0)

#ifdef __cplusplus
extern "C" {
#endif
void ssebinit(unsigned short sse2[1<<9][17]);
void sseinit( unsigned short sse[1<<8][17]);
#ifdef __cplusplus
}
#endif
