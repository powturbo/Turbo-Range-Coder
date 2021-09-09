/**
    Copyright (C) powturbo 2013-2020
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
// Turbo Range Code: templates include
//----------- Range Coder : order 0 -----------------------------------------------------------------------
#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <string.h>

#include "turborc.h"
#include "mb_o0.h"  

#define OVERFLOW(_in_,_inlen_) if(op >= out+(_inlen_*255)/256-8) { memcpy(out,_in_,_inlen_); op = out+_inlen_; goto e; }
// byte adaptive ---------------------------------------------------------------
size_t TEMPLATE3(rc,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out) {
  mbu           mb[0x100];             // predictor
  unsigned char *op = out, *ip; 
  rcencdef(rcrange,rclow);             // range coder
  
  mbu_init1(mb, 0x100);  
  rceinit(rcrange,rclow);
  
  for(ip = in; ip < in+inlen; ip++) {
    unsigned ch = *ip; 
    mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1,op, ch); 
    OVERFLOW(in,inlen); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t TEMPLATE3(rc,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out) {
  mbu           mb[0x100];           // predictor
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode);         // range coder
  
  mbu_init1(mb, 0x100);
  rcdinit(rcrange, rccode, ip);
  for(op = out; op < out+outlen; op++) { 
    unsigned ch; 
    mb8dec(rcrange, rccode, mb,RCPRM0,RCPRM1,ip, ch);
    *op = ch; 
  }
} 

// Nibble adaptive -------------------------------------------------------------
size_t TEMPLATE3(rc4,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out) {
  mbu           mb[0x10];             // predictor
  unsigned char *op = out, *ip; 
  rcencdef(rcrange,rclow);             // range coder
  
  mbu_init1(mb, 0x10);  
  rceinit(rcrange,rclow);
  
  for(ip = in; ip < in+inlen; ip++) {
    unsigned ch = *ip; 
    mb4enc(rcrange,rclow, mb,RCPRM0,RCPRM1,op, ch); 
    OVERFLOW(in,inlen); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t TEMPLATE3(rc4,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out) {
  mbu           mb[0x10];            // predictor
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode);         // range coder
  
  mbu_init1(mb, 0x10);
  rcdinit(rcrange, rccode, ip);
  for(op = out; op < out+outlen; op++) { 
    unsigned ch; 
    mb4dec(rcrange, rccode, mb,RCPRM0,RCPRM1,ip, ch); 
    *op = ch & 0xf; 
  }
} 

// Nibble static -----------------------------------------------------------------
size_t TEMPLATE3(rc4c,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out) {
  mbu           mb[0x10];             // predictor
  unsigned char *op = out, *ip; 
  rcencdef(rcrange,rclow);             // range coder
  
  mbu_init1(mb, 0x10);  
  rceinit(rcrange,rclow);
  
  for(ip = in; ip < in+inlen; ip++) {
    unsigned ch = *ip; 
    mb4senc(rcrange,rclow, mb, op, ch); 
    OVERFLOW(in,inlen); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t TEMPLATE3(rc4c,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out) {
  mbu           mb[0x10];            // predictor
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode);         // range coder
  
  mbu_init1(mb, 0x10);
  rcdinit(rcrange, rccode, ip);
  for(op = out; op < out+outlen; op++) { 
    unsigned ch; 
    mb4sdec(rcrange, rccode, mb,ip, ch); 
    *op = ch & 0xf; 
  }
} 
   
//------------------- Range Coder : order N bits with sliding context --------------------
#define MBC_C 8     // order 8bits context  (4 <= MBC_C <= 20)
#include "mb_on.h"
  
size_t TEMPLATE3(rcx,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out) {
  MBC_DEF(mbc, MBC_C);           // predictor with MBC_C context bits 
  unsigned char *op = out,*ip; 
  unsigned      cx = 0;
  rcencdef(rcrange,rclow);                 // range coder
  
  MBC_INIT(mbc, MBC_C); 
  rceinit(rcrange,rclow); 
  
  for(ip = in; ip < in+inlen; ip++) { 
    unsigned ch = *ip; 
    mbcenc(rcrange,rclow, cx, MBC_C,     mbc,RC1PRM0,RC1PRM1,op, ch);
    OVERFLOW(in,inlen);
  } 
  rceflush(rcrange,rclow, op); 
  e:return op - out;
}

size_t TEMPLATE3(rcx,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out) { 
  MBC_DEF(mbc, MBC_C);                    // predictor with MBC_C bits context
  unsigned char *ip = in, *op; 
  unsigned cx = 0;
  rcdecdef(rcrange, rccode);              // range coder
 
  rcdinit(rcrange, rccode,ip); 
  MBC_INIT(mbc, MBC_C); 
  for(op = out; op < out+outlen; op++) { 
    mbcdec(rcrange,rccode, cx, MBC_C, mbc,RC1PRM0,RC1PRM1,ip);
    *op = cx; 
  }
}

//------------------- Varible length coding : gamma---------------------------------------
#define MBR
#include "mb_vint.h"  

size_t TEMPLATE3(rcg,RC_PRED, enc8)(unsigned char *in, size_t inlen, unsigned char *out) {
  MBG_DEF(mbg0c, mbguc, mbgbc); 
  uint8_t *op = out, *ip; 
  rcencdef(rcrange,rclow);                       // range coder
  
  MBG_INIT(mbg0c, mbguc, mbgbc); 
  rceinit(rcrange,rclow);
  
  for(ip = in; ip < in+inlen; ip++) {         
    unsigned ch = ip[0]+1;
    mbgenc(rcrange,rclow, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,op, ch);
    OVERFLOW(in,inlen);
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t TEMPLATE3(rcg,RC_PRED,dec8)(unsigned char *in, size_t outlen, unsigned char *out) {
  MBG_DEF(mbg0c, mbguc, mbgbc);            // predictor
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode);               // range coder
  
  MBG_INIT(mbg0c, mbguc, mbgbc);
  rcdinit(rcrange, rccode, ip);
  
  for(op = out; op < out+outlen; op++) { 
    unsigned ch;
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, ch);
    *op = ch-1; 
  }
} 
  
size_t TEMPLATE3(rcg,RC_PRED,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip;
  size_t        inlen = (_inlen+1)/2;

  rcencdef(rcrange,rclow);                       // range coder
 
  MBG_DEF(mbg0c, mbguc, mbgbc); 
  MBG_INIT(mbg0c, mbguc, mbgbc); 
  rceinit(rcrange,rclow);
  
  for(ip = in; ip < in+inlen; ip++) {         
    unsigned ch = ip[0]+1; 
    mbgenc(rcrange,rclow, &mbg0c, mbguc,mbgbc,RCPRM0,RCPRM1,op, ch);
    OVERFLOW(_in,_inlen);
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t TEMPLATE3(rcg,RC_PRED,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out) { 
  MBG_DEF(mbg0c, mbguc, mbgbc);            // predictor
  unsigned char *ip = in;
  uint16_t      *out = _out, *op;
  size_t        outlen = (_outlen+1)/2;

  rcdecdef(rcrange, rccode);               // range coder
  
  MBG_INIT(mbg0c, mbguc, mbgbc);
  rcdinit(rcrange, rccode, ip);
 
  for(op = out; op < out+outlen; op++) { 
    unsigned ch;
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, ch);
    *op = ch-1;
  }
}

size_t TEMPLATE3(rcg,RC_PRED,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  MBG_DEF(mbg0c, mbguc, mbgbc); 
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip;
  size_t        inlen = (_inlen+3)/4;

  rcencdef(rcrange,rclow);                       // range coder
  
  MBG_INIT(mbg0c, mbguc, mbgbc); 
  rceinit(rcrange,rclow);
  
  for(ip = in; ip < in+inlen; ip++) {         
    uint64_t ch = (uint64_t)ip[0]+1;
    mbgenc(rcrange,rclow, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,op, ch);
    OVERFLOW(_in,_inlen);
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t TEMPLATE3(rcg,RC_PRED,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out) { 
  MBG_DEF(mbg0c, mbguc, mbgbc);            // predictor
  unsigned char *ip = in;
  uint32_t      *out = _out, *op;
  size_t        outlen = (_outlen+3)/4;

  rcdecdef(rcrange, rccode);               // range coder
  
  MBG_INIT(mbg0c, mbguc, mbgbc);
  rcdinit(rcrange, rccode, ip);

  for(op = out; op < out+outlen; op++) { 
    uint64_t ch;
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, ch);
    *op = ch - 1;
  }
}

//------------------- RLE : run length encoded in gamma+rc ----------------------------------
size_t TEMPLATE3(rcrle,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out) { 
  MBG_DEF(mbg0, mbgu, mbgb);                    // run length predictor
  mbu           mb[0x100];                      // byte predictor
  rcencdef(rcrange,rclow);                      // range coder
  unsigned char *ip = in, *in_ = in+inlen, *op = out; 

  mbu_init1(mb, 0x100);  
  MBG_INIT(mbg0, mbgu, mbgb);
  rceinit(rcrange,rclow);

  while(ip < in_) { 
    unsigned char *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; // run length

    mbgenc(rcrange,rclow, &mbg0,mbgu,mbgb,RCPRM0,RCPRM1,op, r);
    mb8enc(rcrange,rclow,              mb,RCPRM0,RCPRM1,op, u);
    OVERFLOW(in,inlen);
  }
  rceflush(rcrange,rclow, op); 
  e:return op-out;
}

#define memset_(_op_, _c_, _n_) do *_op_++ = _c_; while(--_n_)

size_t TEMPLATE3(rcrle,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out) {
  mbu           mb[0x100];                      // byte predictor
  MBG_DEF(mbg0, mbgu, mbgb);                    // run length predictor
  rcencdef(rcrange,rccode);                     // range coder
  unsigned char *ip = in, *op = out; 

  mbu_init1(mb, 0x100);  
  MBG_INIT(mbg0, mbgu, mbgb);
  rcdinit(rcrange, rccode, ip);

  while(op < out+outlen) { 
    unsigned r,ch;
    mbgdec(rcrange,rccode, &mbg0,mbgu,mbgb,RCPRM0,RCPRM1,ip, r);
    mb8dec(rcrange,rccode,              mb,RCPRM0,RCPRM1,ip, ch);
    memset_(op, ch, r);
  }
  return ip-in;   
}

size_t TEMPLATE3(rcrlex,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out) { 
  MBC_DEF(mbc, MBC_C);                             // predictor with MBC_C context bits 
  MBG_DEF2(mbg0, 0x100, mbgu, 0x100, mbgb, 0x100); // run length predictor
  rcencdef(rcrange,rclow);                         // range coder
  unsigned      cx = 0;
  unsigned char *ip = in, *in_ = in+inlen, *op = out; 

  MBC_INIT(mbc, MBC_C); 
  MBG_INIT2(mbg0, 0x100, mbgu, 0x100, mbgb, 0x100);
  rceinit(rcrange,rclow);

  while(ip < in_) { 
    unsigned char *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; // r:run length

    mbcenc(rcrange,rclow, cx, MBC_C, mbc,RCPRM0,RCPRM1,op, u);
    mbgenc(rcrange,rclow, &mbg0[u], mbgu[u], mbgb[u], RCPRM0,RCPRM1,op, r);
    OVERFLOW(in,inlen);
  }
  rceflush(rcrange,rclow, op); 
  e:return op-out;
}

size_t TEMPLATE3(rcrlex,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out) {
  MBC_DEF(mbc, MBC_C);                             // predictor with MBC_C bits context
  MBG_DEF2(mbg0, 0x100, mbgu, 0x100, mbgb, 0x100); // run length predictor
  rcencdef(rcrange,rccode);                        // range coder
  unsigned      cx = 0;
  unsigned char *ip = in, *op = out; 

  MBC_INIT(mbc, MBC_C); 
  MBG_INIT2(mbg0, 0x100, mbgu, 0x100, mbgb, 0x100);
  rcdinit(rcrange, rccode, ip);

  while(op < out+outlen) { 
    unsigned r;
    unsigned char ch;
    mbcdec(rcrange,rccode, cx, MBC_C, mbc, RCPRM0,RCPRM1, ip);
    ch = cx; mbgdec(rcrange,rccode, &mbg0[ch], mbgu[ch], mbgb[ch], RCPRM0,RCPRM1,ip, r);
    memset_(op, ch, r);
  }
  return ip-in;   
}
//--------------------------------------- MTF/QLFC Run Length Encoding ------------------------------------------------------------------------
#undef RC_SIZE            
#undef RC_IO
#undef RC_BITS
#undef mbu
#undef mbu_p
#undef mbu_init
#undef mbu_update0
#undef mbu_update1

  #ifdef _RC_S_            // redefine optimal RC_BITS+parameters for qlfc/bwt
#define RC_BITS 10
#include "mbc_s.h"     
  #elif defined(_RC_SS_)
#define RC_BITS 15          
#include "mbc_ss.h"         
  #elif defined(_RC_NZ_)
#define RC_BITS 24  
#define RCPRM0  37
#include "mbc_nz.h"         
  #elif defined(_RC_SH_)
#define RC_BITS 15
#include "mbc_sh.h"         
  #endif
#include "turborc_.h"

enum { CL0=8,CL=8, RL0=8,RL=8 };                                                                            //#define bitglen(_x_) ((__bsr32(_x_)<<1)+1)
size_t TEMPLATE3(rcqlfc,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out) {
  unsigned char rmtf[0x100], *ip = in, *in_,*op = out, *_rk = malloc(inlen+1), *rk; if(!_rk) { fprintf(stderr, "allocate memory error.\n"); exit(-1); }    
  MBG_DEC( mbg0a,           mbgua,          mbgba);         		//mtf
  MBG_DEC2(mbg0c, (1<<CL0), mbguc, (1<<CL), mbgbc, 1<<CL);  		//rank
  MBG_DEC2(mbg0r, (1<<RL0), mbgur, (1<<RL), mbgbr, 1<<RL);  		//run length
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);                  //range coder
  																											
  rk = rcqlfc(in, _rk, inlen, rmtf);                                                                        //size_t st_m=0,st_k=0,st_r=0,st_kbits[0x33]={0},st_rbits[0x33]={0};
  for(unsigned cx = 0; cx < 0x100; cx++) { mbgenc(rcrange,rclow, &mbg0a, mbgua, mbgba, RCPRM0,RCPRM1,op, (rmtf[cx]+1)); /*st_m+=bitglen(rmtf[cx]+1);*/ }
  
  for(ip = in, in_ = in+inlen; ip < in_;) {
    unsigned char *p = ip, u = *ip++; while(ip < in_ && *ip == u) ip++; // run length encoding
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p, k = *--rk;                                     	//st_k+=bitglen(k+1); st_kbits[bsr32(k)]++;
    mbgenc(rcrange,rclow, &mbg0c[u], mbguc[u], mbgbc[u], RCPRM0,RCPRM1,op, (k+1));                         	//st_r+=bitglen(r);   st_rbits[bsr32(r)]++; 																										
    mbgenc(rcrange,rclow, &mbg0r[u], mbgur[u], mbgbr[u], RCPRM0,RCPRM1,op,    r);
    OVERFLOW(in,inlen);
  }																										  	//printf("bits=%zuB+%zuMB+%zuMB=%zuMB\n", (st_m+7)/8, (st_k+7)/(8*1024*1024), (st_r+7)/(8*1024*1024), (st_m+st_k+st_r+7)/(8*1024*1024));	
  rceflush(rcrange,rclow, op);																			  	//printf("kbits:"); for(int i=0;i<33;i++) if(st_kbits[i]) printf("%u,%zu ", i, st_kbits[i]/1024); printf("\n");
  e:free(_rk);                                                                                            	//printf("rbits:"); for(int i=0;i<33;i++) if(st_rbits[i]) printf("%u,%zu ", i, st_rbits[i]/1024); printf("\n");																										  
  return op - out;
}

