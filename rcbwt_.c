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
// Turbo Range Coder bwt: templates include
#include "rcutil_.h"
#include "mb_vint.h"  

#include "turborc.h"
#include "rcutil.h"
#include "bec.h"
  #ifdef _LIBSAIS
#include "libsais/src/libsais.h"
  #endif

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

#define QMAXK 8   																// length limited rice threshold 
#define QMAXR 8
#define PREDEMAK(_avg_,_x_) EMA8( _avg_,5,_x_) 
#define PREDEMAR(_avg_,_x_) EMA16(_avg_,11,(_x_)>31?31:(_x_))                   // max. avg 31
#define CXK unsigned cxk = RICEK(K[u]>31?31:K[u]) << 8 | u   					// cxk: 3+8bits
#define CXR unsigned ku,cxr = RICEK(R[u]) << 8 | u; ku = RICEK(K[u]>14?14:K[u]) // cxr: 3+8 = 11, ku:2bits
extern int bwtx, forcelzp;
enum { KU0=11,KU=11,KB=11, RU0=13,RU=12,RB=8 };                                                                         

#ifndef NO_COMP			
//extern FILE *fdbg;
size_t T3(rcqlfc,RC_PRD,enc)(uint8_t *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t _r2cr[(1<<8)+32], *r2cr = &_r2cr[32], *ip = in, *in_,*op = out, *_rk = vmalloc((inlen+1)*sizeof(in[0])), *rk; if(!_rk) die("malloc failed. size=%u\n", inlen);   
  MBG_DEC( mbg0a,         mbgua,        mbgba,        33, 33);       			//initial mtf r2c 
  MBG_DEC2(mbg0c, 1<<KU0, mbguc, 1<<KU, mbgbc, 1<<KB, 33, 33);  				//rank
  MBG_DEC2(mbg0r, 1<<RU0, mbgur, 1<<RU, mbgbr, 1<<RB, 33, 33);  				//run length
  rcencdec(rcrange,rclow); 							                     		//range coder
  
  uint8_t K[1<<8], R[1<<8] = {1};
  rk = rcqlfc(in, inlen, _rk, r2cr);                                                       
  for(unsigned cx = 0; cx < (1<<8); cx++) { mbgenc(rcrange,rclow, &mbg0a, mbgua, mbgba, RCPRM0,RCPRM1,op, r2cr[cx]); K[cx] = r2cr[cx]; }
  for(ip = in, in_ = in+inlen; ip < in_;) {
    unsigned k = *--rk,r;
    uint8_t  u = *ip++, *p = ip; while(ip < in_ && *ip == u) ip++; r = ip - p;  // run length encoding      fwrite(&r, 1, 4, fdbg); 																							  
	CXK; mbgxenc(rcrange,rclow, &mbg0c[cxk],        mbguc[cxk],            mbgbc[cxk], RCPRM0K,RCPRM1K,op, k); K[u] = PREDEMAK(K[u],k);
	CXR; mbgxenc(rcrange,rclow, &mbg0r[ku<<11|cxr], mbgur[(ku>0)<<11|cxr], mbgbr[u],   RCPRM0R,RCPRM1R,op, r); R[u] = PREDEMAR(R[u],r);
																				//TODO: rcvgenc32 for r
																				OVERFLOW(in,inlen, out, op, goto e);
  }	
  rceflush(rcrange,rclow, op);													OVERFLOW(in,inlen, out,op,;);
  e:vfree(_rk); 																								// if(fdbg) fclose(fdbg);      
  return op - out;
}
#endif 

size_t T3(rcqlfc,RC_PRD,dec)(uint8_t *in, size_t outlen, uint8_t *out RCPRM) {
  uint8_t r2c[257+O], *op, *ip = in, *p, K[256], R[256] = {1};;
  unsigned i;
  MBG_DEC( mbg0a,         mbgua,        mbgba,        33, 33);        
  MBG_DEC2(mbg0c, 1<<KU0, mbguc, 1<<KU, mbgbc, 1<<KB, 33, 33);
  MBG_DEC2(mbg0r, 1<<RU0, mbgur, 1<<RU, mbgbr, 1<<RB, 33, 33); 
  rcencdef(rcrange,rccode); rcdinit(rcrange, rccode, ip);         
  
  for(i = 0; i < (1<<8); i++) { unsigned x; mbgdec(rcrange,rccode, &mbg0a, mbgua, mbgba, RCPRM0,RCPRM1,ip, x); r2c[O+i] = x; K[i] = x; }   
  for(op = out; op < out+outlen;) { 	
    uint8_t u = r2c[O+0];  
    unsigned k,r; 
	CXK; _mbgxdec(rcrange,rccode, &mbg0c[cxk],        mbguc[cxk],            mbgbc[cxk], RCPRM0K,RCPRM1K,ip, k, MTFD1(r2c,u), MTFD(r2c,k+1,u)); K[u] = PREDEMAK(K[u],k); 
	CXR;  mbgxdec(rcrange,rccode, &mbg0r[ku<<11|cxr], mbgur[(ku>0)<<11|cxr], mbgbr[u],   RCPRM0R,RCPRM1R,ip, r);                             	R[u] = PREDEMAR(R[u],r); 
    r++; memset_(op, u, r);  	
  }
  return outlen;   
}

