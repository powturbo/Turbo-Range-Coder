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
// TurboRC: Range Coder Benchmark and Compressor Application
#include <stdio.h>
#include <string.h>
  #if !defined(_WIN32) && !defined(_WIN64)
#include <sys/resource.h>
  #endif
#define __STDC_WANT_IEC_60559_TYPES_EXT__
#include <float.h>

#include "include_/rcutil.h"
#include "include_/conf.h"

  #ifdef _MSC_VER
#include "vs/getopt.h"
  #else
#include <getopt.h> 
  #endif
#include "include/turborc.h"
 
#include "include_/conf.h"
#include "include_/time_.h"
#include "include_/bec.h"
#include "rcutil_.h"
  #ifdef _TRANSPOSE
#include "include_/transpose.h"
  #endif
#ifdef _TURBORLE
#include "include_/trle.h"
#endif
  #ifdef _ANS
#include "include/anscdf.h"
  #endif
  
  #ifdef _BWTDIV      
#include "libdivsufsort/include/divsufsort.h"
#include "libdivsufsort/unbwt.h"
  #else
#include "libsais/include/libsais.h"
  #endif

  #ifdef _EXT
#include "xturborc.h"
  #endif

#ifdef _BWTSATAN  
#define NO_BENCH
#define NO_RC
#endif

unsigned bwtx, forcelzp, xprep8, xsort, nutf8;
int BGFREQMIN = 50, BGMAX = 250, itmax;
#define bwtflag(z) (z==2?BWT_BWT16:0) | (xprep8?BWT_PREP8:0) | forcelzp | (nutf8?BWT_NUTF8:0) | (verbose?BWT_VERBOSE:0) | xsort <<14 | itmax <<10 | lenmin

#define MAGIC     0x154 // 12 bits
#define BLKMAX    3584
#define BLKBWTMAX 2047

#define CODEC_MAX 32  // max. id for file compression
int verbose;
enum { E_FOP=1, E_FCR, E_FRD, E_FWR, E_MEM, E_CORR, E_MAG, E_CODEC, E_FSAME };

static char *errs[] = {"", "open error", "create error", "read error", "write error", "malloc failed", "file corrupted", "no TurboRc file", "no codec", "input and output files are same" };
 
// program parameters
static unsigned xnibble, lenmin = 1, lev=8, thnum=0, xtpbyte=-1;
unsigned prm1=5, prm2=6; 

//       0       1        2         3         4         5         6         7,       8        9        10      11      12      13      14       15
enum { T_0, T_UINT8, T_UINT16, T_UINT24, T_UINT32, T_UINT40, T_UINT48, T_UINT56, T_UINT64, T_FLOAT, T_DOUBLE, T_CHAR, T_TXT, T_TIM32, T_TIM64, T_RAW, T_TST };

#ifndef NO_BENCH
void mbcset(unsigned m);
//----------------------------- Convert iso-8601 and similar formats to timestamp -------------------------
// Date separator : '.'. '/' or '-'
// Hour separator : ':'
// Fraction sep.: '.' or ','
// examples: "2020" "20211203" "20211022 11:09:45.1234",
uint64_t strtots(char *p, char **pq, int type) {  // string to timestamp
  struct   tm tm;
  uint64_t u;
  char     *s = p;
  int      frac = 0, c;

  memset(&tm, 0, sizeof(tm)); tm.tm_mday = 1;
  while(!isdigit(*p)) p++;
  u = strtoull(p, &p, 10);                  // first number

  if(     u <= 99) u += 2000;               // year  "yy": 00-99 -> 2000-2099
  else if(u >= 19710101 && u < 20381212) {  // date: "yyyymmdd"
    tm.tm_year =  u/10000;
    tm.tm_mon  = (u%10000)/100;   if(!tm.tm_mon  || tm.tm_mon  > 12) goto a; tm.tm_mon--;
    tm.tm_mday = u%10;            if(!tm.tm_mday || tm.tm_mday > 31) goto a;
    goto h;
  } else if(u < 1971 || u > 2099) goto a;   // invalid
  tm.tm_year = u;                           // year       "yyyy"

  c = *p;                                   // month,day: "mm.dd", "mm-dd", "mm/dd"
  if(c != '.' && c != '-' && c != '/') goto b;    tm.tm_mon    = strtoul(p+1, &p, 10);   if(!tm.tm_mon  || tm.tm_mon  > 12) goto a; tm.tm_mon--;
  if(c != '.' && c != '-' && c != '/') goto b;    tm.tm_mday   = strtoul(p+1, &p, 10);   if(!tm.tm_mday || tm.tm_mday > 31) goto a;
  if(c != '.' && c != '-' && c != '/') goto b;    h:tm.tm_hour = strtoul(p+1, &p, 10);

  if(tm.tm_hour <= 24 && *p == ':') {       // time ":hh:mm:ss.frac", ":hh:mm:ss,frac"
    tm.tm_min = strtoul(p+1, &p, 10);   if(tm.tm_min > 60) tm.tm_hour = tm.tm_min = 0;
    tm.tm_sec = strtoul(p+1, &p, 10);   if(tm.tm_sec > 60) tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    if(type > 0 && (*p == '.' || *p == ',' || *p == ':')) {
	  frac = strtoul(p+1, &p, 10);
	  if((c = p-(p+1)) > 6) frac /= 1000000;
	  else if(c > 3) frac /= 1000;
	}
  } else tm.tm_hour = 0;
  
  b:tm.tm_year -= 1900;
  u = mktime(&tm);
  u = u * 1000 + frac;                      // milliseconds
  a:*pq = p;                                if(verbose >= 9) printf("[%d-%d-%d %.2d:%.2d:%.2d.%d->%llx]\n", tm.tm_year, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, frac, u);
  return u;
}

#define EPUSH(_op_, _out__, _osize_, _u_) {\
  switch(abs(_osize_)) {\
    case 1:       *_op_++ = _u_;            if(_u_ >       0xffu) ovf++;break;\
    case 2: ctou16(_op_)  = _u_; _op_ += 2; if(_u_ >     0xffffu) ovf++;break;\
    case 4: ctou32(_op_)  = _u_; _op_ += 4; if(_u_ > 0xffffffffu) ovf++;break;\
    case 8: ctou64(_op_)  = _u_; _op_ += 8; break;\
  } if(_op_+_osize_ > _out__) goto end;\
}

size_t befgen(FILE *fi, unsigned char *out, size_t outsize, int fmt, int isize, int osize, int kid, int skiph, int decs, int divs, char *keysep, int mdelta) {
  unsigned char *op = out, *out_ = out+outsize;                             
  unsigned ovf = 0;
  #define LSIZE (1024*16)
  char s[LSIZE+1];
  double pre;

  while(skiph-- > 0) {
    fgets(s, LSIZE, fi);                                                        if(verbose>=5 && (op-out)/osize < 100 ||verbose>5) printf("skip first line\n");
  }
  if(decs) {
    pre = decs?pow(10.0f,(float)decs):1;
    pre /= divs;
  } else pre = 1;
  
  
  switch(fmt) {
    case T_TXT:
    case T_TIM32:
    case T_TIM64:                                                               if(verbose>2) printf("reading text lines. pre=%.2f, col=%d, sep='%s'\n", pre, kid, keysep?keysep:"");
      while(fgets(s, LSIZE, fi)) {                                          
        unsigned char *p = s,*q;
        int k = 0, keyid = 1, c;
        s[strlen(s) - 1] = 0;
        q = p;
        if(kid > 1)
          do {
            p = q;
            if(keysep && strchr(keysep,*q)) keyid++;
            q++;
          } while(*q && keyid != kid);
        if(fmt == T_TIM32 || fmt == T_TIM64) {
          while(!isdigit(*p)) p++;
          uint64_t u = strtots(p, &q, fmt == T_TIM64?1:0);
          if(fmt == T_TIM32) u /= 1000;
          a: EPUSH(op,out_,osize,u);                                            c=*q; *q=0; if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) printf("\'%s\'->%llu  ", p, u); *q = c;
        } else if(osize > 0) {
          while(!isdigit(*p) && *p != '-' && *p != '+') p++;
          uint64_t u = strtoll(p, &q, 10)*pre - mdelta;
          if(*q == '.')
            u = pre>1.0?round(strtod(p, &q)*pre):strtod(p, &q) - mdelta;
          EPUSH(op,out_,osize,u);                                               c=*q;   *q=0; if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) printf("\'%s\'->%lld ", p, u); *q = c;
        } else {
          while(*p && !isdigit(*p) && *p != '-' && *p != '.' && *p != '+') {  
                    if(keysep && strchr(keysep,*p)) keyid++; p++; 
                  }
          double d = strtod(p, &q) - mdelta;
          uint64_t u;
          memcpy(&u,&d,sizeof(u));
          EPUSH(op,out_,osize,u);                                               if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) { c=*q; *q=0; double d; memcpy(&d,&u,sizeof(d)); printf("\'%s\'->%f  ", p, d); *q = c; }
        }
      }
      break;
    case T_CHAR:                                                                if(verbose>2) printf("reading char file. pre=%.2f\n", pre);
      for(;;) {
        char *p = s,*q;
        int c;
        if(osize > 0) {
          int64_t u;
          while((c = getc(fi)) >= '0' && c <= '9' || c == '-' || c == '+')
            if(p - s < LSIZE) *p++ = c;
                  if(c == EOF) break;
          if(c == '.')  {
            *p++ = c;
            while((c = getc(fi)) >= '0' && c <= '9' || c == '-' || c == '+' || c == 'e' || c == 'E')
              if(p - s < LSIZE) *p++ = c;
            *p = 0;
            u = pre>1.0?round(strtod(s, &q)*pre):strtod(s, &q) - mdelta;
          } else {
            *p = 0;
                        unsigned char *q = s; 
                        while((*q < '0' || *q > '9') && *q != '-' && *q == '+' && *q != 'e' && *q != 'E' && *q != '.') q++;
                        if(q == p) continue;
            u = strtoll(q, &p, 10) - mdelta;
          }
          EPUSH(op,out_,osize,u);                                               if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) printf("'%s'->%lld ", s, (int64_t)u);
        } else {
          while((c = getc(fi)) >= '0' && c <= '9' || c == '-')
            if(p - s < LSIZE) *p++ = c;
          if((*p++ = c) == '.')
            while((c = getc(fi)) >= '0' && c <= '9')
              if(p - s < LSIZE) *p++ = c;
          *p = 0;
          double d = strtod(s, &p) - mdelta;
          uint64_t u;
          memcpy(&u,&d,sizeof(u));                                              if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) { double d; memcpy(&d,&u,sizeof(u)); printf("\'%s\'->%e  ", s, d); }
          EPUSH(op,out_,osize,u);
        }
        if(c == EOF) break;
      }
      break;
    default: { unsigned char *ip = s;
      for(;;) {
        if(fread(s, 1, abs(isize), fi) != abs(isize)) goto end; 
        switch(abs(isize)) {
          case 1: 
            switch(abs(osize)) {
              case 1: *op++      = *ip;        break;
              case 2: ctou16(op) = *ip; op+=2; break;
              case 4: ctou32(op) = *ip; op+=4; break;
              case 8: ctou64(op) = *ip; op+=8; break;
            } break;
          case 2: 
           switch(abs(osize)) {
             case 1: *op++      = ctou16(ip);        if(ctou16(ip) >      0xffu) ovf++; break;
             case 2: ctou16(op) = ctou16(ip); op+=2; break;
             case 4: ctou32(op) = ctou16(ip); op+=4; break;
             case 8: ctou64(op) = ctou16(ip); op+=8; break;
           } break;
         case 4: 
           switch(abs(osize)) {
             case 1: *op++      = ctou32(ip);        if(ctou32(ip) >       0xffu) ovf++; break;
             case 2: ctou16(op) = ctou32(ip); op+=2; if(ctou32(ip) >     0xffffu) ovf++; break;
             case 4: ctou32(op) = ctou32(ip); op+=4; break;
             case 8: ctou64(op) = ctou32(ip); op+=8; break;
           } break;
         case 8: 
           switch(abs(osize)) {
             case 1: *op++      = ctou64(ip);        if(ctou64(ip) >       0xffu) ovf++; break;
             case 2: ctou16(op) = ctou64(ip); op+=2; if(ctou64(ip) >     0xffffu) ovf++;break;
             case 4: ctou32(op) = ctou64(ip); op+=4; if(ctou64(ip) > 0xffffffffu) ovf++;break;
             case 8: ctou64(op) = ctou64(ip); op+=8; break;
           } break;
        }
      }
    }
  }
  end:;if(verbose >= 5) printf(" n=%lld \n", op-out);
  if(ovf) { unsigned l = (op-out)/abs(osize); 
    printf("Number of items truncated=%u of %u = %.2f%%\n", ovf, l, (double)ovf*100.0/(double)l ); 
  }
  return op - out;
}

