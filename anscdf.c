/**
    Copyright (C) powturbo 2013-2023
    SPDX-License-Identifier: GPL v3 License

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
// TurboRANS Range Asymmetric Numeral Systems 
  #ifdef __APPLE__
#include <sys/malloc.h>
  #else
#include <malloc.h>
  #endif
#include "include/anscdf.h"
#include "include_/conf.h" 
#define _DIVTDEF32
#include "include/turborc.h"
#include "anscdf_.h"
 
  #ifdef __AVX2__
#define FSUFFIX x
  #elif defined(__SSE2__) || defined(__ARM_NEON) || defined(__powerpc64__)
#define FSUFFIX s
  #else
#define FSUFFIX 0
  #endif
  
  #ifndef min
#define min(x,y) (((x)<(y)) ? (x) : (y))
#define max(x,y) (((x)>(y)) ? (x) : (y))
  #endif
  
LIBAPI unsigned T2(anscdfdec,FSUFFIX)(unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize) {
  unsigned char *op = out, *out_ = out+outlen, *ip=in;  
  unsigned oplen,i;
  anscdfini(0);

  for(;out < out_; out += oplen) {  
    STATEDEF(st);
    CDF16DEC0(mb0);
    CDF16DEC1(mb,16);  
    CDF16DEF; 
    mnfill(st, ip);
	oplen = out_ - out;  oplen = min(oplen, blksize);
    for(; op < out+(oplen&~3);op+=4) {  
      mn8dec(mb0,mb,op[0]); 
      mn8dec(mb0,mb,op[1]); 
      mn8dec(mb0,mb,op[2]);    
      mn8dec(mb0,mb,op[3]);  
    } 
    for(; op < out+oplen;op++) mn8dec(mb0,mb,op[0]); 
  }
  return outlen;   
}
 
LIBAPI unsigned T2(anscdfenc,FSUFFIX)(unsigned char *in, unsigned inlen, unsigned char *out, unsigned osize, unsigned blksize) {
  size_t isize = inlen*2+256, iplen; 
  unsigned char *op = out,*ip=in,*in_ = in+inlen, *out_ = out+osize; 
  mbu *_stk = malloc(isize*sizeof(_stk[0])),*stk=_stk; if(!_stk) die("malloc error %d ", isize);      			                      
  anscdfini(0);
  
  for(; in < in_; in += iplen) { 
    CDF16DEC0(mb0); 
    CDF16DEC1(mb,16);
    CDF16DEF;
    iplen = in_-in; iplen = min(iplen, blksize);
    for(; ip < in + (iplen&~3); ip += 4) {
	  mn8enc(mb0,mb,ip[0],stk); 
	  mn8enc(mb0,mb,ip[1],stk); 
	  mn8enc(mb0,mb,ip[2],stk); 
	  mn8enc(mb0,mb,ip[3],stk); 
    }
    for(; ip < in + iplen; ip++) 
	  mn8enc(mb0,mb,ip[0],stk);
    mnflush(op,out_,_stk,stk);
  } 
  free(_stk);
  return op - out;    
}  

LIBAPI unsigned T2(anscdf4dec,FSUFFIX)(unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize) { 
  unsigned char *op = out, *out_ = out+outlen, *ip = in;  
  unsigned      oplen,i;
  anscdfini(0);
  
  for(;out < out_; out += oplen) { 
    STATEDEF(st);
    CDF16DEC0(mb); 
    CDF16DEF;  
    mnfill(st,ip); 
	oplen = out_-out; oplen = min(oplen, blksize);
    for(; op < out+(oplen&~3);op+=4) { 
      mn4dec0(ip,mb,op[0],0); 
      mn4dec0(ip,mb,op[1],1); 
      mn4dec0(ip,mb,op[2],0); 
      mn4dec0(ip,mb,op[3],1); 
    }
    for(; op < out+oplen;op++) 
	  mn4dec0(ip,mb,op[0],0);
  }
  return outlen;  
}
 
LIBAPI unsigned T2(anscdf4enc,FSUFFIX)(unsigned char *in, unsigned inlen, unsigned char *out, unsigned osize, unsigned blksize) { 	
  size_t isize = inlen*2+256, iplen; 
  mbu *_stk = malloc(isize*sizeof(_stk[0])),*stk=_stk; if(!_stk) die("malloc error %d ", isize);      			                      
  unsigned char *op = out,*ip=in,*in_ = in+inlen, *out_=out+osize; 
  anscdfini(0);

  for(; in < in_; in += iplen) { 
    CDF16DEC0(mb); 
    CDF16DEF;
    iplen = in_-in; iplen = min(iplen, blksize);
    for(; ip < in + (iplen&~3); ip += 4) { 
      mn4enc(mb,ip[0],1,stk);  
      mn4enc(mb,ip[1],0,stk);
      mn4enc(mb,ip[2],1,stk);
      mn4enc(mb,ip[3],0,stk);
    }
    for(; ip < in + iplen; ip++) 
	  mn4enc(mb,ip[0],0,stk); 
    mnflush(op,out_,_stk,stk);
  }
  free(_stk);
  return op - out;    

} 

  #if !defined(__SSE3__) && !defined(__ARM_NEON)
  static unsigned _cpuisa;
//--------------------- CPU detection -------------------------------------------
    #if defined(__i386__) || defined(__x86_64__)
      #if _MSC_VER >=1300
#include <intrin.h>
      #elif defined (__INTEL_COMPILER)
#include <x86intrin.h>
      #endif

static inline void cpuid(int reg[4], int id) {
      #if defined (_MSC_VER) //|| defined (__INTEL_COMPILER)
  __cpuidex(reg, id, 0);
      #elif defined(__i386__) || defined(__x86_64__)
  __asm("cpuid" : "=a"(reg[0]),"=b"(reg[1]),"=c"(reg[2]),"=d"(reg[3]) : "a"(id),"c"(0) : );
      #endif
}

static inline uint64_t xgetbv (int ctr) {
      #if(defined _MSC_VER && (_MSC_FULL_VER >= 160040219) || defined __INTEL_COMPILER)
  return _xgetbv(ctr);
      #elif defined(__i386__) || defined(__x86_64__)
  unsigned a, d;
  __asm("xgetbv" : "=a"(a),"=d"(d) : "c"(ctr) : );
  return (uint64_t)d << 32 | a;
      #else
  unsigned a=0, d=0;
  return (uint64_t)d << 32 | a;
      #endif
}
    #endif

#define AVX512F     0x001
#define AVX512DQ    0x002
#define AVX512IFMA  0x004
#define AVX512PF    0x008
#define AVX512ER    0x010
#define AVX512CD    0x020
#define AVX512BW    0x040
#define AVX512VL    0x080
#define AVX512VNNI  0x100
#define AVX512VBMI  0x200
#define AVX512VBMI2 0x400

#define IS_SSE       0x10
#define IS_SSE2      0x20
#define IS_SSE3      0x30
#define IS_SSSE3     0x32
#define IS_POWER9    0x34 // powerpc
#define IS_NEON      0x38 // arm neon
#define IS_SSE41     0x40
#define IS_SSE41x    0x41 //+popcount
#define IS_SSE42     0x42
#define IS_AVX       0x50
#define IS_AVX2      0x60
#define IS_AVX512    0x800

unsigned cpuisa(void) {
  int c[4] = {0};
  if(_cpuisa) return _cpuisa;
  _cpuisa++;
    #if defined(__i386__) || defined(__x86_64__)
  cpuid(c, 0);
  if(c[0]) {
    cpuid(c, 1);
    //family = ((c >> 8) & 0xf) + ((c >> 20) & 0xff)
    //model  = ((c >> 4) & 0xf) + ((c >> 12) & 0xf0)
    if( c[3] & (1 << 25)) {         _cpuisa  = IS_SSE;
    if( c[3] & (1 << 26)) {         _cpuisa  = IS_SSE2;
    if( c[2] & (1 <<  0)) {         _cpuisa  = IS_SSE3;
      //                            _cpuisa  = IS_SSE3SLOW; // Atom SSSE3 slow
    if( c[2] & (1 <<  9)) {         _cpuisa  = IS_SSSE3;
    if( c[2] & (1 << 19)) {         _cpuisa  = IS_SSE41;
    if( c[2] & (1 << 23)) {         _cpuisa  = IS_SSE41x; // +popcount
    if( c[2] & (1 << 20)) {         _cpuisa  = IS_SSE42;  // SSE4.2
    if((c[2] & (1 << 28)) &&
       (c[2] & (1 << 27)) &&                           // OSXSAVE
       (c[2] & (1 << 26)) &&                           // XSAVE
       (xgetbv(0) & 6)==6) {        _cpuisa  = IS_AVX; // AVX
      if(c[2]& (1 <<  3))           _cpuisa |= 1;      // +FMA3
      if(c[2]& (1 << 16))           _cpuisa |= 2;      // +FMA4
      if(c[2]& (1 << 25))           _cpuisa |= 4;      // +AES
      cpuid(c, 7);
      if(c[1] & (1 << 5)) {         _cpuisa = IS_AVX2;
        if(c[1] & (1 << 16)) {
          cpuid(c, 0xd);
          if((c[0] & 0x60)==0x60) { _cpuisa = IS_AVX512;
            cpuid(c, 7);
            if(c[1] & (1<<16))      _cpuisa |= AVX512F;
            if(c[1] & (1<<17))      _cpuisa |= AVX512DQ;
            if(c[1] & (1<<21))      _cpuisa |= AVX512IFMA;
            if(c[1] & (1<<26))      _cpuisa |= AVX512PF;
            if(c[1] & (1<<27))      _cpuisa |= AVX512ER;
            if(c[1] & (1<<28))      _cpuisa |= AVX512CD;
            if(c[1] & (1<<30))      _cpuisa |= AVX512BW;
            if(c[1] & (1u<<31))     _cpuisa |= AVX512VL;
            if(c[2] & (1<< 1))      _cpuisa |= AVX512VBMI;
            if(c[2] & (1<<11))      _cpuisa |= AVX512VNNI;
            if(c[2] & (1<< 6))      _cpuisa |= AVX512VBMI2;
      }}}
    }}}}}}}}}
    #elif defined(__powerpc64__)

  _cpuisa = IS_POWER9; // power9
    #elif defined(__ARM_NEON)
  _cpuisa = IS_NEON; // ARM_NEON
    #endif
  return _cpuisa;
}

unsigned cpuini(unsigned cpuisa) { if(cpuisa) _cpuisa = cpuisa; return _cpuisa; }

char *cpustr(unsigned cpuisa) {
  if(!cpuisa) cpuisa = _cpuisa;
    #if defined(__i386__) || defined(__x86_64__)
  if(cpuisa >= IS_AVX512) {
    if(cpuisa & AVX512VBMI2) return "avx512vbmi2";
    if(cpuisa & AVX512VBMI)  return "avx512vbmi";
    if(cpuisa & AVX512VNNI)  return "avx512vnni";
    if(cpuisa & AVX512VL)    return "avx512vl";
    if(cpuisa & AVX512BW)    return "avx512bw";
    if(cpuisa & AVX512CD)    return "avx512cd";
    if(cpuisa & AVX512ER)    return "avx512er";
    if(cpuisa & AVX512PF)    return "avx512pf";
    if(cpuisa & AVX512IFMA)  return "avx512ifma";
    if(cpuisa & AVX512DQ)    return "avx512dq";
    if(cpuisa & AVX512F)     return "avx512f";
    return "avx512";
  }
  else if(cpuisa >= IS_AVX2)    return "avx2";
  else if(cpuisa >= IS_AVX)
    switch(cpuisa&0xf) {
      case 1: return "avx+fma3";
      case 2: return "avx+fma4";
      case 4: return "avx+aes";
      case 5: return "avx+fma3+aes";
      default:return "avx";
    }
  else if(cpuisa >= IS_SSE42)   return "sse4.2";
  else if(cpuisa >= IS_SSE41x)  return "sse4.1+popcnt";
  else if(cpuisa >= IS_SSE41)   return "sse4.1";
  else if(cpuisa >= IS_SSSE3)   return "ssse3";
  else if(cpuisa >= IS_SSE3)    return "sse3";
  else if(cpuisa >= IS_SSE2)    return "sse2";
  else if(cpuisa >= IS_SSE)     return "sse";
     #elif defined(__powerpc64__)
  if(cpuisa >= IS_POWER9)       return "power9";
    #elif defined(__ARM_NEON)
  if(cpuisa >= IS_NEON)         return "arm_neon";
    #endif
  return "none";
}
 
fanscdfenc _anscdfenc  = anscdfencs; //set sse2
fanscdfdec _anscdfdec  = anscdfdecs;
fanscdfenc _anscdf4enc = anscdf4encs;
fanscdfdec _anscdf4dec = anscdf4decs;
static int anscdfset;

void anscdfini(unsigned id) {  
  if(anscdfset && !id) return;
  div32init();
  anscdfset = 1;     
  id = id?id:cpuisa();          //printf("cpu=id=%d,%s\n", id, cpustr(id) );   
    #ifndef NAVX2
  if(id >= 0x60) { _anscdfenc  = anscdfencx;  _anscdfdec  = anscdfdecx; 
                   _anscdf4enc = anscdf4encx; _anscdf4dec = anscdf4decx;
  }  else 
    #endif
    #ifndef NSSE
  if(id >= 0x20) { 
    _anscdfenc  = anscdfencs;  _anscdfdec  = anscdfdecs; 
    _anscdf4enc = anscdf4encs; _anscdf4dec = anscdf4decs;
  } else 
    #endif
  { 
    _anscdfenc  = anscdfencs;  _anscdfdec  = anscdfdecs;   //TODO:replace to scalar for non SIMD systems
    _anscdf4enc = anscdf4encs; _anscdf4dec = anscdf4decs; 
  }
}

LIBAPI unsigned anscdfenc( unsigned char *in, unsigned inlen,  unsigned char *out, unsigned outsize, unsigned blksize) { anscdfini(0); return _anscdfenc( in, inlen,  out, outsize, blksize); };
LIBAPI unsigned anscdfdec( unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize) {                   anscdfini(0); return _anscdfdec( in, outlen, out,          blksize); };
LIBAPI unsigned anscdf4enc(unsigned char *in, unsigned inlen,  unsigned char *out, unsigned outsize, unsigned blksize) { anscdfini(0); return _anscdf4enc(in, inlen,  out, outsize, blksize); };
LIBAPI unsigned anscdf4dec(unsigned char *in, unsigned outlen, unsigned char *out, unsigned blksize) {                   anscdfini(0); return _anscdf4dec(in, outlen, out,          blksize); };
  #endif    