#define O 16
#define MTFD1(mtf,u) mtf[O] = mtf[O+1], mtf[O+1] = u
#define MTFD(mtf,k,u)\
  if(k <= O) {\
    unsigned char *_c = &mtf[k];\
    ctou64(&_c[ 0]) = ctou64(&_c[0+1]); ctou64(&_c[ 8]) = ctou64(&_c[8+1]);\
    _c[O] = u;\
  } else {\
    ctou64(&mtf[O+0]) = ctou64(&mtf[O+1]); ctou64(&mtf[O+8]) = ctou64(&mtf[O+9]);\
    unsigned char *_p; for(_p = mtf+O+16; _p != mtf+O+k; ++_p) _p[0] = _p[1]; *_p = u;\
  }

size_t TEMPLATE3(rcqlfc,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out) { 
  unsigned char mtf[257+O],*op,*ip = in, *p;
  MBG_DEC( mbg0a,           mbgua,          mbgba);        
  MBG_DEC2(mbg0c, (1<<CL0), mbguc, (1<<CL), mbgbc, 1<<CL);
  MBG_DEC2(mbg0r, (1<<RL0), mbgur, (1<<RL), mbgbr, 1<<RL); 
  rcencdef(rcrange,rccode); rcdinit(rcrange, rccode, ip);         // range coder                                                                        
  unsigned x,cx;
  																												
  for(cx = 0; cx < 0x100; cx++) { mbgdec(rcrange,rccode, &mbg0a, mbgua, mbgba, RCPRM0,RCPRM1,ip, x); mtf[O+cx] = x-1; }   
  for(cx = 0,op = out; op < out+outlen;) { 																	
    unsigned char u = mtf[O+0];                                                      
    unsigned k,r; _mbgdec(rcrange,rccode, &mbg0c[u], mbguc[u], mbgbc[u], RCPRM0,RCPRM1,ip, k, MTFD1(mtf,u), MTFD(mtf,k,u));
                  mbgdec(rcrange,rccode, &mbg0r[u], mbgur[u], mbgbr[u], RCPRM0,RCPRM1,ip, r);
    memset_(op, u, r);               
  }
  return outlen;   
}

  #ifdef _BWT
    #ifdef _BWTDIV      