int memcheck(unsigned char *in, unsigned n, unsigned char *cpy) { 
  int i;
  for(i = 0; i < n; i++)
    if(in[i] != cpy[i]) { 
      printf("ERROR in[%d]=%x dec[%d]=%x\n", i, in[i], i, cpy[i]);
      return i+1; 
    }
  return 0;
} 

//************************ TurboRC functions *******************************************
// functions parameters
extern int fsm[]; // fsm global array declared as fsm_t in rc_s.c "

  #ifdef _SF
#define SF(x) x
  #else
#define SF(x) 
  #endif

  #ifdef _NZ
#define NZ(x) x
  #else
#define NZ(x) 
  #endif

//  Generate functions with all predictors
#define RCGEN(_p_) \
size_t _p_##enc(unsigned char *in, size_t inlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
    case RC_PRD_S :    return _p_##senc( in, inlen, out);\
    case RC_PRD_SS:    return _p_##ssenc(in, inlen, out, prm1,prm2);\
    SF(case RC_PRD_SF: return _p_##sfenc(in, inlen, out, fsm));\
    NZ(case RC_PRD_NZ: return _p_##nzenc(in, inlen, out, fsm));\
  }\
  return 0; \
}\
\
size_t _p_##dec(unsigned char *in, size_t outlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
    case RC_PRD_S :    return _p_##sdec( in, outlen, out);\
    case RC_PRD_SS:    return _p_##ssdec(in, outlen, out, prm1,prm2);\
    SF(case RC_PRD_SF: return _p_##sfdec(in, outlen, out, fsm));\
    NZ(case RC_PRD_NZ: return _p_##nzdec(in, outlen, out, fsm));\
  }\
  return 0; \
}

// Generate integer functions with all predictors
#define RCGEN2(_p_, _s_) \
size_t _p_##enc##_s_(unsigned char *in, size_t inlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
    case RC_PRD_S :    return _p_##senc##_s_( in, inlen, out);\
    case RC_PRD_SS:    return _p_##ssenc##_s_(in, inlen, out, prm1,prm2);\
    SF(case RC_PRD_SF: return _p_##sfenc##_s_(in, inlen, out, fsm));\
    NZ(case RC_PRD_NZ: return _p_##nzenc##_s_(in, inlen, out, fsm));\
  }\
  return 0; \
}\
\
size_t _p_##dec##_s_(unsigned char *in, size_t outlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
    case RC_PRD_S :    return _p_##sdec##_s_( in, outlen, out);\
    case RC_PRD_SS:    return _p_##ssdec##_s_(in, outlen, out, prm1,prm2);\
    SF(case RC_PRD_SF: return _p_##sfdec##_s_(in, outlen, out, fsm));\
    NZ(case RC_PRD_NZ: return _p_##nzdec##_s_(in, outlen, out, fsm));\
  }\
  return 0; \
}

RCGEN(rc)            // order0,1,2
RCGEN2(rc,    16)
RCGEN2(rc,    32)
RCGEN(rcc)
RCGEN2(rcc,   16)
RCGEN2(rcc,   32)
RCGEN(rcc2)
RCGEN2(rcc2,  32)
RCGEN(rcx)
RCGEN(rcx2)

RCGEN(rcm)           // context mixing
RCGEN(rcm2)
RCGEN(rcmr)  
RCGEN(rcmrr)  

RCGEN(rcrle)        // rle
RCGEN2(rcrle, 16)
RCGEN(rcrle1)
RCGEN2(rcrle1,16)
RCGEN(rcqlfc)       // qlfc
RCGEN(rcu3)         // varint8

RCGEN2(rcg,   8)    // gamma
RCGEN2(rcg,  16)
RCGEN2(rcg,  32)
RCGEN2(rcgz,  8)
RCGEN2(rcgz, 16)
RCGEN2(rcgz, 32)

RCGEN2(rcr,   8)    // rice
RCGEN2(rcr,  16)
RCGEN2(rcr,  32)
RCGEN2(rcrz,  8)
RCGEN2(rcrz, 16)
RCGEN2(rcrz, 32)

RCGEN2(rcv,  16)    // Turbo vlc
RCGEN2(rcv,  32)
RCGEN2(rcvz, 16)
RCGEN2(rcvz, 32)
RCGEN2(rcvg, 16)
RCGEN2(rcvg, 32)
RCGEN2(rcvgz,16)
RCGEN2(rcvgz,32)
RCGEN2(rcve, 32)
RCGEN2(rcvez,32)

  #ifdef _V8
RCGEN2(rcv8, 16)
RCGEN2(rcv8, 32)
RCGEN2(rcv8z,16)
RCGEN2(rcv8z,32)
  #endif

//FILE *fdbg;
int xcheck;
#define FP_ZERO DBL_EPSILON
double   gmin = FP_ZERO, gmax = FP_ZERO, zerrlim;
unsigned quantb;

