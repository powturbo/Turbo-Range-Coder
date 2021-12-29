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
// bit entropy coder with bitio
#include "conf.h"
//---- BitIO encode - Little Endian ------------------------------------------------------
#define bitput_t  					     T3(uint, __WORDSIZE, _t)
#define bitebr_t                         unsigned
#define bitedec(  _bw_,_br_)             bitput_t _bw_; bitebr_t _br_
#define bitedef(  _bw_,_br_)             bitput_t _bw_=0; unsigned _br_=0
#define biteini(  _bw_,_br_)             _bw_=_br_=0
#define bitput(   _bw_,_br_,_nb_,_x_)    (_bw_) += (bitput_t)(_x_) << (_br_), (_br_) += (_nb_)

// Encode renorm Right->Left 
#define bitenorm( _bw_,_br_,_op_)        ctou64(_op_) = _bw_; _op_ += ((_br_)>>3), (_bw_) >>=((_br_)&~7), (_br_) &= 7
#define bitflush( _bw_,_br_,_op_)        ctou64(_op_) = _bw_, _op_ += ((_br_)+7)>>3, _bw_=_br_=0

//---- BitIO decode - Big Endian -------------------------------------------------------
#define bitget_t  					     T3(uint, __WORDSIZE, _t)
#define bitdbr_t                         unsigned char
#define bitddef(  _bw_,_br_)             bitget_t _bw_; bitdbr_t _br_
#define bitdini(  _bw_,_br_)             _bw_ = _br_ = 0
#define bitbw(    _bw_,_br_)             (_bw_ << _br_) // _bw_//
#define bitrmv(   _bw_,_br_,_nb_)        (_br_ += (_nb_)) // _bw_ <<= (_nb_), _br_ += (_nb_)//
#define bitpeek(  _bw_,_br_,_nb_)        (bitbw(_bw_,_br_)>>(sizeof(_bw_)*8-  (_nb_)))
#define bitget(   _bw_,_br_,_nb_,_x_)    _x_ = bitpeek(_bw_, _br_, _nb_), bitrmv(_bw_, _br_, _nb_)

// Decode renorm Right->Left 
#define bitdinir( _bw_,_br_,_ip_)        bitdini(_bw_,_br_),_ip_ -= sizeof(_bw_)
#define bitdnormr(_bw_,_br_,_ip_)        _bw_  = *(bitget_t *)(_ip_ -= _br_>>3), _br_ &= 7 //, _bw_  <<= _br_

//----------------------- Bit entropy stream i/o using bitio ------------------------------------------------- 
typedef struct { bitedec(bw,br); unsigned char *_p,*p; } stm_t; // bitio
#define STMSAVE(_s_) bitedef(bw,br); bw = (_s_)->bw; br = (_s_)->br; unsigned char *p = (_s_)->p
#define STMREST(_s_) (_s_)->bw = bw; (_s_)->br = br; (_s_)->p = p

static size_t stmetell(stm_t *s) { return s->p - s->_p; }
static size_t stmseek(stm_t *s, unsigned o, unsigned x) { s->p+=x; }

//------ Stream encode ---
#define MSBREV(_cl_,_l_) { unsigned _msb = (_l_ | (1u << _cl_)) <= _n; _l_ = _l_ << _msb | _l_ >> _cl_; _cl_ += _msb; _l_ = bzhi31(_l_,_cl_); } //#define MSBREV(_cl,_l) cl++

#define BSR(_n_)  (_n_)==1?1:__bsr32(_n_) 

#define stmput0_(bw,br,p, _l_, _n_, _lx_, _hx_) {\
  if(_n_ < ECN) { ectab_t *e = &bectab[_lx_>ECN-1?ECN-1:_lx_][_hx_>ECN-1?ECN-1:_hx_][_l_][_n_]; bitput(bw,br, e->cl,e->cw); }\
  else { unsigned _hx=_hx_, _n=_n_, _l=_l_, _x;\
    if(_n > _lx_) _x = _n-_lx_, _hx-=_x, _n-=_x;\
    if(_n > _hx ) _x = _n-_hx,  _l -=_x, _n-=_x;\
    if(_n) { unsigned _msb,_cl = BSR(_n); MSBREV(_cl,_l); bitput(bw,br, _cl,_l); }\
  } bitenorm(bw,br,p);\
}

static void stmeini(stm_t *s, unsigned char *op, size_t outsize) { s->_p = s->p = op; bitdini(s->bw, s->br); }

static ALWAYS_INLINE void stmput( stm_t *s, unsigned l, unsigned n, unsigned lx, unsigned hx) { stmput0_(s->bw,s->br,s->p, l, n, lx, hx); }

static ALWAYS_INLINE void stmputx(stm_t *s, unsigned l, unsigned x) { bitput(s->bw,s->br, l,x); bitenorm(s->bw,s->br,s->p); }

static unsigned stmflush(stm_t *s) { 
  unsigned _br = (64-s->br)&7; 
  bitflush(s->bw,s->br,s->p); 
  s->p[0] = _br; s->p++; ctou32(s->_p) = s->p - s->_p; 
  return stmetell(s); 
}

//------ Stream decode ---
#define stmget0_(bw,br,p,_n_, _lx_, _hx_, _x_) do { unsigned _n = _n_, _hx = _hx_;\
  if(_n > (_lx_)) { unsigned ifh = _n - (_lx_); _hx -= ifh; _n -= ifh; }\
  _x_ = _n > _hx?_n-_hx:0;\
  if(_n-=_x_) { bitdnormr(bw,br,p);\
	unsigned _cw, _cl = BSR(_n); _cw = bitpeek(bw,br, _cl);\
    if(likely((1u << _cl | _cw) > _n)) bitrmv(bw,br,_cl); else { bitget(bw,br, _cl+1,_cw); _cw = (_cw&1) << _cl | _cw>>1; }\
    _x_ += _cw;\
  }\
} while(0) 

static void stmdini(stm_t *s, unsigned char *in, size_t inlen) { unsigned char *ip = in+ctou32(in), sbr = *--ip; bitdinir(s->bw,s->br,ip); s->br = sbr; s->_p = in; s->p = ip; }
#define stmgetx_(_l_,_x_)                bitget(bw,br, _l_,_x_)
#define stmget_(_n_, _lx_, _hx_, _x_)    stmget0_(bw,br,p,_n_, _lx_, _hx_, _x_)
#define stmgetx(_s_, _l_, _x_)           { bitdnormr((_s_)->bw,(_s_)->br,(_s_)->p); bitget((_s_)->bw,(_s_)->br, _l_,_x_); }
#define stmget(_s_,_n_, _lx_, _hx_, _x_) stmget0_((_s_)->bw,(_s_)->br,(_s_)->p,_n_, _lx_, _hx_, _x_)