//------------------------------------------- bwt ----------------------------------------------------------  
  #ifdef _BWTDIV      
#include "libdivsufsort/include/divsufsort.h"
#include "libdivsufsort/include/unbwt.h"
  #else
typedef int32_t saidx_t;	  
  #endif
    
static unsigned calcmod(size_t len) {
  int mod  = len / 8;
      mod |= mod >> 1; 	mod |= mod >> 2;
      mod |= mod >> 4;  mod |= mod >> 8;
      mod |= mod >> 16; mod >>= 1;
  return mod;			
}	

  #ifndef NO_COMP
size_t T3(rcbwt,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out, unsigned lev, unsigned thnum, unsigned _lenmin RCPRM) { 
  unsigned char *op    = out;              
  unsigned char *bwt   = vmalloc(inlen+1024), *ip = in; if(!bwt) { op = out+inlen; goto e; }  // inlen + space for bwt indexes idxns
  size_t        iplen  = inlen;
  unsigned      lenmin = _lenmin, xbwt16 = (_lenmin & BWT_BWT16)?0x80:0, verbose = _lenmin & BWT_VERBOSE; lenmin &= lenmin & 0x3ff; 
																				if(verbose) { printf("level=%u ", lev);fflush(stdout); } 
  if(lenmin) {  																if(verbose) { printf("lenmin=%u ", lenmin);fflush(stdout); } 
    ip    = bwt;
    iplen = utf8enc(in, inlen, ip, _lenmin);									// try utf8 preprocessing
	if(iplen != inlen) lenmin = 0x1f;   										// lenmin = 31 for utf8enc success
	else {
      lenmin = ((lenmin>480?480:lenmin)+15)/16; 								// lenmin 0-30, 31:utf8enc
      ip     = bwt;
      iplen  = lzpenc(in, inlen, ip, lenmin*16);
	  if(iplen+(inlen>>4)+256 > inlen && !forcelzp) { /*Not enough saving*/		if(verbose) { printf("r=%.2fNoLzp ", lenmin*16, (double)iplen*100.0/inlen);fflush(stdout); }  
        ip = in; iplen = inlen; lenmin = 0;		
      } 																		else if(verbose) { printf("r=%.2fLzp ",  (double)iplen*100.0/inlen);fflush(stdout); } 
	}
  } 
  *op++ = lenmin|xbwt16; 
  if(lenmin) ctou32(op) = iplen, op += 4; 
    #ifdef _BWTDIV
  *op++ = 0;
  saidx_t *sa   = (saidx_t *)vmalloc((iplen+2)*sizeof(sa[0])); if(!sa) { op = out+inlen; goto e; }
  *(saidx_t *)op  = divbwt(ip, bwt, sa, iplen); 
              op += sizeof(sa[0]);
    #else
  unsigned idxs[256], iplen_ = xbwt16?(iplen/2):iplen, mod = calcmod(iplen_), idxsn = (unsigned char)((iplen_-1) / (mod + 1)); // TODO:truncate to mod 8
  *op++ = idxsn; 
  saidx_t *sa = (saidx_t *)vmalloc((iplen_+2)*sizeof(sa[0])); if(!sa) { op = out+inlen; goto e; }	if(verbose) { printf("xbwt16=%u ", xbwt16>0);fflush(stdout); } 
	  #ifdef _LIBSAIS16	                                                                        
  if(xbwt16) { 																	if(verbose) { printf("-"); fflush(stdout); } 
    unsigned rc = libsais16_bwt_aux(ip, bwt, sa, iplen_, 0, 0, mod+1, idxs); 	if(verbose) { printf("+"); fflush(stdout); } 
    if(iplen & 1) bwt[iplen-1] = ip[iplen-1]; 
  }  else 
	  #endif
	libsais_bwt_aux(ip, bwt, sa, iplen,  0, 0, mod+1, idxs); 		         //libsais_bwt(ip, bwt, sa, iplen, fs);
  memcpy(op, idxs, (idxsn+1)*sizeof(idxs[0]));
  op   +=          (idxsn+1)*sizeof(idxs[0]);    
    #endif
  vfree(sa);
  switch(lev) {
    case  0: memcpy(op, bwt, iplen); op += iplen; vfree(bwt); if(op-out == inlen) op++; return op - out; break;
	case  1: op += xbwt16?becenc16(bwt, iplen, op):becenc8(bwt, iplen, op); break;
    case  2: op += rcqlfcsenc(   bwt, iplen, op); break;
    case  3: op += rcqlfcssenc(  bwt, iplen, op, 4, 7); break;
	case  4: op += xbwt16?rcrlesenc16(  bwt, iplen, op):     rcrlesenc(bwt, iplen, op); break;
	case  5: op += xbwt16?rcrlessenc16( bwt, iplen, op, 2,6):rcrlessenc( bwt, iplen, op, 2,6); break;
	case  6: op += xbwt16?rcrle1senc16( bwt, iplen, op):     rcrle1senc( bwt, iplen, op); break;
	case  7: op += xbwt16?rcrle1ssenc16(bwt, iplen, op, 2,6):rcrle1ssenc(bwt, iplen, op, 2,6); break;
    default: op += T3(rcqlfc,RC_PRD,enc)(bwt, iplen, op RCPRMC); break;
  }																								OVERFLOW(in,inlen,out, op, goto e); 
  e: if(bwt) vfree(bwt);														
  return op - out;
}
  #endif
  