#define OSIZE(_n_) ((_n_)*4/3)
#define ID_RC16 8
unsigned bench(unsigned char *in, unsigned n, unsigned char *out, unsigned char *cpy, int id, int r, int z) { //rcxor(in, n); return n;
  unsigned i, m, flag = bwtflag(z), on = OSIZE(n);  
  size_t l = 0;
  cdf_t cdf[0x100+1]; 
  if(xnibble)                                                                                                       // use only low nibble
    for(int i = 0; i < n; i++) in[i] &= 0xf;
    #ifndef _MSC_VER
  memrcpy(cpy,in,n); 
    #endif
  if(id >= 40 && id <= 65) {                           // enable low nibble functions, if input values are <=0xf
    for(m = i = 0; i < n; i++) 
      if(in[i] > m) m = in[i];                              
    cdfini(in, n, cdf, 0x100);                                                                  // calculte freq. for static distribution functions     
  }
  #define CCPY l==n?(size_t)memcpy(cpy,out,n)
  switch(id) {  
    case  1:         TM(" 1:rc        o0                         ",l=rcenc(      in,n,out,r), n,l, CCPY:rcdec(      out,n,cpy,r));   break;
    case  2:         TM(" 2:rcc       o1                         ",l=rccenc(     in,n,out,r), n,l, CCPY:rccdec(     out,n,cpy,r));   break;
    case  3:         TM(" 3:rcc2      o2                         ",l=rcc2enc(    in,n,out,r), n,l, CCPY:rcc2dec(    out,n,cpy,r));   break;
    case  4:         TM(" 4:rcx       o8b =o1 context slide      ",l=rcxenc(     in,n,out,r), n,l, CCPY:rcxdec(     out,n,cpy,r));   break;
    case  5:         TM(" 5:rcx2      o16b=o2 context slide      ",l=rcx2enc(    in,n,out,r), n,l, CCPY:rcx2dec(    out,n,cpy,r));   break;
    case  6:if(z==2){TM(" 6:rc-16     o0  16-bits                ",l=rcenc16(    in,n,out,r), n,l, CCPY:rcdec16(    out,n,cpy,r));   break;}
            if(z==4){TM(" 6:rc-32     o0  32-bits                ",l=rcenc32(    in,n,out,r), n,l, CCPY:rcdec32(    out,n,cpy,r)); } break;
    case  7:if(z==2){TM(" 7:rcc-16    o1  16-bits                ",l=rccenc16(   in,n,out,r), n,l, CCPY:rccdec16(   out,n,cpy,r));   break;}
            if(z==4){TM(" 7:rcc-32    o7bs  32-bits o[24-30]     ",l=rccenc32(   in,n,out,r), n,l, CCPY:rccdec32(   out,n,cpy,r)); } break;
    case  8:if(z==4){TM(" 8:rcc2-32   o11bs 32-bits o[20-30]     ",l=rcc2enc32(  in,n,out,r), n,l, CCPY:rcc2dec32(  out,n,cpy,r)); } break;
    case  9:         TM(" 9:rcms      o1 mixer/sse               ",l=rcmenc(     in,n,out,r), n,l, CCPY:rcmdec(     out,n,cpy,r));   break;
    case 10:         TM("10:rcm2      o2 mixer/sse               ",l=rcm2enc(    in,n,out,r), n,l, CCPY:rcm2dec(    out,n,cpy,r));   break;
    case 11:         TM("11:rcmr      o2 8b mixer/sse run        ",l=rcmrenc(    in,n,out,r), n,l, CCPY:rcmrdec(    out,n,cpy,r));   break;
    case 12:         TM("12:rcmrr     o2 8b mixer/sse run > 2    ",l=rcmrrenc(   in,n,out,r), n,l, CCPY:rcmrrdec(   out,n,cpy,r));   break;
    case 13:if(z==2){TM("13:rcrle-16  RLE o0                     ",l=rcrleenc16( in,n,out,r), n,l, CCPY:rcrledec16( out,n,cpy,r)); }
            else    {TM("13:rcrle     RLE o0                     ",l=rcrleenc(   in,n,out,r), n,l, CCPY:rcrledec(   out,n,cpy,r)); } break;
    case 14:if(z==2){TM("14:rcrle1-16 RLE o1                     ",l=rcrle1enc16(in,n,out,r), n,l, CCPY:rcrle1dec16(out,n,cpy,r)); }
            else    {TM("14:rcrle1    RLE o1                     ",l=rcrle1enc(  in,n,out,r), n,l, CCPY:rcrle1dec(  out,n,cpy,r)); } break;
    case 17:         TM("17:rcu3      varint8 3/5/8 bits         ",l=rcu3enc(    in,n,out,r), n,l, CCPY:rcu3dec(    out,n,cpy,r));   break;
    case 18:         TM("18:rcqlfc    QLFC                       ",l=rcqlfcenc(  in,n,out,r), n,l, CCPY:rcqlfcdec(  out,n,cpy,r));   break;
    case 19:if(z==2){TM("19:bec-16    Bit EC                     ",l=becenc16(   in,n,out),   n,l, CCPY:becdec16(   out,n,cpy  )); }
            else    {TM("19:bec       Bit EC                     ",l=becenc8(    in,n,out),   n,l, CCPY:becdec8(    out,n,cpy  )); } break;
          #ifdef _BWT
    case 20:if(n > BLKBWTMAX*MB) printf("blocksize too big for bwt.max=%d\n", BLKBWTMAX);
                    else {   TM("20:bwt                                  ",l=rcbwtenc(in,n,out,lev,thnum,flag), n,l, l>=n?memcpy(cpy,out,n):rcbwtdec(  out,n,cpy,lev, thnum));} break;
      #endif
    case 26:if(z==1){TM("26:rcg-8     gamma                      ",l=rcgenc8(    in,n,out,r), n,l, CCPY:rcgdec8(    out,n,cpy,r));   break;}
            if(z==2){TM("26:rcg-16    gamma                      ",l=rcgenc16(   in,n,out,r), n,l, CCPY:rcgdec16(   out,n,cpy,r));   break;}
            if(z==4){TM("26:rcg-32    gamma                      ",l=rcgenc32(   in,n,out,r), n,l, CCPY:rcgdec32(   out,n,cpy,r)); } break;
    case 27:if(z==1){TM("27:rcgz-8    gamma zigzag               ",l=rcgzenc8(   in,n,out,r), n,l, CCPY:rcgzdec8(   out,n,cpy,r));   break;}
            if(z==2){TM("27:rcgz-16   gamma zigzag               ",l=rcgzenc16(  in,n,out,r), n,l, CCPY:rcgzdec16(  out,n,cpy,r));   break;}
            if(z==4){TM("27:rcgz-32   gamma zigzag               ",l=rcgzenc32(  in,n,out,r), n,l, CCPY:rcgzdec32(  out,n,cpy,r)); } break;
                        
    case 28:if(z==1){TM("28:rcr-8     rice                       ",l=rcrenc8(    in,n,out,r), n,l, CCPY:rcrdec8(    out,n,cpy,r));   break;}
            if(z==2){TM("28:rcr-16    rice                       ",l=rcrenc16(   in,n,out,r), n,l, CCPY:rcrdec16(   out,n,cpy,r));   break;}
            if(z==4){TM("28:rcr-32    rice                       ",l=rcrenc32(   in,n,out,r), n,l, CCPY:rcrdec32(   out,n,cpy,r)); } break;
    case 29:if(z==1){TM("29:rcrz-8    rice zigzag                ",l=rcrzenc8(   in,n,out,r), n,l, CCPY:rcrzdec8(   out,n,cpy,r));   break;}
            if(z==2){TM("29:rcr-16    rice zigzag                ",l=rcrzenc16(  in,n,out,r), n,l, CCPY:rcrzdec16(  out,n,cpy,r));   break;}
            if(z==4){TM("29:rcr-32    rice zigzag                ",l=rcrzenc32(  in,n,out,r), n,l, CCPY:rcrzdec32(  out,n,cpy,r));}  break;
                            
    case 30:if(z==2){TM("30:rcv-16    Turbo vlc8                 ",l=rcvenc16(   in,n,out,r), n,l, CCPY:rcvdec16(   out,n,cpy,r));   break;}
            if(z==4){TM("30:rcv-32    Turbo vlc8                 ",l=rcvenc32(   in,n,out,r), n,l, CCPY:rcvdec32(   out,n,cpy,r));}  break;
    case 31:if(z==4){TM("31:rcvc-32   Turbo vlc10                ",l=rcv10senc32(in,n,out ),  n,l, CCPY:rcv10sdec32(out,n,cpy)); }   break;
    case 32:if(z==4){TM("32:rcve-32   Turbo vlc12                ",l=rcveenc32(  in,n,out,r), n,l, CCPY:rcvedec32(  out,n,cpy,r)); } break;
    case 33:if(z==2){TM("33:rcvz-16   Turbo vlc8 zigzag          ",l=rcvzenc16(  in,n,out,r), n,l, CCPY:rcvzdec16(  out,n,cpy,r));   break;}
            if(z==4){TM("33:rcvz-32   Turbo vlc8 zigzag          ",l=rcvzenc32(  in,n,out,r), n,l, CCPY:rcvzdec32(  out,n,cpy,r)); } break;
    case 34:if(z==4){TM("34:rcvez-32  Turbo vlc12 zigzag         ",l=rcvezenc32( in,n,out,r), n,l, CCPY:rcvezdec32( out,n,cpy,r)); } break;
    case 35:if(z==2){TM("35:rcvg-16   Turbo vlc8 gamma           ",l=rcvgenc16(  in,n,out,r), n,l, CCPY:rcvgdec16(  out,n,cpy,r));   break;}
            if(z==4){TM("35:rcvg-32   Turbo vlc8 gamma           ",l=rcvgenc32(  in,n,out,r), n,l, CCPY:rcvgdec32(  out,n,cpy,r)); } break;
    case 36:if(z==2){TM("36:rcvgz-16  Turbo vlc8 gamma zigzag    ",l=rcvgzenc16( in,n,out,r), n,l, CCPY:rcvgzdec16( out,n,cpy,r));   break;}
            if(z==4){TM("36:rcvgz-32  Turbo vlc8 gamma zigzag    ",l=rcvgzenc32( in,n,out,r), n,l, CCPY:rcvgzdec32( out,n,cpy,r));}  break;
          #ifdef _V8        
    case 37:if(z==2){TM("37:rcv8-16   Turbobyte                  ",l=rcv8enc16(  in,n,out,r), n,l, CCPY:rcv8dec16(  out,n,cpy,r));   break;}
            if(z==4){TM("37:rcv8-32   TurboByte                  ",l=rcv8enc32(  in,n,out,r), n,l, CCPY:rcv8dec32(  out,n,cpy,r));}  break;
    case 38:if(z==2){TM("38:rcv8-16   Turbobyte zigzag           ",l=rcv8zenc16( in,n,out,r), n,l, CCPY:rcv8zdec16( out,n,cpy,r));   break;}
            if(z==4){TM("38:rcv8-32   TurboByte zigzag           ",l=rcv8zenc32( in,n,out,r), n,l, CCPY:rcv8zdec32( out,n,cpy,r));}  break;
          #endif                
    case 40:if(m<16){TM("40:rc4cs     bitwise nibble static      ",l=rc4csenc(   in,n,out),   n,l, CCPY:rc4csdec(  out,n,cpy)); } break;            // Static
    case 41:if(m<16){TM("41:rc4s      bitwise nibble adaptive    ",l=rc4senc(    in,n,out),   n,l, CCPY:rc4sdec(   out,n,cpy)); } break;           // Adaptive
    case 42:         TM("42:cdfsb     static/decode search       ",l=rccdfsenc( in,n,out,cdf,m+1),n,l, CCPY:(m<16?rccdfsldec( out,n,cpy, cdf, m+1):rccdfsbdec( out,n,cpy, cdf, m+1))); break; // static
    case 43:         TM("43:cdfsv     static/decode division     ",l=rccdfsenc( in,n,out,cdf,m+1),n,l, CCPY:(m<16?rccdfsvldec(out,n,cpy, cdf, m+1):rccdfsvbdec(out,n,cpy, cdf, m+1))); break;
    case 44:         TM("44:cdfsm     static/decode division lut ",l=rccdfsmenc(in,n,out,cdf,m+1),n,l, CCPY:(m<16?rccdfsmldec(out,n,cpy, cdf, m+1):rccdfsmbdec(out,n,cpy, cdf, m+1))); break;
    case 45:         TM("45:cdfsb     static interlv/dec. search ",l=rccdfs2enc(in,n,out,cdf,m+1),n,l, CCPY:(m<16?rccdfsl2dec(out,n,cpy, cdf, m+1):rccdfsb2dec(out,n,cpy, cdf, m+1))); break; // static
    case 46:if(m<16){TM("46:cdf4      nibble adaptive            ",l=rccdf4enc(  in,n,out),   n,l, CCPY:rccdf4dec( out,n,cpy)); }
            else {   TM("46:cdf       byte   adaptive            ",l=rccdfenc(   in,n,out),   n,l, CCPY:rccdfdec(  out,n,cpy)); } break;
    case 47:if(m<16){TM("47:cdf4i     nibble adaptive interleaved",l=rccdf4ienc( in,n,out),   n,l, CCPY:rccdf4idec(out,n,cpy)); }
            else {   TM("47:cdfi      byte   adaptive interleaved",l=rccdfienc(  in,n,out),   n,l, CCPY:rccdfidec( out,n,cpy)); } break;
    case 48:         TM("48:cdf-8     vnibble                    ",l=rccdfenc8(  in,n,out),   n,l, CCPY:rccdfdec8( out,n,cpy)); break;
    case 49:         TM("49:cdfi-8    vnibble interleaved        ",l=rccdfienc8( in,n,out),   n,l, CCPY:rccdfidec8(out,n,cpy)); break; 
    case 50:if(z==2){TM("50:cdf-16    Turbo vlc6                 ",l=rccdfuenc16(in,n,out),   n,l, CCPY:rccdfudec16(out,n,cpy)); break; }
            if(z==4){TM("50:cdf-32    Turbo vlc6                 ",l=rccdfuenc32(in,n,out),   n,l, CCPY:rccdfudec32(out,n,cpy));} break;
    case 52:if(z==2){TM("52:cdf-16    Turbo vlc7                 ",l=rccdfvenc16(in,n,out),   n,l, CCPY:rccdfvdec16(out,n,cpy)); break;}
            if(z==4){TM("52:cdf-32    Turbo vlc7                 ",l=rccdfvenc32(in,n,out),   n,l, CCPY:rccdfvdec32(out,n,cpy));} break;
    case 53:if(z==2){TM("53:cdf-16    Turbo vlc7 zigzag          ",l=rccdfvzenc16(in,n,out),  n,l, CCPY:rccdfvzdec16(out,n,cpy)); break;}
            if(z==4){TM("53:cdf-32    Turbo vlc7 zigzag          ",l=rccdfvzenc32(in,n,out),  n,l, CCPY:rccdfvzdec32(out,n,cpy));} break;
      #ifdef _ANS           
  //case 54:if(m<16){TM("54:ans scalar nibble                    ",l=anscdf4enc0( in,n,out), n,l, anscdf4dec0( out,n,cpy));}
  //        else    {TM("54:ans scalar                           ",l=anscdfenc0(  in,n,out), n,l, anscdfdec0(  out,n,cpy));} break;
    case 56:if(m<16){TM("56:ans auto   nibble                    ",l=anscdf4enc(  in,n,out), n,l, CCPY:anscdf4dec(  out,n,cpy));}
            else    {TM("56:ans auto                             ",l=anscdfenc(   in,n,out), n,l, CCPY:anscdfdec(   out,n,cpy));} break;
    case 57:if(m<16){TM("57:ans sse nibble                       ",l=anscdf4encs( in,n,out), n,l, CCPY:anscdf4decs( out,n,cpy));}
            else    {TM("57:ans sse                              ",l=anscdfencs(  in,n,out), n,l, CCPY:anscdfdecs(  out,n,cpy));} break;
        #ifndef _NAVX2             
    case 58:if(cpuisa()>=0x60) {
              if(m<16){TM("58:ans avx2 nibble                      ",l=anscdf4encx( in,n,out), n,l, CCPY:anscdf4decx( out,n,cpy));}
              else    {TM("58:ans avx2                             ",l=anscdfencx(  in,n,out), n,l, CCPY:anscdfdecx(  out,n,cpy));} 
            }  break;
        #endif      
    //case 59:if(m<16){TM("57:ansx sse nibble                      ",l=anscdf4encs( in,n,out), n,l, CCPY:anscdf4decs( out,n,cpy));}
    //        else    {TM("57:ansx sse                             ",l=anscdfxencx(  in,n,out), n,l, CCPY:anscdfxdecx(  out,n,cpy));} break;
    case 60:if(z==2){TM("60:anscdf-16 Turbo vlc6                 ",l=anscdfuenc16(in,n,out),   n,l, CCPY:anscdfudec16( out,n,cpy));  } break;
          //if(z==4){TM("60:anscdf-32 Turbo vlc6                 ",l=rccdfuenc32(in,n,out),   n,l, CCPY:rccdfudec32(out,n,cpy));} break;
    case 61:if(z==2){TM("61:anscdf-16 Turbo vlc6 zigzag          ",l=anscdfuzenc16(in,n,out),  n,l, CCPY:anscdfuzdec16(out,n,cpy)); } break; 
          //if(z==4){TM("61:cdf-32    Turbo vlc6                 ",l=rccdfuenc32(in,n,out),   n,l, CCPY:rccdfudec32(out,n,cpy));} break;
    case 62:if(z==2){TM("62:anscdf-16 Turbo vlc7                 ",l=anscdfvenc16( in,n,out),  n,l, CCPY:anscdfvdec16( out,n,cpy)); break;}
            if(z==4){TM("62:anscdf-32 Turbo vlc7                 ",l=anscdfvenc32( in,n,out),  n,l, CCPY:anscdfvdec32( out,n,cpy)); } break;
    case 63:if(z==2){TM("63:anscdf-16 Turbo vlc7 zigzag          ",l=anscdfvzenc16(in,n,out),  n,l, CCPY:anscdfvzdec16(out,n,cpy)); break;}
            if(z==4){TM("63:anscdf-32 Turbo vlc7 zigzag          ",l=anscdfvzenc32(in,n,out),  n,l, CCPY:anscdfvzdec32(out,n,cpy));} break;
    case 64:         TM("64:ans auto  o1                         ",l=anscdf1enc(   in,n,out),  n,l, CCPY:anscdf1dec(   out,n,cpy)); break;
    case 65:if(m<16){TM("65:anscdf4s  nibble static              ",l=anscdf4senc(in,n,out,cdf),n,l, CCPY:anscdf4sdec(  out,n,cpy,cdf)); } break; // static
    case 66:         TM("66:ansb      bitwise ans                ",l=ansbc(        in,n,out),  n,l, CCPY:ansbd(        out,n,cpy)); break;

      #endif    
    #define ID_LAST   79
    #define ID_MEMCPY 79 
    case ID_MEMCPY:  TM("79:memcpy                               ", memcpy(out,in,n),         n,n, memcpy(cpy,out,n)); l = n; break;
      #ifdef _BWT
    case 80:{ unsigned *sa = malloc((n+1)*sizeof(sa[0]));if(!sa) die("malloc of '' failed\n", n*4);
        #ifdef _BWTDIV
                     TM("80:bwt libdivsufsort                    ",l=divbwt(in,out,sa,n),     n,n, obwt_unbwt_biPSIv2(out,cpy,sa,n,l)); free(sa); } break; //ctou32(out)=l; fwrite(out,1,n+4,fdbg); 
        #else
                     TM("80:bwt libsais                          ",l=libsais_bwt(in,out,sa,n,0,0), n,n, libsais_unbwt(out,cpy,sa,n,0,l)); free(sa);} break;
        #endif
      #endif
    case 81:         TM("81:utf8 preprocessor                    ",l=utf8enc(in,n,out, flag|BWT_COPY|BWT_RATIO),n,l,CCPY:utf8dec(out,n,cpy)); break;
    case 82:         TM("82:lzp                                  ",l=lzpenc( in,n,out,lenmin,0),                n,l,CCPY:lzpdec( out,n,cpy,lenmin,0)); break;
    case 83:         TM("83:bitenc                               ",l=bitenc( in,n,out),                         n,l,     bitdec(out,n,cpy)); break;
      #ifndef _NDELTA
    case 84:l=n;     TM("84:delta8e24                            ",delta8e24( in,n,out),                        n,l,     delta8d24( out,n,cpy)); break;
  //case 65:l=n;     TM("85:delta24e24                           ",delta24e24(in,n,out),                        n,l,     delta24d24(out,n,cpy)); break;
      #endif   
	  #ifdef _TRANSPOSE
    case 85:l=n;     TM("85:tpenc 24 bits                        ",tpenc(in,n,out,3),                           n,l,     tpdec( out,n,cpy, 3)); break;
	  #endif
	  #ifndef _NQUANT
		#if defined(FLT16_BUILTIN)
    case 86: { _Float16 fmin = -1.16, fmax = 1.4; if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;
	  if(!quantb || quantb > 8) quantb = 8;                                      
	  size_t clen = fpquant8e16(in,n,out, BZMASK32(quantb), &fmin, &fmax, FLT16_EPSILON);  if(verbose>2) printf("\nlen:%u R:[%g/%g]=%g q=%u,%u ", clen, (double)fmin, (double)fmax, (double)fmax-(double)fmin, quantb, BZMASK32(quantb));	
	  fpquant8d16(out, n, cpy, BZMASK32(quantb), fmin, fmax, clen);         
      fpstat(in, n/2, cpy, -2, NULL);
	} break;
    case 87: if(zerrlim>DBL_EPSILON) { l=n; TM0("", fprazor16(in, n/2, out,zerrlim), n, l);                                          memcpy(cpy,in,n); if(verbose>1) fpstat(in, n/2, out, -2, NULL); } break;
		#endif
      #endif

      #ifdef _EXT
    #include "xturborc.c"
      #endif
    default: return 0;
  }
  if(l && !xcheck) { memcheck(in,n,cpy); }
  return l;
} 
#endif //NO_BENCH
 
