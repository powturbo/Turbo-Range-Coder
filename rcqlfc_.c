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
// Turbo Range Coder bwt: templates include
#include "include/turborc.h"
#include "include_/rcutil.h"

#include "rcutil_.h"
#include "mb_vint.h"  
//-------------------------- Post bwt stage: QLFC entropy coding ------------------------------------------------------------------
  #ifndef RCPRM
#define RCPRM
#define RCPRMC
  #endif

#if   RC_PRDID == 1        // optimal predictor parameters 
#define RCPRM0K 5
#define RCPRM1K 5
#elif RC_PRDID == 2
#define RCPRM0K 4 // 4,8
#define RCPRM1K 8
#define RCPRM0R 5 //5,8
#define RCPRM1R 8
#else
#define RCPRM0K RCPRM0
#define RCPRM1K RCPRM1
#define RCPRM0R RCPRM0
#define RCPRM1R RCPRM1
#endif

#define PREDEMAK(_avg_,_x_) EMA(3, _avg_,  5,  _x_)                                                     // 3 bits
#define PREDEMAR(_avg_,_x_) EMA(5, _avg_, 23, (_x_)>31?31:(_x_))                                        // 5 bits 
#define CXK                 unsigned cxk = RICEK(K[u]>31?31:K[u]) << 8 | u   	                        // cxk: 3+8bits
#define CXR                 unsigned cxr = RICEK(R[u])            << 8 | u, ku = RICEK(K[u]>14?14:K[u]) // cxr: 3+8 = 11, ku:2bits
enum { KU0=11, KU=11, KB=11,
       RU0=13, RU=12, RB= 8 };                                                                         
  #ifndef NCOMP																	
size_t T3(rcqlfc,RC_PRD,enc)(uint8_t *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t _r2cr[(1<<8)+32], *r2cr = &_r2cr[32], *ip = in, *in_,*op = out, *_rk = vmalloc((inlen+1)*sizeof(in[0])), *rk; if(!_rk) die("malloc failed. size=%u\n", inlen);   
  MBG_DEC( mbg0a,         mbgua,        mbgba,        33, 33);       			//initial mtf r2c 
  MBG_DEC2(mbg0c, 1<<KU0, mbguc, 1<<KU, mbgbc, 1<<KB, 33, 33);  				//rank
  MBG_DEC2(mbg0r, 1<<RU0, mbgur, 1<<RU, mbgbr, 1<<RB, 33, 33);  				//run length
  rcencdec(rcrange, rclow); 							                     	//range coder
  
  uint8_t   K[1<<8], R[1<<8] = {1}; 
  unsigned cx;
  rk = rcqlfc(in, inlen, _rk, r2cr);                                                       
  for(cx = 0; cx < (1<<8); cx++) { mbgenc(rcrange,rclow, &mbg0a, mbgua, mbgba, RCPRM0,RCPRM1,op, r2cr[cx]); K[cx] = r2cr[cx]; }
  for(ip = in, in_ = in+inlen; ip < in_;) {
    unsigned k = *--rk,r;                   
	uint8_t  u = *ip++, *p = ip; while(ip < in_ && *ip == u) ip++; r = ip - p;  // run length encoding  //uint8_t  u = *ip; r = memrun8(ip, in_); ip += r--; 
	CXK; mbgxenc(rcrange,rclow, &mbg0c[       cxk], mbguc[           cxk], mbgbc[cxk], RCPRM0K,RCPRM1K,op, k); K[u] = PREDEMAK(K[u],k);
	CXR; mbgxenc(rcrange,rclow, &mbg0r[ku<<11|cxr], mbgur[(ku>0)<<11|cxr], mbgbr[u  ], RCPRM0R,RCPRM1R,op, r); R[u] = PREDEMAR(R[u],r);		
																				OVERFLOW(in,inlen, out, op, goto e);
  }
  rceflush(rcrange,rclow, op);													OVERFLOW(in,inlen, out,op,;);
  e:vfree(_rk); 																																				    
  return op - out;
}
  #endif 

  #ifndef NDECOMP
size_t T3(rcqlfc,RC_PRD,dec)(uint8_t *in, size_t outlen, uint8_t *out RCPRM) {
  uint8_t r2c[257+O], *op, *ip = in, *p, K[256], R[256] = {1};;
  unsigned i;
  MBG_DEC( mbg0a,         mbgua,        mbgba,        33, 33);        
  MBG_DEC2(mbg0c, 1<<KU0, mbguc, 1<<KU, mbgbc, 1<<KB, 33, 33);
  MBG_DEC2(mbg0r, 1<<RU0, mbgur, 1<<RU, mbgbr, 1<<RB, 33, 33); 
  rcencdef(rcrange,rccode); rcdinit(rcrange, rccode, ip);         
  
  for(i = 0; i < (1<<8); i++) { unsigned x; mbgdec(rcrange,rccode, &mbg0a, mbgua, mbgba, RCPRM0,RCPRM1,ip, x); r2c[O+i] = x; K[i] = x; }   
  for(op = out; op < out+outlen;) { 	
    uint8_t  u = r2c[O+0];  
    unsigned k,r; 
	CXK; _mbgxdec(rcrange,rccode, &mbg0c[       cxk], mbguc[           cxk], mbgbc[cxk], RCPRM0K,RCPRM1K,ip, k, MTFD1(r2c,u), MTFD(r2c,k+1,u)); K[u] = PREDEMAK(K[u],k); 
	CXR;  mbgxdec(rcrange,rccode, &mbg0r[ku<<11|cxr], mbgur[(ku>0)<<11|cxr], mbgbr[u  ], RCPRM0R,RCPRM1R,ip, r);                             	R[u] = PREDEMAR(R[u],r); 
    r++; memset_(op, u, r);  	
  }
  return outlen;   
}
  #endif