size_t T3(rcbwt,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned thnum RCPRM) {
  unsigned char *ip    = in;                     
  unsigned      lenmin = *ip++, xbwt16 = lenmin&0x80; lenmin &=0x1f;
  size_t        oplen  = outlen, rc;
  
  if(lenmin) oplen = ctou32(ip),ip += 4;    
  
    #ifdef _BWTDIV
  ip++;
  saidx_t       bwtidx = *(saidx_t *)ip; ip += sizeof(saidx_t);
    #else
  unsigned idxs[256];
  int oplen_ = xbwt16?oplen/2:oplen, mod = calcmod(oplen_), idxsn = (unsigned char)((oplen_ - 1) / (mod+1));		
  ip++; // idxsn
  memcpy(idxs, ip, (idxsn+1)*sizeof(idxs[0])); ip += (idxsn+1)*sizeof(idxs[0]);
    #endif
  unsigned char *_bwt = vmalloc(oplen), *op = out, *bwt = _bwt;  if(!_bwt) die("malloc failed\n"); 
    {      if(lenmin) { bwt = out; op = _bwt; } }  
  switch(lev) {
    case  0: memcpy(bwt,    ip, oplen+bwtx);      break;
	case  1: xbwt16?becdec16(ip, oplen+bwtx, bwt):becdec8(ip, oplen+bwtx, bwt); break;
	case  2: rcqlfcsdec(    ip, oplen+bwtx, bwt); break;
	case  3: rcqlfcssdec(   ip, oplen+bwtx, bwt, 4, 7); break;
	case  4: xbwt16?rcrlesdec16(  ip, oplen+bwtx, bwt):      rcrlesdec(  ip, oplen+bwtx, bwt); break;
	case  5: xbwt16?rcrlessdec16( ip, oplen+bwtx, bwt, 2, 6):rcrlessdec( ip, oplen+bwtx, bwt, 2, 6); break;
	case  6: xbwt16?rcrle1sdec16( ip, oplen+bwtx, bwt):      rcrle1sdec( ip, oplen+bwtx, bwt); break;
	case  7: xbwt16?rcrle1ssdec16(ip, oplen+bwtx, bwt, 2, 6):rcrle1ssdec(ip, oplen+bwtx, bwt, 2, 6); break;
    default: T3(rcqlfc,RC_PRD,dec)(ip, oplen+bwtx, bwt RCPRMC); break;
  }
  saidx_t *sa = (saidx_t *)vmalloc((oplen+2)*sizeof(sa[0])); if(!sa) { vfree(bwt); die("malloc failed\n"); }
   
    #ifdef _BWTDIV
  rc = obwt_unbwt_biPSIv2(bwt, op, sa, oplen, bwtidx);
    #else
	  #ifdef _LIBSAIS16 
  if(xbwt16) { rc = libsais16_unbwt_aux(bwt, op, sa, oplen_, 0, mod+1, idxs); if(oplen & 1) op[oplen-1] = bwt[oplen-1]; }
  else
	  #endif
    rc = libsais_unbwt_aux(bwt, op, sa, oplen, 0, mod+1, idxs); 						//libsais_unbwt(bwt, op, sa, oplen, idxs[0]);  
    #endif
  vfree(sa);

  if(lenmin)
    lenmin == 0x1f?utf8dec(op, outlen, out):lzpdec(op, outlen, out, lenmin*16);
  vfree(_bwt);
  return rc;
}