static void usage(char *pgm) {
    #ifdef _BWTSATAN
  fprintf(stderr, "\nBwtSatan 23.05 Copyright (c) 2018-2023 Powturbo %s\n", __DATE__);
    #else
  fprintf(stderr, "\nTurboRC 23.05 Copyright (c) 2018-2023 Powturbo %s\n", __DATE__);
        #endif
  fprintf(stderr, "\n Usage: %s <options> <infile1> <outfile>\n", pgm);
  fprintf(stderr, "<options>\n");
    #ifndef _BWTSATAN   
  fprintf(stderr, " -# #: compression codec (0:all)\n");
  fprintf(stderr, "   Range Coder       : 1/2/3=order 0/1/2, 4/5=Order 8b,15b (context slide), 6/7:16-bits o0/o1, 6/7/8:32bits o0/o1/o2\n");
  fprintf(stderr, "   Context mixing    : 9/10/11= O1/O2/O1+run\n");
  fprintf(stderr, "   RLE+rc            : 12/14=o0/o1, 13/15=16-bits o0/o1\n");
  fprintf(stderr, "   varin8            : 16\n");
  fprintf(stderr, "   QLFC+rc           : 17\n");
  fprintf(stderr, "   Gamma+rc          : 26=8/16/32 bits (+option: -Ob/Os/Ou)\n");
  fprintf(stderr, "   Gamma+delta+rc    : 27=8/16/32 bits\n");
  fprintf(stderr, "   Rice+rc           : 28=8/16/32 bits\n");
  fprintf(stderr, "   Rice+delta+rc     : 29=8/16/32 bits\n");
  fprintf(stderr, "   Turbo VLC+rc+bitio: 30/31/34: 16/32 bits\n");
  fprintf(stderr, "   Turbo VLC zigzag  : 32/33/35: 16/32 bits\n");
    #ifdef _BWT
  fprintf(stderr, "   BWT+rc         : 20 options: -l#  #:0:store, 2:bit ec, 3/4:RLE, 5/6:RLE o1, 7/8:QLFC, 9:Max\n");
    #endif
    #endif
        #ifndef NO_BENCH
  fprintf(stderr, " -b#     #: block size in MB (default %d)\n", 1<<10);
  fprintf(stderr, " -d      decompress\n");
  fprintf(stderr, " -v      verbose\n");
  fprintf(stderr, " -o      write on standard output\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Ex.: turborc -1 -f file.jpg file.jpg.rc\n");
  fprintf(stderr, "     turborc -d file.jpg.rc file1.jpg\n");
  fprintf(stderr, "---------- Benchmark ---------------------\n");
  fprintf(stderr, " -k      benchmark\n");
  fprintf(stderr, " -e#      # = function ids separated by ',' or ranges '#-#' \n", ID_MEMCPY);
  fprintf(stderr, "          # = 0 Benchmark all functions\n");
  fprintf(stderr, " -i#/-j#  # = Minimum  de/compression iterations per run (default=auto)\n");
  fprintf(stderr, " -I#/-J#  # = Number of de/compression runs (default=3)\n");
  fprintf(stderr, " -n      enc/dec low nibble only\n");
  fprintf(stderr, "File input format:\n");
  fprintf(stderr, " -F[Xx[k][H]][.d]\n");
  fprintf(stderr, "    X = file format:\n");
  fprintf(stderr, "        t = text:one integer per line, k=column number in multiple columns line\n");
  fprintf(stderr, "        c = text:integers separated by non digit char\n");
  fprintf(stderr, "    x = entry format\n");
  fprintf(stderr, "        [b=int8], [s=int16], [u=int32(default)], [l=int64], [f:float] [d:double]\n");
  fprintf(stderr, "    .# = decimal digits (default 2). Convert dec. numbers to integers\n");
  fprintf(stderr, "    H  = skip first line(s)\n");
  fprintf(stderr, " -H = skip first line(s). Only for text files\n");
  fprintf(stderr, " -K# = #:column number in multiple columns/line. Only for text files\n");
  fprintf(stderr, " -ks = s:separator(s) (default , ; and TAB) in multiple columns/line. Only for text files\n");
  fprintf(stderr, " -s# = #:integer size 2,4,8\n");
  fprintf(stderr, " -f# = #:floating point size 4,8\n");
  fprintf(stderr, " -t# = #:Timestamp in iso-8601 converted to seconds (32 bits)\n");
  fprintf(stderr, " -T# = #:Timestamp in iso-8601 converted to milliseconds (64 bits)\n");
  fprintf(stderr, " -V# = #:divisor. Only for text files\n");
  fprintf(stderr, " -D# = #:decimals. Only for text files\n");
  fprintf(stderr, "\n"); 
  fprintf(stderr, "Target/Processing format:\n");
  fprintf(stderr, " -O[b|s|u]  b=8 bits s=16 bits u=32 bits\n"); 
    #endif
  //fprintf(stderr, " -f      force overwrite of output file\n");
    #ifdef _BWTSATAN
  fprintf(stderr, "compress:   bwtsatan -20[e#][m#] input output\n");
  fprintf(stderr, "   e# : # = 0:store, 2:bit ec, 3/4:RLE, 5/6:RLE o1, 7/8:QLFC, 9:Max\n");
  fprintf(stderr, "   m# : # = lzp min. match length\n");
  fprintf(stderr, "decompress: bwtsatan -d input output\n");
    #else
  fprintf(stderr, "Ex.:   turborc -e0 file\n");
  fprintf(stderr, "       benchmark all functions\n");
  fprintf(stderr, "Ex.:   turborc -e1,2,12,40 file\n");
  fprintf(stderr, "       benchmark functions with id 1,2,12 and 40\n");
  fprintf(stderr, "Ex.:   turborc -e0 file -Os\n"); 
  fprintf(stderr, "       benchmark file with 16 bits input\n"); 
  fprintf(stderr, "Ex.:   turborc -e0 file -Ft -Ou\n"); 
  fprintf(stderr, "       convert text file to 32 bits integers, then benchmark\n"); 
  fprintf(stderr, "Ex.:   turborc -e0 file -Ft -K2 -Ou\n"); 
  fprintf(stderr, "       convert csv text file to 32 bits integers, then benchmark the integers at column 2\n"); 
    #endif
  exit(1);
} 