#include "libdivsufsort/include/divsufsort.h"
int obwt_unbwt_biPSIv2(const unsigned char *T, unsigned char *U, unsigned *PSI, int n, int pidx);
    #else
#include "libbsc/libbsc/bwt/libsais/libsais.h"
typedef unsigned saidx_t;
    #endif

size_t TEMPLATE3(rcbwt,RC_PRED,enc)(unsigned char *in, size_t inlen, unsigned char *out, unsigned lev, unsigned thnum, unsigned lenmin) {
  unsigned char *op     = out; 
  unsigned char *_bwt   = malloc(inlen), *ip = in; if(!_bwt) { op = out+inlen; goto e; }  
  size_t         iplen  = inlen;

  if(lenmin>256) lenmin = 256; 															
  if(lenmin) {      
    lenmin = (lenmin+15)/16;
    ip    = _bwt;
    iplen = lzpenc(in, inlen, ip, lenmin*16);
	if(iplen+(inlen>>4)+256 > inlen) { //Not enough saving, 		//printf("r=%.2f#\n", (double)iplen*100.0/inlen); 
      ip = in; iplen = inlen; lenmin = 0; 		
    } 																//else printf("r=%.2f\n", (double)iplen*100.0/inlen);
  }                    												

  saidx_t   *sa = (saidx_t *)malloc(iplen*sizeof(sa[0])); if(!sa) { op = out+inlen; goto e; }
            *op++ = lenmin; 
  if(lenmin) ctou32(op) = iplen, op += 4;
    #ifdef _BWTDIV
  *(saidx_t *)op  = divbwt(ip, _bwt, sa, iplen); 
              op += sizeof(sa[0]);
    #else
        int mod = iplen / 8;
            mod |= mod >> 1; 	mod |= mod >> 2;
            mod |= mod >> 4;  	mod |= mod >> 8;
            mod |= mod >> 16; 	mod >>= 1;
   unsigned idxs[256];
   libsais_bwt_aux(ip, _bwt, sa, iplen, 0, mod + 1, idxs); 		 //libsais_bwt(ip, _bwt, sa, iplen, fs);
   int nidxs = (unsigned char)((iplen - 1) / (mod + 1));			 
   memcpy(op, idxs, (nidxs+1)*sizeof(idxs[0]));
   op   +=          (nidxs+1)*sizeof(idxs[0]);    
    #endif
  free(sa);

  op += rcqlfcsenc(_bwt, iplen, op);
  e: if(_bwt) free(_bwt);
  if(op - out >= inlen) { memcpy(out, in, inlen); return inlen; }
  return op - out;
}