//----------------------------- File compression header serialization ----------------------------------------------------
typedef struct hd {                                                                                     // main header
  unsigned bsize;
  unsigned short magic;
  unsigned char  codec, lev, prm1, prm2, prdid;
} hd_t;
typedef struct chdb {                                                                       // block header
  unsigned bsize, inlen, clen;
} hdb_t;

int hdwr(hd_t *hd, FILE *fo) {    																		 //if(powof2(hd->bsize)) b = ctz32(hd->bsize)<<1; // file header  else 
  unsigned hdlen = 0, u32 = hd->codec << 12 | hd->magic;                                      //12+8+12=32 bits
  if(hd->bsize < (1<<12)) u32 |= hd->bsize << 20;
  if(fwrite(&u32, 1, 4, fo) != 4) return -E_FWR;                                  hdlen   = 4; // blocksize 
  if(hd->bsize >= (1<<12)) { if(fwrite(&hd->bsize, 1, 4, fo) != 4) return -E_FWR; hdlen  += 4; }
  unsigned short u16 = hd->lev<<10 | hd->prm2<<6 | hd->prm1<<2 | (hd->prdid-1); // 2+4+4+4+2=16 bits
  if(fwrite(&u16, 1, 2, fo) != 2) return -E_FWR;             hdlen += 2;   
  return hdlen; 
}

int hdrd(hd_t *hd, FILE *fi) {                                                                                                  // file header
  unsigned hdlen, u32;
  unsigned short u16;
  if(fread(&u32, 1, 4, fi) != 4) return -E_FRD;                  hdlen  = 4;
  if((u32&0xfffu) != MAGIC) return -E_MAG;
  if((hd->codec  = (char)(u32>>12)) > CODEC_MAX)     return -E_CODEC;
  hd->bsize = u32>>20;
  if(!hd->bsize && fread(&hd->bsize, 1, 4, fi) != 4) return -E_FRD; 
  if(fread(&u16, 1, 2, fi) != 2)                     return -E_FRD;                  
  hd->prdid =  (u16&3)+1;                                        hdlen += 2;
  hd->prm1  =  (u16>> 2)&0xf;
  hd->prm2  =  (u16>> 6)&0xf;
  hd->lev   =  (u16>>10);      
  if(hd->lev > 9) return -E_CORR;
  return hdlen;
}

int hdbwr(hdb_t *hdb, FILE *fo) {                                                                                               // block header
  unsigned hdlen, h = hdb->clen >= (1<<30),                                     // block length > 1GB?
           u32 =  hdb->clen << 2 |                                              // compressed length lsb = 30bits
                          h << 1 |                                                      // large block size?
                         (hdb->inlen < hdb->bsize);                                     // last block?
  if(fwrite(&u32, 1, 4, fo) != 4) return -E_FWR;         hdlen  = 4;            //printf("clen=%u ", hdb->clen);
  if(h) {
    unsigned short u16 = hdb->clen >> 30;                                       // compressed length msb
    if(fwrite(&u16, 1, 2, fo) != 2) return -E_FWR;       hdlen += 2;
  }
  if(hdb->inlen < hdb->bsize)                                                                      // length of last block < block size
    if(fwrite(&hdb->inlen, 1, 4, fo) != 4) return -E_FWR;   
                                                         hdlen +=4;
  return hdlen;
}

unsigned hdbrd(hdb_t *hdb, FILE *fi) {
  unsigned hdlen, u32;
  //unsigned short u16;
  hdb->inlen = hdb->bsize;
  if(fread(&u32, 1, 4, fi) != 4) return -E_FRD;          hdlen  = 4;
  hdb->clen = u32>>2;                                                           //printf("clen=%u ", hdb->clen);
  if(u32&2) {
    unsigned short u16;
    if(fread(&u16, 1, 2, fi) != 2) return -E_FRD;        hdlen += 2;
    hdb->clen |= (uint32_t)u16 << 30;
  }
  if(u32&1) {                                                                   // last block
    if(fread(&hdb->inlen, 1, 4, fi) != 4) return -E_FRD; hdlen  += 4;
  }       
  return hdlen;         
}

typedef struct len_t { unsigned id, len; } len_t;
#define CMPSA(_a_,_b_, _t_, _v_)  (((((_t_ *)_a_)->_v_) > (((_t_ *)_b_)->_v_)) - ((((_t_ *)_a_)->_v_) < (((_t_ *)_b_)->_v_)))
static int cmpsna(const void *a, const void *b) { return CMPSA(a, b, len_t, len); }


//---------------------------------------------- main : benchmark + file compression ----------------------------------------------
int main(int argc, char* argv[]) {
  size_t   bsize = 1792*Mb; unsigned prdid = RC_PRD_S;
  int      xstdout=0, xstdin=0, decomp=0, codec=0, dobench=0, cmp=1, c, digit_optind=0, decs=0, divs=0, skiph=0, isize=4, dfmt=0, mdelta=0, kid=0, osize=1;
  char     *scmd = NULL, prids[8]="s", *keysep = NULL;                                                  //fdbg = fopen("test.dat", "wb"); if(!fdbg) perror("fopen failed");
  #define CODECNUM 256
  len_t    lens[CODECNUM] = { 0 };
  for(c = 0; c < CODECNUM; c++) 
    lens[c].id = 0, lens[c].len = -1;

    #ifndef _WIN32 
  { const  rlim_t kStackSize = 32 * 1024 * 1024; 
    struct rlimit rl; 
    int rc = getrlimit(RLIMIT_STACK, &rl);
    if (!rc && rl.rlim_cur < kStackSize) { 
      rl.rlim_cur = kStackSize; 
          if(rc = setrlimit(RLIMIT_STACK, &rl)) { 
            fprintf(stderr, "setrlimit failed. rc = %d. set stack size to '20971520'\n", rc); 
          }
    }
  }
    #endif
  tm_verbose = 4;
  
  for(;;) {
    int this_option_optind = optind ? optind : 1, optind = 0;
    static struct option long_options[] = {
      { "help",     0, 0, 'h'},
      { 0,          0, 0, 0}
    }; 
    if((c = getopt_long(argc, argv, "0:1:2:3:4:5:6:7:8:9:b:B:cde:fF:g:G:hH:I:J:k:K:l:m:noO:p:P:q:Q:r:S:t:T:UV:v:x:XY:zZ:", long_options, &optind)) == -1) break;
    switch(c) {
      case 0:
        printf("Option %s", long_options[optind].name);
        if(optarg) printf(" with arg %s", optarg);  printf ("\n");
        break;          
      case 'b': bsize = argtol(optarg); if(bsize < 16) bsize = 16;else if(bsize > BLKMAX*Mb) bsize = BLKMAX*Mb; break;
      case 'e': scmd = optarg; dobench++; break;
      case 'f': xprep8=1; break;      
      case 'F': { char *s = optarg;    // Input format
        switch(*s) {
          case 'c': dfmt = T_CHAR; s++; break;
          case 't': dfmt = T_TXT; if(*++s > '0' && *s <= '9') { kid = *s++ - '0'; if(*s > '0' && *s <= '9') kid = kid*10 + (*s++ - '0'); } break;
          case 'e': dfmt = T_TST; s++; break;
          case 'r': dfmt = T_RAW; s++; break; // raw default
        }
        switch(*s) {
          case 'b': isize =  1, s++; break;                                     // 1 byte
          case 's': isize =  2, s++; break;                                     // 2 bytes
          case 'u': isize =  4, s++; break;                                     // 4 bytes
          case 'l': isize =  8, s++; break;                                     // 8 bytes
          case 'f': isize = -4, s++; break;                                     // float : 4 bytes
          case 'd': isize = -8, s++; break;                                     // double: 8 bytes
          case 't': isize =  4, s++, dfmt = T_TIM32; break; // 4 bytes, timestamp
          case 'T': isize =  8, s++, dfmt = T_TIM64; break; // 8 bytes, timestamp
        }
        if(*s == '.') { if(*++s >= '0' && *s <= '9') { decs = s[0] - '0'; s++; } } // number of decimals after .
        if(*s == 'v') { divs = strtod(++s, &s); }
        if(*s == 'H') { skiph++; s++; } // skip first line(s). ex.  HHH : skip 3 first lines
        //switch(*s) { case 's': be_mindelta = 0; break; case 'S': be_mindelta = 1; break; case 'z': be_mindelta = 2; break; }
      } break;
      case 'O': { char *s = optarg;   // target/processing format
        switch(*s) {
          case 'b': osize =  1, s++; break; // 1 byte
          case 's': osize =  2, s++; break; // 2 bytes
          case 'u': osize =  4, s++; break; // 4 bytes 
          case 'f': osize = -4, s++; break; // float : 4 bytes
          case 'd': osize = -8, s++; break; // double: 8 bytes
        }
      } break;
	  case 'g': gmin = strtod(optarg, NULL); break;
	  case 'G': gmax = strtod(optarg, NULL); break;
      case 'H': skiph = atoi(optarg); break;
      case 'K': { kid = atoi(optarg); if(!keysep) keysep = ",;\t"; } break;
      case 'k': keysep = optarg; break;
      case 'T': itmax = atoi(optarg); break;
      case 'c': cmp++;     break;
      case 'd': decomp++;  break;
      case 'n': xnibble++; break;
      case 'o': xstdout++; break;
      case 'P': prdid = atoi(optarg); if(prdid<1 || prdid>RC_PRD_LAST) prdid=RC_PRD_LAST; break;
      case 'p': { char *p = optarg; strncpy(prids, p, 2); prids[2]=0;
                  if(p[0]=='s') {
                         if(!p[1])       prdid = 1; 
                    else if(p[1] == 's') prdid = 2; 
                    else if(p[1] == 'f') prdid = 3;
                  } else if(p[0]=='n' && p[1] == 'z') prdid = 4;
         
      } break;
	  case 'q': quantb = atoi(optarg); break;
	  //case 'Q': qmax   = atoi(optarg); break;
      case 'v': verbose = atoi(optarg); break;
          
      case 'U': nutf8++;    break;
      case 'X': bwtx++;    break;
      case 'S': xsort = atoi(optarg); break;
      case 'z': forcelzp = BWT_LZP; break;
      case 'r': { char *p = optarg; if(*p >= '0' && *p <= '9') { prm1 = p[0]-'0'; prm2 = p[1]-'0'; } if(prm1>9) prm1=9; if(prm2>9) prm2=9; } break;
      case 't': xtpbyte = atoi(optarg); if(xtpbyte) { if(xtpbyte < 1) xtpbyte = 1;else if(xtpbyte > 30) xtpbyte = 30; } break; 
        #ifndef NO_BENCH
      case 'I': if((tm_Rep  = atoi(optarg))<=0) tm_rep = tm_Rep =1; break;
      case 'J': tm_Rep2 = atoi(optarg); if(tm_Rep2<0) xcheck++,tm_Rep2=-tm_Rep2; if(!tm_Rep2) tm_rep= tm_Rep2=1;  break;
        #endif
      case 'l': lev = atoi(optarg); if(lev>9) lev=9; break;
      case 'm': lenmin = atoi(optarg); if(lenmin > 256) lenmin = 256; break;
      case 'Y':      if(!strcasecmp(optarg,"sse"))    cpuini(0x33);  
                else if(!strcasecmp(optarg,"avx"))    cpuini(0x50); 
                else if(!strcasecmp(optarg,"avx2"))   cpuini(0x60); 
                else if(!strcasecmp(optarg,"avx512")) cpuini(0x78);   
                else cpuini(0x1);
        break;
      case 'V': tm_verbose = atoi(optarg);  break;
        #ifndef NO_BENCH
      case 'x': { int m = atoi(optarg); if(m<4) m=4;else if(m>16) m=16; mbcset(m); /*set context bits*/} break;
        #endif
	  case 'Z': zerrlim = strtod(optarg, NULL); break;

      case '0':case '1':case '2':case '3': case '4':case '5':case '6':case '7':case '8':case '9': {
        char *q;
                unsigned l = atoi(optarg); 
        if(l >= 0 && l <= 9)
          codec = (c-'0')*10 + l;                                  
                if(q = strchr(optarg,'e')) lev    = atoi(q+(q[1]=='='?2:1));  if(lev>9) lev=9;   
            if(q = strchr(optarg,'m')) lenmin = atoi(q+(q[1]=='='?2:1));                    
            if(q = strchr(optarg,'U')) nutf8  = 1;                        if(verbose>2) printf("codec=%d lev=%d lmin=%d nutf8=%d ", codec, lev, lenmin, nutf8);
                decomp = 0;
        if(codec>=0 && codec<=99) break; 
      }
      case 'h':
      default: 
        usage(argv[0]);
        exit(0); 
    }
  }
  //anscdfini(0);                                                                   if(verbose>1) printf("detected simd id=%x, %s\n\n", cpuini(0), cpustr(cpuini(0)));
  tm_init(tm_Rep, tm_verbose /* 2 print id */);  
  #define ERR(e) do { rc = e; printf("line=%d ", __LINE__); goto err; } while(0)
  int  rc = 0; unsigned inlen;
  unsigned char *in = NULL, *out = NULL, *cpy = NULL; 
    #ifdef _SF
  if(prdid == RC_PRD_SF) { printf("fsm"); if(prm1<0) prm1=1;if(prm1>9) prm1=9; fsm_init(prm1); }
    #endif
    #ifdef _NZ
  if(prdid == RC_PRD_NZ) { printf("nz"); }
    #endif
    #ifndef NO_BENCH
  if(dobench) { //---------------------------------- Benchmark -----------------------------------------------------
    char _scmd[33];
    int  fno=0, nblk = 0, nid = 0, cminid=0, cminl = (unsigned)-1;
    sprintf(_scmd, "1-%d", ID_MEMCPY);                                                      if(verbose>2) printf("BENCHMARK ARGS: fno=%d,optind=%d,argc=%d\n", fno, optind, argc);
    printf("      size   ratio     E MB/s   D MB/s function prdid=");
    switch(prdid) {
      case 1: printf("'s(5)'\n"); break;
      case 2: printf("'ss(%u,%u)'\n", prm1, prm2); break;
      case 3: printf("'sf(%u)'\n", prm1); break;
      case 4: printf("'nz'\n"); break;
    }
    uint64_t clen = 0;  
    for(fno = optind; fno < argc; fno++) {
      uint64_t filen;
      int      n;    
      char     *finame = argv[fno];                                     
      FILE     *fi = fopen(finame, "rb");                                           if(!fi ) { perror(finame); continue; }   if(verbose>2) printf("'%s'\n", finame);

      fseeko(fi, 0, SEEK_END); 
      filen = ftello(fi);                                                                                                           if(!filen) { printf("file empty: '%s'\n",finame); continue; }
      fseeko(fi, 0, SEEK_SET);
    
      size_t b = (filen < bsize && !dfmt)?filen:bsize, tpbyte=0;                            if(verbose>2) printf("bsize=%zu b=%zu ", bsize, b);
      
      in  = vmalloc(b*4/3);    if(!in)  ERR(E_MEM);
      out = vmalloc(b*4/3);    if(!out) ERR(E_MEM);
      cpy = vmalloc(b*4/3);    if(!cpy) ERR(E_MEM);
      unsigned tid = 0;
      if((n = strlen(finame)) >= 3) {                     // auto method determination based on filename
        unsigned char *p = finame+n-3;
        if(!xtpbyte) { 
          if(     !strcasecmp(p,"16u") || !strcasecmp(p,"u16")) { tid = 7; prm1=5;prm2=7; }
          else if(!strcasecmp(p,"16s") || !strcasecmp(p,"s16")) { tid = 7; prm1=5;prm2=7; }
          if(!strcasecmp(p,"32u") || !strcasecmp(p,"u32")) { tid = 8; }
            else if(!strcasecmp(p,"32s") || !strcasecmp(p,"s32")) { tid = 8; }
            else if(!strcasecmp(p,"64u") || !strcasecmp(p,"u64")) { tpbyte = 8; } // transpose (not included)
            else if(!strcasecmp(p,"16f") || !strcasecmp(p,"f16")) { tpbyte = 2; }
            else if(!strcasecmp(p,"32f") || !strcasecmp(p,"f32")) { tid = 8; prm1=1; prm2=6; }
            else if(!strcasecmp(p,"64f") || !strcasecmp(p,"f64")) { tpbyte = 8; }
          //else if(!strcasecmp(p,"bmp")) { tpbyte = 16; }         
        } else tpbyte = xtpbyte;
      }
      for(;;) {                                                                                                                     
        n = dfmt?befgen(fi, in, b, dfmt, isize, osize, kid, skiph, decs, divs, keysep, mdelta):fread(in, 1, b, fi); 
        if(n <= 0) break;                                                       if(verbose>2) printf("read=%u t=%zd\n", n, tpbyte);		//memcpy(cpy, in, n); fpstat(in, n/2, cpy, -2, NULL);
        switch(tpbyte) {
			#ifdef _TRANSPOSE
          case 22:  tpenc(in, n, out, 2); memcpy(in, out, n); break;
          case 11: delta8e24(in,n,out); tpenc(out, n, in, 3); break;
          case 12: tpenc(in, n, out, 3); memcpy(in, out, n); break;
		    #endif
			#ifdef _TURBORLE
          case 13: { unsigned l = bitenc(in, n, out); l = trlec(out, l, in); n = l; } break;
		    #endif
          case 14: { unsigned l = bitenc(in, n, out); memcpy(in, out, l); n = l; } break;
            #ifndef _NDELTA
          case 15: delta8e16(in,n,out); memcpy(in, out, n); break;
          case 16: delta16e16(in,n,out); memcpy(in, out, n); break;
          case 17: delta8e24( in,n,out); memcpy(in, out, n); break;
          case 18: delta16e32(in,n,out); memcpy(in, out, n); break;
          case 19: xorenc16(in,n,out); memcpy(in, out, n); break;
          case 20: zzagenc16(in,n,out); memcpy(in, out, n); break;
          case 21: nbenc16(in,n,out); memcpy(in, out, n); break;
          //case 16: delta24e24(in,n,out); memcpy(in, out, n); break;
            #endif
			#ifndef _NQUANT
			  #if defined(FLT16_BUILTIN)
		  case 7: { _Float16 fmin=0.0,fmax=0.0;	if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;											
	        if(!quantb || quantb > 8) quantb = 8;                                          printf("Quantization=%d\n", quantb);
		    n = fpquant8e16(in,n,out,BZMASK32(quantb), &fmin,&fmax,FLT16_EPSILON); memcpy(in,out,n); 
		  } break;			  
		  case 8: { _Float16 fmin=0.0,fmax=0.0;	if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;
	        if(!quantb || quantb > 16) quantb = 16;                                             printf("Quantization=%d\n", quantb);
		    fpquant16e16(in,n/2,out,BZMASK32(quantb), &fmin,&fmax,FLT16_EPSILON); tpenc(out, n, in, 2);
		  } break;			  
		      #endif
		  case 9 : { float fmin=0.0, fmax=0.0;	if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;
	        if(!quantb || quantb > 32) quantb = 32;
	        fpquant32e32(in,n/4,out,BZMASK32(quantb), &fmin,&fmax,FLT_EPSILON);  tpenc(out, n, in, 4);             
		  } break;
		  case 10 : { double fmin=0.0, fmax=0.0;	if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;
	        if(!quantb || quantb > 32) quantb = 32;
	        fpquant64e64(in,n/8,out,BZMASK32(quantb), &fmin,&fmax,DBL_EPSILON);  tpenc(out, n, in, 8);            
		  } break;
			#endif
        }

        nblk++;
        char *p = (scmd && (scmd[0] != '0' || scmd[1]))?scmd:_scmd;     
                  
        if(tid) { 
          uint64_t l; 
          if(l = bench(in, n, out, cpy, tid, prdid, osize)) printf("\t%s\n", finame);  
          clen += l;
        } else do { 
          int id = strtoul(p, &p, 10),idx = id, i;
          uint64_t l; 
          if(id >= 0) {    
            while(isspace(*p)) p++; 
            if(*p == '-') { if((idx = strtoul(p+1, &p, 10)) < id) idx = id; if(idx > ID_LAST) idx = ID_LAST; } //printf("ID=%d,%d ", id, idx);
            for(i = id; i <= idx; i++) {
              if(l = bench(in, n, out, cpy, i, prdid, osize)) {
                printf("\t%s\n", finame);  
                clen        += l;
                lens[i].id   = i;
                lens[i].len  = l;
                nid++;
              }
            }                     
          }        
        } while(*p++);
      }
      fclose(fi);         fi  = NULL;
      if(in)  vfree(in);  in  = NULL; 
      if(out) vfree(out); out = NULL; 
      if(cpy) vfree(cpy); cpy = NULL;      
    }
        
    if(argc - optind > 1) 
      printf("Total compressed %lld\n", clen);
    else if(nblk == 1 && nid > 1) { 
      int i; 
      qsort(lens, CODECNUM, sizeof(lens[0]), cmpsna); 
      printf("Best methods =");
      for(i = 0; i < 20; i++) 
        if(lens[i].len != -1) 
          printf("%s%d", i?",":"", lens[i].id);  
      }                                                                                                                               //if(fdbg) fclose(fdbg); 
      printf("\n"); exit(0);   
    }
    #endif
  //---------------------------------- File Compression/Decompression -----------------------------------------------------
  if(!decomp && prdid != 1 && prdid !=2) {
        fprintf(stderr,"Only predictors 's' and 'ss' are supported for file compression\n");
        exit(0);
  }
  if(argc <= optind) xstdin++;
  unsigned long long filen=0,folen=0;
  char *finame = xstdin ?"stdin":argv[optind], *foname, _foname[1027];          if(verbose>1) printf("'%s'\n", finame);
  
  if(xstdout) foname = "stdout";
  else {
        if(optind+1 >= argc) { fprintf(stderr, "destination file not specified or wrong number of parameters\n");exit(-1); }
    foname = argv[optind+1];                                                                 
    if(!decomp) {
      int len = strlen(foname), xext = len>3 && !strncasecmp(&foname[len-3], ".rc", 3);
      if(!xext && len < sizeof(_foname)-3) {
        strcpy(_foname, foname);
        strcat(_foname, ".rc");
        foname = _foname;
      }
    }
  }
  if(!strcasecmp(finame,foname)) { printf("'%s','%s' \n", finame, foname); ERR(E_FSAME); }

  FILE *fi = xstdin ?stdin :fopen(finame, "rb"); if(!fi) { perror(finame); return 1; }
  FILE *fo = xstdout?stdout:fopen(foname, "wb"); if(!fo) { perror(finame); return 1; } 
    #ifndef NO_COMP 
  if(!decomp) {  																						if(verbose>3) printf("bsize=%zu ", bsize);
    hd_t hd = { 0 }; hd.magic = MAGIC; hd.bsize = bsize; hd.codec = codec; hd.lev = lev; hd.prdid = prdid; hd.prm1 = prm1; hd.prm2 = prm2; 
    if((rc = hdwr(&hd, fo)) < 0) ERR(-rc); folen = rc;      
    in  = vmalloc(bsize+1024);
    out = vmalloc(bsize*4/3+1024); if(!in || !out) ERR(E_MEM); 
    unsigned clen;
    while((inlen = fread(in, 1, bsize, fi)) > 0) {        filen += inlen;                               if(verbose>3) printf("read=%u ", inlen);
      switch(codec) {             
        case  0: clen = inlen; memcpy(out, in, inlen);    break;
                  #ifndef NO_RC
        case  1: clen = rcenc(    in, inlen, out, prdid); break;
        case  2: clen = rccenc(   in, inlen, out, prdid); break;
        case  3: clen = rcc2enc(  in, inlen, out, prdid); break;
        case  4: clen = rcxenc(   in, inlen, out, prdid); break;
        case  5: mbcset(15); clen = rcx2enc(  in, inlen, out, prdid); break;
        case  9: clen = rcmsenc(  in, inlen, out);        break;
        case 10: clen = rcm2senc( in, inlen, out);        break;
        case 12: clen = rcrleenc( in, inlen, out, prdid); break;
        case 14: clen = rcrle1enc(in, inlen, out, prdid); break;
        case 17: clen = rcqlfcenc(in, inlen, out, prdid); break;
        case 18: clen = becenc8(  in, inlen, out);        break;
        case 19: clen = becenc16( in, inlen, out);        break;
                  #endif
          #ifdef _BWT
        case 20: clen = rcbwtenc( in, inlen, out, lev, thnum, bwtflag(osize)); break;
          #endif            
        case 21: clen = utf8enc( in, inlen, out, bwtflag(osize)); break; 
          #ifndef _NDELTA
        case 22: delta8e24(in,inlen,out); clen = inlen+1; break;
          #endif
	      #ifndef _NQUANT
	        #if defined(FLT16_BUILTIN)
        case 24: { _Float16 fmin = -1.16, fmax = 1.4; if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;
		  if(quantb > 8) quantb = 8;                                     
	      clen = fpquant8e16(in,inlen,out, BZMASK32(quantb), &fmin, &fmax, FLT16_EPSILON);  
          if(clen < inlen) {
		    ctof16(out+clen) = fmin; ctof16(out+clen+2) = fmax; ctou8(out+clen+4) = quantb; clen += 4+1;
          }	                                                                    if(verbose>2) printf("\nlen:%u R:[%g/%g]=%g q=%u ", clen, (double)fmin, (double)fmax, (double)fmax-(double)fmin, quantb);	   
	    } break;
        //case 23: { _Float16 fmin=0.0, fmax=0.0; if(quantb > 16) quantb = 16; clen = fpquantv8e16(in,inlen,out, BZMASK32(quantb), &fmin, &fmax, FLT16_EPSILON); if(clen < inlen) { ctof16(out+clen) = fmin; ctof16(out+clen+2) = fmax; ctou8(out+clen+4) = quantb; clen += 4+1;
        //  }	else printf("overflow ");                                           if(verbose>2) printf("\nlen:%u R:[%g/%g]=%g q=%u ", clen, (double)fmin, (double)fmax, (double)fmax-(double)fmin, quantb);	   
	    //} break;
        case 25: { _Float16 fmin=0.0, fmax=0.0; if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax; if(quantb > 16) quantb = 16;
	      fpquant16e16(in,inlen,out, BZMASK32(quantb), &fmin, &fmax, FLT16_EPSILON); memcpy(in,out,inlen); tpenc(in, inlen, out, 2); 
		  ctof16(out+inlen) = fmin; ctof16(out+inlen+2) = fmax; ctou8(out+inlen+4) = quantb; clen = inlen+4+1;  
                                                                                if(verbose>2) printf("\nlen:%u R:[%g/%g]=%g q=%u ", inlen, (double)fmin, (double)fmax, (double)fmax-(double)fmin, quantb);	    
		} break;
	        #endif
        case 26: { float fmin=0.0, fmax=0.0;	if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;
  		  if(quantb > 32) quantb = 32;
	      fpquant32e32(in,inlen,out, BZMASK32(quantb), &fmin, &fmax, FLT_EPSILON); memcpy(in,out,inlen); tpenc(in, inlen, out, 4); 
		  ctof32(out+inlen) = fmin; ctof32(out+inlen+4) = fmax; ctou8(out+inlen+8) = quantb; clen = inlen+8+1;	 if(verbose>2) printf("len:%u R:[%g/%g]=%g q=%u ", inlen, (double)fmin, (double)fmax, (double)fmax-(double)fmin, quantb);
	    } break;
        case 27: { double fmin=0.0, fmax=0.0;	if(gmin != FP_ZERO) fmin = gmin; if(gmax != FP_ZERO) fmax = gmax;
		  if(quantb > 32) quantb = 32;
	      fpquant64e64(in,inlen,out, BZMASK32(quantb), &fmin, &fmax, DBL_EPSILON); memcpy(in,out,inlen); tpenc(in, inlen, out, 8); 
		  ctof64(out+inlen) = fmin; ctof64(out+inlen+8) = fmax; ctou8(out+inlen+16) = quantb; clen = inlen+16+1; 
		                                                                        if(verbose>2) printf("\nlen:%u R:[%g/%g]=%g q=%u ", inlen, (double)fmin, (double)fmax, (double)fmax-(double)fmin, quantb);	 
	    } break;
	      #endif
        default: ERR(E_CODEC); 
      }
      hdb_t hdb = { 0 }; hdb.inlen = inlen; hdb.bsize = bsize; hdb.clen = clen; if ((rc = hdbwr(&hdb, fo)) < 0) ERR(-rc); folen += rc;
      if(fwrite(out, 1, clen, fo) != clen) ERR(E_FWR);          
          folen += clen;
    }                                                                           if(verbose>2) printf("compress: '%s'  %lld->%lld\n", finame, filen, folen);                                                                    
  } else 
        #endif
  { // Decompress
    hd_t hd; if((rc = hdrd(&hd, fi)) < 0) ERR(-rc); filen = rc; bsize = hd.bsize; codec = hd.codec; lev = hd.lev; prdid = hd.prdid; prm1 = hd.prm1; prm2 = hd.prm2; 
    in    = vmalloc(bsize); 
    out   = vmalloc(bsize); if(!in || !out) ERR(E_MEM); 
    for(;;) {
      hdb_t hdb = { 0 }; hdb.bsize = bsize; hdb.inlen = 0;
      if((rc = hdbrd(&hdb, fi)) < 0) break;     
      filen += rc;
      unsigned outlen = hdb.inlen, inlen = hdb.clen;
      if(fread(in, 1, inlen, fi) != inlen) ERR(E_FRD);   
        filen += inlen; // read block
      if(inlen == outlen) memcpy(out, in, outlen);
      else switch(codec) { 
              #ifndef NO_RC
        case  0: memcpy(out,in, outlen);             break;
        case  1: rcdec(     in, outlen, out, prdid); break;
        case  2: rccdec(    in, outlen, out, prdid); break;
        case  3: rcc2dec(   in, outlen, out, prdid); break;
        case  4: rcxdec(    in, outlen, out, prdid); break;
        case  5: mbcset(15); rcx2dec(in, outlen, out, prdid); break;
        case  9: rcmsdec(   in, outlen, out);        break;
        case 10: rcm2sdec(  in, outlen, out);        break;
        case 12: rcrledec(  in, outlen, out, prdid); break;
        case 14: rcrle1dec( in, outlen, out, prdid); break;
        case 17: rcqlfcdec( in, outlen, out, prdid); break;
        case 18: becdec8(   in, outlen, out);        break;
        case 19: becdec16(  in, outlen, out);        break;
                  #endif
              #ifdef _BWT
        case 20: rcbwtdec(  in, outlen, out, lev, thnum); break;
          #endif
        case 21: utf8dec(   in, outlen, out);        break;
        case 22: delta8d24(in,outlen,out); break; 
	      #ifndef _NQUANT
	        #if defined(FLT16_BUILTIN) 
	    case 24: { _Float16 fmin = ctof16(in+inlen-5), fmax = ctof16(in+inlen-3); quantb = ctou8(in+inlen-1); if(verbose>3) printf("len = %u R:[%g - %g] q=%u ", outlen, (double)fmin, (double)fmax, quantb);
	      fpquant8d16(in, outlen, out, BZMASK32(quantb), fmin, fmax, inlen-5); 
	    } break;
	  //case 23: { _Float16 fmin = ctof16(in+inlen), fmax = ctof16(in+inlen+2); quantb = ctou8(in+inlen+4); fpquantv8d16(in, outlen, out, BZMASK32(quantb), fmin, fmax); } break;
	    case 25: { _Float16 fmin = ctof16(in+outlen), fmax = ctof16(in+outlen+2); quantb = ctou8(in+outlen+4); 
          tpdec(in, outlen, out,  2); memcpy(in, out, outlen); fpquant16d16(in, outlen, out, BZMASK32(quantb), fmin, fmax); 
	    } break;
	        #endif
	    case 27: { float    fmin=ctof32(in+outlen), fmax = ctof32(in+outlen+4); quantb = ctou8(in+outlen+8);  
	      tpdec(in, outlen, out, 4); memcpy(in, out, outlen); fpquant32d32(in, outlen, out, BZMASK32(quantb), fmin, fmax);
	    } break;
	    case 28: { double   fmin=ctof32(in+outlen), fmax = ctof32(in+outlen+8); quantb = ctou8(in+outlen+16); 
	      tpdec(in, outlen, out, 8); memcpy(in, out, outlen); fpquant32d32(in, outlen, out, BZMASK32(quantb), fmin, fmax);
	    } break;
	      #endif
        default: ERR(E_CODEC); 
      }  
      if(fwrite(out, 1, outlen, fo) != outlen) ERR(E_FWR); folen += outlen;  
      if(hdb.inlen < bsize) break;        
    }                                                                           if(verbose>2) printf("decompress:'%s' %lld->%lld\n", foname, filen, folen);
  }                                                              
  end: rc = 0;
  if(fi) fclose(fi);
  if(fo) fclose(fo);
  err: if(rc) { fprintf(stderr,"%s\n", errs[rc]); fflush(stderr); }
  if(in) vfree(in);
  if(out) vfree(out);
  if(cpy) vfree(cpy);
  return rc;
}