size_t TEMPLATE3(rcbwt,RC_PRED,dec)(unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned thnum) {
  unsigned char *ip    = in;
  unsigned      lenmin = *ip++;
  size_t        oplen  = outlen;
  if(lenmin)    oplen  = ctou32(ip),     ip += 4;
  
    #ifdef _BWTDIV
  saidx_t       bwtidx = *(saidx_t *)ip; ip += sizeof(saidx_t);
    #else
        int mod  = oplen / 8;
            mod |= mod >> 1;  mod |= mod >> 2;
            mod |= mod >> 4;  mod |= mod >> 8;
            mod |= mod >> 16; mod >>= 1;	
  int nidxs = (unsigned char)((oplen - 1) / (mod + 1));			 
  unsigned idxs[256];
  memcpy(idxs, ip, (nidxs+1)*sizeof(idxs[0])); ip += (nidxs+1)*sizeof(idxs[0]);
    #endif
  unsigned char *bwt = malloc(oplen),*op = out;          if(!bwt) die("malloc failed\n"); 
  rcqlfcsdec(ip, oplen, bwt);

  saidx_t *sa = (saidx_t *)malloc((oplen+1)*sizeof(sa[0])); if(!sa) { free(bwt); die("malloc failed\n"); }
  if(lenmin) { op = malloc(oplen); if(!op) die("malloc failed\n"); }
    
    #ifdef _BWTDIV
  size_t rc = obwt_unbwt_biPSIv2(bwt, op, sa, oplen, bwtidx); //size_t rc = inverse_bw_transform(bwt, op, sa, oplen, bwtidx);
    #else
  size_t rc = libsais_unbwt_aux(bwt, op, sa, oplen, mod+1, idxs); //libsais_unbwt(bwt, op, sa, oplen, idxs[0]);    
    #endif
  free(sa);
  free(bwt);

  if(lenmin) {
    lzpdec(op, outlen, out, lenmin*16);
    free(op);
  }
  return rc;
}
  #elif defined(_EXT)
#include "ext_.c"
  #endif


