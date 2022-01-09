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
// TurboRC: Range Coder Benchmark and Compressor Application
#include <stdio.h>
#include <string.h>
  #if !defined(_WIN32) && !defined(_WIN64)
#include <sys/resource.h>
  #endif

#include "conf.h"

  #ifdef _MSC_VER
#include "vs/getopt.h"
  #else
#include <getopt.h> 
  #endif
 
#include "rcutil_.h"
#include "rcutil.h"
#include "time_.h"
#include "turborc.h"
#include "bec.h"

  #ifdef _LIBSAIS
#include "libsais/src/libsais.h"
  #endif
  #ifdef _BWTDIV      
#include "libdivsufsort/include/divsufsort.h"
#include "libdivsufsort/include/unbwt.h"
  #endif

  #ifdef _EXT
#include "xturborc.h"
  #endif
  
//#define NO_BENCH
//#define NO_RC

#define MAGIC     0x153 // 12 bits
#define BLKMAX    3584
#define CODEC_MAX 32  // max. id for file compression
int verbose;
enum { E_FOP=1, E_FCR, E_FRD, E_FWR, E_MEM, E_CORR, E_MAG, E_CODEC, E_FSAME };

static char *errs[] = {"", "open error", "create error", "read error", "write error", "malloc failed", "file corrupted", "no TurboRc file", "no codec", "input and output files are same" };
 
// program parameters
static unsigned xnibble, lenmin = 64, lev=9, thnum=0, xtpbyte=-1;
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
  struct tm tm;
  uint64_t  u;
  char     *s=p;
  int       frac = 0,c;

  memset(&tm, 0, sizeof(tm)); tm.tm_mday = 1;
  while(!isdigit(*p)) p++;
  u = strtoull(p, &p, 10);                  // first number

  if(     u <= 99) u += 2000;               // year  "yy": 00-99 -> 2000-2099
  else if(u >= 19710101 && u < 20381212) {  // date: "yyyymmdd"
    tm.tm_year =  u/10000;
    tm.tm_mon  = (u%10000)/100; if(!tm.tm_mon  || tm.tm_mon  > 12) goto a; tm.tm_mon--;
    tm.tm_mday = u%10;          if(!tm.tm_mday || tm.tm_mday > 31) goto a;
    goto h;
  } else if(u < 1971 || u > 2099) goto a;   // invalid
  tm.tm_year = u;                           // year       "yyyy"
  c = *p;                                   // month,day: "mm.dd", "mm-dd", "mm/dd"
  if(c != '.' && c != '-' && c != '/') goto b; tm.tm_mon    = strtoul(p+1, &p, 10); if(!tm.tm_mon  || tm.tm_mon  > 12) goto a; tm.tm_mon--;
  if(c != '.' && c != '-' && c != '/') goto b; tm.tm_mday   = strtoul(p+1, &p, 10); if(!tm.tm_mday || tm.tm_mday > 31) goto a;
  if(c != '.' && c != '-' && c != '/') goto b; h:tm.tm_hour = strtoul(p+1, &p, 10);
  if(tm.tm_hour <= 24 && *p == ':') {       // time ":hh:mm:ss.frac", ":hh:mm:ss,frac"
    tm.tm_min = strtoul(p+1, &p, 10); if(tm.tm_min > 60) tm.tm_hour = tm.tm_min = 0;
    tm.tm_sec = strtoul(p+1, &p, 10); if(tm.tm_sec > 60) tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    if(type > 0 && (*p == '.' || *p == ',')) { frac = strtoul(p+1, &p, 10); if((c=p-(p+1)) > 6) frac /= 1000000;else if(c > 3) frac /= 1000; }
  } else tm.tm_hour = 0;
  b:u = mktime(&tm);
  u = u * 1000 + frac;                      // milliseconds
  a:*pq = p;                                               //if(verbose >= 9) printf("[%d-%d-%d %.2d:%.2d:%.2d.%d]\n", tm.tm_year, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, frac, u);exit(0);
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
    fgets(s, LSIZE, fi);                                                    if(verbose>=5 && (op-out)/osize < 100 ||verbose>5) printf("skip first line\n");
  }
  if(decs) {
    pre = decs?pow(10.0f,(float)decs):1;
    pre /= divs;
  } else pre = 1;
  
  
  switch(fmt) {
    case T_TXT:
    case T_TIM32:
    case T_TIM64:                                                           if(verbose>1) printf("reading text lines. pre=%.2f, col=%d, sep='%s'\n", pre, kid, keysep?keysep:"");
      while(fgets(s, LSIZE, fi)) {                                          //printf("'%s' ", s);
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
          a: EPUSH(op,out_,osize,u);            c=*q; *q=0; if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) printf("\'%s\'->%llu  ", p, u); *q = c;
        } else if(osize > 0) {
          while(!isdigit(*p) && *p != '-' && *p != '+') p++;
          uint64_t u = strtoll(p, &q, 10)*pre - mdelta;
          if(*q == '.')
            u = pre>1.0?round(strtod(p, &q)*pre):strtod(p, &q) - mdelta;
          EPUSH(op,out_,osize,u);                             c=*q;   *q=0; if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) printf("\'%s\'->%lld ", p, u); *q = c;
        } else {
          while(*p && !isdigit(*p) && *p != '-' && *p != '.' && *p != '+') {  if(keysep && strchr(keysep,*p)) keyid++; p++; }
          double d = strtod(p, &q) - mdelta;
          uint64_t u;
          memcpy(&u,&d,sizeof(u));
          EPUSH(op,out_,osize,u);                                          if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) { c=*q; *q=0; double d; memcpy(&d,&u,sizeof(d)); printf("\'%s\'->%f  ", p, d); *q = c; }
        }
      }
      break;
    case T_CHAR:                                                           if(verbose>1) printf("reading char file. pre=%.2f\n", pre);
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
          EPUSH(op,out_,osize,u);                                         if(verbose>=5 && (op-out)/osize < 100 || verbose>=9) printf("'%s'->%lld ", s, (int64_t)u);
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
    default: { unsigned char *ip = s;  //printf("(%u,%u)", isize, osize);
      for(;;) {
	  if(fread(s, 1, abs(isize), fi) != abs(isize)) goto end;  //printf("#");
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
	  //die("unknown data format %d\n", fmt);
  }
  end:;if(verbose >= 5) printf(" n=%d \n", op-out);
  if(ovf) { unsigned l = (op-out)/abs(osize); printf("Number of items truncated=%u of %u = %.2f%%\n", ovf, l, (double)ovf*100.0/(double)l ); }
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
unsigned bwtx, forcelzp, xprep8, xsort;
int BGFREQMIN = 50, BGMAX = 250, itmax;

  #ifdef _SF
#define SF(x) x
  #else
#define SF(x) 
  #endif

// Generate functions with all predictors
#define RCGEN(_p_) \
size_t _p_##enc(unsigned char *in, size_t inlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
	case RC_PRD_S :    return _p_##senc( in, inlen, out);\
	case RC_PRD_SS:    return _p_##ssenc(in, inlen, out, prm1,prm2);\
	case RC_PRD_SF: SF(return _p_##sfenc(in, inlen, out, fsm));\
  }\
}\
\
size_t _p_##dec(unsigned char *in, size_t outlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
	case RC_PRD_S :    return _p_##sdec( in, outlen, out);\
	case RC_PRD_SS:    return _p_##ssdec(in, outlen, out, prm1,prm2);\
	case RC_PRD_SF: SF(return _p_##sfdec(in, outlen, out, fsm));\
  }\
}

// Generate integer functions with all predictors
#define RCGEN2(_p_, _s_) \
size_t _p_##enc##_s_(unsigned char *in, size_t inlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
	case RC_PRD_S :    return _p_##senc##_s_( in, inlen, out);\
	case RC_PRD_SS:    return _p_##ssenc##_s_(in, inlen, out, prm1,prm2);\
	case RC_PRD_SF: SF(return _p_##sfenc##_s_(in, inlen, out, fsm));\
  }\
}\
\
size_t _p_##dec##_s_(unsigned char *in, size_t outlen, unsigned char *out, int prdid) {\
  switch(prdid) {\
	case RC_PRD_S :    return _p_##sdec##_s_( in, outlen, out);\
	case RC_PRD_SS:    return _p_##ssdec##_s_(in, outlen, out, prm1,prm2);\
	case RC_PRD_SF: SF(return _p_##sfdec##_s_(in, outlen, out, fsm));\
  }\
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

RCGEN(rcm)    		// context mixing
RCGEN(rcm2)
RCGEN(rcmr)  

RCGEN(rcrle)        // rle
RCGEN2(rcrle, 16)
RCGEN(rcrle1)
RCGEN2(rcrle1,16)
RCGEN(rcqlfc)     	// qlfc
RCGEN(rcu3)         // varint8

RCGEN2(rcg,   8)  	// gamma
RCGEN2(rcg,  16)
RCGEN2(rcg,  32)
RCGEN2(rcgz,  8)
RCGEN2(rcgz, 16)
RCGEN2(rcgz, 32)

RCGEN2(rcr,   8)   	// rice
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

size_t rcbwtenc(unsigned char *in, size_t inlen, unsigned char *out, int prdid, unsigned flag) { 
  switch(prdid) {
	case RC_PRD_S :    return rcbwtsenc( in, inlen, out, lev, thnum, flag);
	case RC_PRD_SS:    return rcbwtssenc(in, inlen, out, lev, thnum, flag, prm1,prm2);
	case RC_PRD_SF: SF(return rcbwtsfenc(in, inlen, out, lev, thnum, flag, fsm));
  }
}

size_t rcbwtdec(unsigned char *in, size_t outlen, unsigned char *out, int prdid) {
  switch(prdid) {
	case RC_PRD_S :    return rcbwtsdec( in, outlen, out, lev, thnum);
	case RC_PRD_SS:    return rcbwtssdec(in, outlen, out, lev, thnum, prm1,prm2);
	case RC_PRD_SF: SF(return rcbwtsfdec(in, outlen, out, lev, thnum, fsm));
  }
}

//********************************************************************************************************
//FILE *fdbg;
#define ID_RC16 8
#define bwtflag(z) (z==2?BWT_BWT16:0) | (xprep8?BWT_PREP8:0) | forcelzp | (verbose?BWT_VERBOSE:0) | xsort <<14 | itmax <<10 | lenmin
unsigned bench(unsigned char *in, unsigned n, unsigned char *out, unsigned char *cpy, int id, int r, int z) {
  unsigned l = 0, m = 0x100, flag = bwtflag(z);                             
  cdf_t cdf[0x100+1]; 
  if(xnibble) 										// use only low nibble
    for(int i = 0; i < n; i++) in[i] &= 0xf;
    #ifndef _MSC_VER
  memrcpy(cpy,in,n); 
    #endif
  if(id >= 40 && id <= 59) {                       // enable low nibble functions, if input values are <=0xf
    unsigned i; 
    for(m = i = 0; i < n; i++) 
	  if(in[i] > m) m = in[i]; 
    cdfini(in, n, cdf, 0x100); 					   // calculte freq. for static distribution functions	
  }
  switch(id) {  
    case  1:         TMBENCH("",l=rcenc(      in,n,out,r),n);     pr(l,n); TMBENCH2(" 1:rc        o0                         ",l==n?memcpy(cpy,out,n):rcdec(     out,n,cpy,r), n); break;
    case  2:         TMBENCH("",l=rccenc(     in,n,out,r),n);     pr(l,n); TMBENCH2(" 2:rcc       o1                         ",l==n?memcpy(cpy,out,n):rccdec(    out,n,cpy,r), n); break;
    case  3:         TMBENCH("",l=rcc2enc(    in,n,out,r),n);     pr(l,n); TMBENCH2(" 3:rcc2      o2                         ",l==n?memcpy(cpy,out,n):rcc2dec(   out,n,cpy,r), n); break;
    case  4:         TMBENCH("",l=rcxenc(     in,n,out,r),n);     pr(l,n); TMBENCH2(" 4:rcx       o8b /o1 context slide      ",l==n?memcpy(cpy,out,n):rcxdec(    out,n,cpy,r), n); break;
    case  5:         TMBENCH("",l=rcx2enc(    in,n,out,r),n);     pr(l,n); TMBENCH2(" 5:rcx2      o16b/o2 context slide      ",l==n?memcpy(cpy,out,n):rcx2dec(   out,n,cpy,r), n); break;
    case  6:if(z==2){TMBENCH("",l=rcenc16(    in,n,out,r),n);     pr(l,n); TMBENCH2(" 6:rc-16     o0  16-bits                ",l==n?memcpy(cpy,out,n):rcdec16(   out,n,cpy,r), n); break; }
            if(z==4){TMBENCH("",l=rcenc32(    in,n,out,r),n);     pr(l,n); TMBENCH2(" 6:rc-32     o0  32-bits                ",l==n?memcpy(cpy,out,n):rcdec32(   out,n,cpy,r), n); } break;
    case  7:if(z==2){TMBENCH("",l=rccenc16(   in,n,out,r),n);     pr(l,n); TMBENCH2(" 7:rcc-16    o1  16-bits                ",l==n?memcpy(cpy,out,n):rccdec16(  out,n,cpy,r), n); break; }
            if(z==4){TMBENCH("",l=rccenc32(   in,n,out,r),n);     pr(l,n); TMBENCH2(" 7:rcc-32    o7bs  32-bits o[24-30]     ",l==n?memcpy(cpy,out,n):rccdec32(  out,n,cpy,r), n); } break;
    case  8:if(z==4){TMBENCH("",l=rcc2enc32(  in,n,out,r),n);     pr(l,n); TMBENCH2(" 8:rcc2-32   o11bs 32-bits o[20-30]     ",l==n?memcpy(cpy,out,n):rcc2dec32( out,n,cpy,r), n); } break;
    case  9:         TMBENCH("",l=rcmenc(     in,n,out,r),n);     pr(l,n); TMBENCH2(" 9:rcms      o1 mixer/sse               ",l==n?memcpy(cpy,out,n):rcmdec(    out,n,cpy,r), n); break;
    case 10:         TMBENCH("",l=rcm2enc(    in,n,out,r),n);     pr(l,n); TMBENCH2("10:rcm2      o2 mixer/sse               ",l==n?memcpy(cpy,out,n):rcm2dec(  out,n,cpy,r), n); break;
    case 11:         TMBENCH("",l=rcmrenc(    in,n,out,r),n);     pr(l,n); TMBENCH2("11:rcmr      o2 8b mixer/sse run        ",l==n?memcpy(cpy,out,n):rcmrdec(  out,n,cpy,r), n); break;
    case 12:         TMBENCH("",l=rcrleenc(   in,n,out,r),n);     pr(l,n); TMBENCH2("12:rcrle     RLE o0                     ",l==n?memcpy(cpy,out,n):rcrledec(  out,n,cpy,r), n); break;
    case 13:if(z==2){TMBENCH("",l=rcrleenc16( in,n,out,r),n);     pr(l,n); TMBENCH2("13:rcrle-16  RLE o0                     ",l==n?memcpy(cpy,out,n):rcrledec16( out,n,cpy,r), n);} break;
    case 14:         TMBENCH("",l=rcrle1enc(  in,n,out,r),n);     pr(l,n); TMBENCH2("14:rcrle1    RLE o1                     ",l==n?memcpy(cpy,out,n):rcrle1dec( out,n,cpy,r), n); break;
    case 15:if(z==2){TMBENCH("",l=rcrle1enc16(in,n,out,r),n);     pr(l,n); TMBENCH2("15:rcrle1-16 RLE o1                     ",l==n?memcpy(cpy,out,n):rcrle1dec16( out,n,cpy,r), n);} break;
    case 16:         TMBENCH("",l=rcu3enc(    in,n,out,r),n);     pr(l,n); TMBENCH2("16:rcu3      varint8 3/5/8 bits         ",l==n?memcpy(cpy,out,n):rcu3dec(   out,n,cpy,r), n); break;
    case 17:         TMBENCH("",l=rcqlfcenc(  in,n,out,r),n);     pr(l,n); TMBENCH2("17:rcqlfc    QLFC                       ",l==n?memcpy(cpy,out,n):rcqlfcdec( out,n,cpy,r), n); break;
    case 18:         TMBENCH("",l=becenc8(    in,n,out),n);       pr(l,n); TMBENCH2("18:bec       Bit EC                     ",l==n?memcpy(cpy,out,n):becdec8(out,n,cpy       ), n); break;
    case 19:if(z==2){TMBENCH("",l=becenc16(   in,n,out),n);       pr(l,n); TMBENCH2("19:bec-16    Bit EC                     ",l==n?memcpy(cpy,out,n):becdec16(out,n,cpy       ), n);} break;
	  #ifdef _BWT
    case 20:         TMBENCH("",l=rcbwtenc(   in,n,out,r,flag),n);pr(l,n); TMBENCH2("20:bwt                                  ",l==n?memcpy(cpy,out,n):rcbwtdec(  out,n,cpy,r), n); break;
      #endif
    case 26:if(z==1){TMBENCH("",l=rcgenc8(    in,n,out,r),n);     pr(l,n); TMBENCH2("26:rcg-8     gamma                      ",l==n?memcpy(cpy,out,n):rcgdec8(   out,n,cpy,r), n); break;}
            if(z==2){TMBENCH("",l=rcgenc16(   in,n,out,r),n);     pr(l,n); TMBENCH2("26:rcg-16    gamma                      ",l==n?memcpy(cpy,out,n):rcgdec16(  out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcgenc32(   in,n,out,r),n);     pr(l,n); TMBENCH2("26:rcg-32    gamma                      ",l==n?memcpy(cpy,out,n):rcgdec32(  out,n,cpy,r), n);} break;
    case 27:if(z==1){TMBENCH("",l=rcgzenc8(   in,n,out,r),n);     pr(l,n); TMBENCH2("27:rcgz-8    gamma zigzag               ",l==n?memcpy(cpy,out,n):rcgzdec8(  out,n,cpy,r), n); break;}
            if(z==2){TMBENCH("",l=rcgzenc16(  in,n,out,r),n);     pr(l,n); TMBENCH2("27:rcgz-16   gamma zigzag               ",l==n?memcpy(cpy,out,n):rcgzdec16( out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcgzenc32(  in,n,out,r),n);     pr(l,n); TMBENCH2("27:rcgz-32   gamma zigzag               ",l==n?memcpy(cpy,out,n):rcgzdec32( out,n,cpy,r), n);} break;

    case 28:if(z==1){TMBENCH("",l=rcrenc8(    in,n,out,r),n);     pr(l,n); TMBENCH2("28:rcr-8     rice                       ",l==n?memcpy(cpy,out,n):rcrdec8(   out,n,cpy,r), n); break;}
            if(z==2){TMBENCH("",l=rcrenc16(   in,n,out,r),n);     pr(l,n); TMBENCH2("28:rcr-16    rice                       ",l==n?memcpy(cpy,out,n):rcrdec16(  out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcrenc32(   in,n,out,r),n);     pr(l,n); TMBENCH2("28:rcr-32    rice                       ",l==n?memcpy(cpy,out,n):rcrdec32(  out,n,cpy,r), n); } break;
    case 29:if(z==1){TMBENCH("",l=rcrzenc8(   in,n,out,r),n);     pr(l,n); TMBENCH2("29:rcrz-8    rice zigzag                ",l==n?memcpy(cpy,out,n):rcrzdec8(  out,n,cpy,r), n); break;}
            if(z==2){TMBENCH("",l=rcrzenc16(  in,n,out,r),n);     pr(l,n); TMBENCH2("29:rcr-16    rice zigzag                ",l==n?memcpy(cpy,out,n):rcrzdec16( out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcrzenc32(  in,n,out,r),n);     pr(l,n); TMBENCH2("29:rcr-32    rice zigzag                ",l==n?memcpy(cpy,out,n):rcrzdec32( out,n,cpy,r), n);} break;
	  
    case 30:if(z==2){TMBENCH("",l=rcvenc16(   in,n,out,r),n);     pr(l,n); TMBENCH2("30:rcv-16    Turbo vlc8                 ",l==n?memcpy(cpy,out,n):rcvdec16( out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcvenc32(   in,n,out,r),n);     pr(l,n); TMBENCH2("30:rcv-32    Turbo vlc8                 ",l==n?memcpy(cpy,out,n):rcvdec32( out,n,cpy,r), n);} break;
    case 31:if(z==4){TMBENCH("",l=rcveenc32(  in,n,out,r),n);     pr(l,n); TMBENCH2("31:rcve-32   Turbo vlc12                ",l==n?memcpy(cpy,out,n):rcvedec32( out,n,cpy,r), n); } break;
    case 32:if(z==2){TMBENCH("",l=rcvzenc16(  in,n,out,r),n);     pr(l,n); TMBENCH2("32:rcvz-16   Turbo vlc8 zigzag          ",l==n?memcpy(cpy,out,n):rcvzdec16( out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcvzenc32(  in,n,out,r),n);     pr(l,n); TMBENCH2("32:rcvz-32   Turbo vlc8 zigzag          ",l==n?memcpy(cpy,out,n):rcvzdec32( out,n,cpy,r), n);} break;
    case 33:if(z==4){TMBENCH("",l=rcvezenc32( in,n,out,r),n);     pr(l,n); TMBENCH2("33:rcvez-32  Turbo vlc12 zigzag         ",l==n?memcpy(cpy,out,n):rcvezdec32( out,n,cpy,r), n);} break;
    case 34:if(z==2){TMBENCH("",l=rcvgenc16(  in,n,out,r),n);     pr(l,n); TMBENCH2("34:rcvg-16   Turbo vlc8 gamma           ",l==n?memcpy(cpy,out,n):rcvgdec16( out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcvgenc32(  in,n,out,r),n);     pr(l,n); TMBENCH2("34:rcvg-32   Turbo vlc8 gamma           ",l==n?memcpy(cpy,out,n):rcvgdec32( out,n,cpy,r), n); } break;
    case 35:if(z==2){TMBENCH("",l=rcvgzenc16( in,n,out,r),n);     pr(l,n); TMBENCH2("35:rcvgz-16  Turbo vlc8 gamma zigzag    ",l==n?memcpy(cpy,out,n):rcvgzdec16( out,n,cpy,r), n); break;}
            if(z==4){TMBENCH("",l=rcvgzenc32( in,n,out,r),n);     pr(l,n); TMBENCH2("35:rcvgz-32  Turbo vlc8 gamma zigzag    ",l==n?memcpy(cpy,out,n):rcvgzdec32( out,n,cpy,r), n);} break;

    case 40:if(m<16){TMBENCH("",l=rc4csenc(   in,n,out),n);       pr(l,n); TMBENCH2("40:rc4cs     bitwise nibble static      ",l==n?memcpy(cpy,out,n):rc4csdec(  out,n,cpy), n); } break;            // Static
    case 41:if(m<16){TMBENCH("",l=rc4senc(    in,n,out),n);       pr(l,n); TMBENCH2("41:rc4s      bitwise nibble adaptive    ",l==n?memcpy(cpy,out,n):rc4sdec(   out,n,cpy), n); } break;           // Adaptive
    case 42:         TMBENCH("",l=rccdfsenc( in,n,out,cdf,m+1),n);pr(l,n); TMBENCH2("42:cdfsb     static/decode search       ",l==n?memcpy(cpy,out,n):(m<16?rccdfsldec( out,n,cpy, cdf, m+1):rccdfsbdec( out,n,cpy, cdf, m+1)), n); break; // static
    case 43:         TMBENCH("",l=rccdfsenc( in,n,out,cdf,m+1),n);pr(l,n); TMBENCH2("43:cdfsv     static/decode division     ",l==n?memcpy(cpy,out,n):(m<16?rccdfsvldec(out,n,cpy, cdf, m+1):rccdfsvbdec(out,n,cpy, cdf, m+1)), n); break;
    case 44:         TMBENCH("",l=rccdfsmenc(in,n,out,cdf,m+1),n);pr(l,n); TMBENCH2("44:cdfsm     static/decode division lut ",l==n?memcpy(cpy,out,n):(m<16?rccdfsmldec(out,n,cpy, cdf, m+1):rccdfsmbdec(out,n,cpy, cdf, m+1)), n); break;
    case 45:         TMBENCH("",l=rccdfs2enc(in,n,out,cdf,m+1),n);pr(l,n); TMBENCH2("45:cdfsb     static interlv/dec. search ",l==n?memcpy(cpy,out,n):(m<16?rccdfsl2dec(out,n,cpy, cdf, m+1):rccdfsb2dec(out,n,cpy, cdf, m+1)), n); break; // static
    case 46:if(m<16){TMBENCH("",l=rccdf4enc(  in,n,out),n);       pr(l,n); TMBENCH2("46:cdf4      nibble adaptive            ",l==n?memcpy(cpy,out,n):rccdf4dec( out,n,cpy), n); }
            else {   TMBENCH("",l=rccdfenc(   in,n,out),n);       pr(l,n); TMBENCH2("46:cdf       byte   adaptive            ",l==n?memcpy(cpy,out,n):rccdfdec(  out,n,cpy), n); } break;
    case 47:if(m<16){TMBENCH("",l=rccdf4ienc( in,n,out),n);       pr(l,n); TMBENCH2("47:cdf4i     nibble adaptive interleaved",l==n?memcpy(cpy,out,n):rccdf4idec(out,n,cpy), n); }
            else {   TMBENCH("",l=rccdfienc(  in,n,out),n);       pr(l,n); TMBENCH2("47:cdfi      byte   adaptive interleaved",l==n?memcpy(cpy,out,n):rccdfidec( out,n,cpy), n); } break;
    case 48:         TMBENCH("",l=rccdfenc8(  in,n,out),n);       pr(l,n); TMBENCH2("48:cdf-8     vnibble                    ",l==n?memcpy(cpy,out,n):rccdfdec8( out,n,cpy), n); break;
    case 49:         TMBENCH("",l=rccdfienc8( in,n,out),n);       pr(l,n); TMBENCH2("49:cdfi-8    vnibble interleaved        ",l==n?memcpy(cpy,out,n):rccdfidec8(out,n,cpy), n); break; 
    case 50:if(z==2){TMBENCH("",l=rccdfuenc16(in,n,out),n);       pr(l,n); TMBENCH2("50:cdf-16    Turbo vlc6                 ",l==n?memcpy(cpy,out,n):rccdfudec16(out,n,cpy), n); break; }
            if(z==4){TMBENCH("",l=rccdfuenc32(in,n,out),n);       pr(l,n); TMBENCH2("50:cdf-32    Turbo vlc6                 ",l==n?memcpy(cpy,out,n):rccdfudec32(out,n,cpy), n);} break;
    case 51:if(z==2){TMBENCH("",l=rccdfvenc16(in,n,out),n);       pr(l,n); TMBENCH2("51:cdf-16    Turbo vlc7                 ",l==n?memcpy(cpy,out,n):rccdfvdec16(out,n,cpy), n); break;}
            if(z==4){TMBENCH("",l=rccdfvenc32(in,n,out),n);       pr(l,n); TMBENCH2("51:cdf-32    Turbo vlc7                 ",l==n?memcpy(cpy,out,n):rccdfvdec32(out,n,cpy), n);} break;
    case 52:if(z==2){TMBENCH("",l=rccdfvzenc16(in,n,out),n);      pr(l,n); TMBENCH2("52:cdf-16    Turbo vlc7 zigzag          ",l==n?memcpy(cpy,out,n):rccdfvzdec16(out,n,cpy), n); break;}
            if(z==4){TMBENCH("",l=rccdfvzenc32(in,n,out),n);      pr(l,n); TMBENCH2("52:cdf-32    Turbo vlc7 zigzag          ",l==n?memcpy(cpy,out,n):rccdfvzdec32(out,n,cpy), n);} break;			
    #define ID_LAST 59
    #define ID_MEMCPY 59 
    case ID_MEMCPY:  TMBENCH("", memcpy(out,in,n) ,n); pr(n,n);            TMBENCH2("59:memcpy                               ", memcpy(cpy,out,n), n);  l=n; break;
    case 60:{ unsigned *sa=malloc((n+1)*sizeof(sa[0]));if(!sa) die("malloc of '' failed\n", n*4);
	  #ifdef _BWTDIV
	                 TMBENCH("",l=divbwt(in,out,sa,n),n);         pr(n,n); TMBENCH2("60:bwt libdivsufsort                    ",obwt_unbwt_biPSIv2(out,cpy,sa,n,l), n); free(sa); } break; //ctou32(out)=l; fwrite(out,1,n+4,fdbg); 
      #elif defined(_LIBSAIS)
	                 TMBENCH("",l=libsais_bwt(in,out,sa,n,0,0),n);pr(n,n); TMBENCH2("60:bwt libsais                          ",libsais_unbwt(out,cpy,sa,n,0,l), n); free(sa);} break;
	  #endif
    case 61:         TMBENCH("",l=utf8enc(  in,n,out, flag),n);   pr(l,n); TMBENCH2("61:utf8 preprocessor                    ",l==n?memcpy(cpy,out,n):utf8dec(   out,n,cpy), n); break;
    case 62:         TMBENCH("",l=lzpenc(  in,n,out,lenmin),n);   pr(l,n); TMBENCH2("62:lzp                                  ",l==n?memcpy(cpy,out,n):lzpdec(    out,n,cpy,lenmin), n); break;
      #ifdef _EXT
    #include "xturborc.c"
      #endif
    default: return 0;
  }
  if(l) { memcheck(in,n,cpy); }
  return l;
} 
#endif

static void usage(char *pgm) {
  fprintf(stderr, "\nTurboRC 22.01 Copyright (c) 2018-2022 Powturbo %s\n", __DATE__);
  fprintf(stderr, "\n Usage: %s <options> <infile1> <outfile>\n", pgm);
  fprintf(stderr, "<options>\n");
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
  fprintf(stderr, "   BWT+rc         : 20 options: l#  #:0:store, 1:bit ec, 2:simple rc, 3:dual rc\n");
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
  exit(1);
} 

//----------------------------- File compression header serialization ----------------------------------------------------
#pragma pack(1)
typedef struct hd {											// main header
  unsigned short magic, _bsize;
  unsigned char  codec, lev, prm1, prm2, prdid;
} _PACKED hd_t;
typedef struct chdb {									    // block header
  unsigned bsize, inlen, clen;
} _PACKED hdb_t;
#pragma pack() 

int hdwr(hd_t *hd, FILE *fo) {
  unsigned folen = 0 ;
  unsigned u32 = (hd->_bsize-1) << 20 | hd->codec << 12 | hd->magic;            //12+8+12=32 bits
  if(fwrite(&u32, 1, 4, fo) != 4) return -E_FWR;             folen  = 4;              // blocksize
  unsigned short u16 = hd->lev<<10 | hd->prm2<<6 | hd->prm1<<2 | (hd->prdid-1); // 4+4+4+2=14 bits
  if(fwrite(&u16, 1, 2, fo) != 2) return -E_FWR;             folen += 2;   
  return folen;	
}

int hdrd(hd_t *hd, FILE *fi) {
  unsigned filen, u32;
  unsigned short u16;
  if(fread(&u32, 1, 4, fi) != 4) return -E_FRD;                  filen  = 4;
  if((u32&0xfffu) != MAGIC) return -E_MAG;
  if((hd->codec  = (char)(u32>>12)) > CODEC_MAX) -E_CODEC;
  if((hd->_bsize = (u32>>20)+1) > BLKMAX) return -E_CORR;
  if(fread(&u16, 1, 2, fi) != 2) return -E_FRD;                  filen += 2;
  hd->prdid =  (u16&3)+1;
  hd->prm1  =  (u16>> 2)&0xf;
  hd->prm2  =  (u16>> 6)&0xf;
  hd->lev   =  (u16>>10)&0xf; 
  return filen;
}

int hdbwr(hdb_t *hdb, FILE *fo) {
  unsigned folen, h = hdb->clen >= (1<<30), u32 = hdb->clen << 2 | h << 1 | (hdb->inlen < hdb->bsize);                // last block?
  if(fwrite(&u32, 1, 4, fo) != 4) return -E_FWR;     folen  = 4; printf("clen=%u ", hdb->clen);
  if(h) {
    unsigned short u16 = hdb->clen >> 30;
    if(fwrite(&u16, 1, 2, fo) != 2) return -E_FWR;   folen += 2;
  }
  if(hdb->inlen < hdb->bsize) 									   // length of last block < block size
    if(fwrite(&hdb->inlen, 1, 4, fo) != 4) return -E_FWR;   folen +=4;
  return folen;
}

unsigned hdbrd(hdb_t *hdb, FILE *fi) {
  unsigned filen, u32, h;
  unsigned short u16;
  if(fread(&u32, 1, 4, fi) != 4) return -E_FRD;          filen  = 4;
  hdb->clen = u32>>2;                                                   printf("clen=%u ", hdb->clen);
  if(u32&2) {
	unsigned short u16;
	if(fread(&u16, 1, 2, fi) != 2) return -E_FRD;        filen += 2;
	hdb->clen |= u16 << 30;
  }
  if(u32&1) {                                                          // last block
    if(fread(&hdb->inlen, 1, 4, fi) != 4) return -E_FRD; filen  += 4;
  }	  
  return filen; 	
}
//---------------------------------------------- main : benchmark + file compression ----------------------------------------------
int main(int argc, char* argv[]) {
  unsigned _bsize = 1536, prdid = RC_PRD_S;
  int      xstdout=0, xstdin=0, decomp=0, codec=0, dobench=0, cmp=1, c, digit_optind=0, decs=0, divs=0, skiph=0, isize=4, dfmt=0, mdelta=0, kid=0, osize=1;
  char     *scmd = NULL, prids[8]="s", *keysep = NULL;													//fdbg = fopen("test.dat", "wb"); if(!fdbg) perror("fopen failed");
  
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
  
  for(;;) {
    int this_option_optind = optind ? optind : 1, optind = 0;
    static struct option long_options[] = {
      { "help",     0, 0, 'h'},
      { 0,          0, 0, 0}
    }; 
    if((c = getopt_long(argc, argv, "0:1:2:3:4:5:6:7:8:9:b:cde:fhk:l:m:nop:r:t:v:x:XzF:H:I:J:K:B:O:P:Q:S:T:", long_options, &optind)) == -1) break;
    switch(c) {
      case 0:
        printf("Option %s", long_options[optind].name);
        if(optarg) printf(" with arg %s", optarg);  printf ("\n");
        break;
		
	  case 'f': xprep8=1; break;	  
      case 'F': { char *s = optarg;    // Input format
	    switch(*s) {
          case 'c': dfmt = T_CHAR; s++; break;
          case 't': dfmt = T_TXT; if(*++s > '0' && *s <= '9') { kid = *s++ - '0'; if(*s > '0' && *s <= '9') kid = kid*10 + (*s++ - '0'); } break;
          case 'e': dfmt = T_TST; s++; break;
          case 'r': dfmt = T_RAW; s++; break; // raw default
		}
		switch(*s) {
          case 'b': isize =  1, s++; break;					// 1 byte
          case 's': isize =  2, s++; break;					// 2 bytes
          case 'u': isize =  4, s++; break;					// 4 bytes
          case 'l': isize =  8, s++; break;					// 8 bytes
          case 'f': isize = -4, s++; break;					// float : 4 bytes
          case 'd': isize = -8, s++; break;					// double: 8 bytes
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
      case 'H': skiph = atoi(optarg); break;
      case 'K': { kid = atoi(optarg); if(!keysep) keysep = ",;\t"; } break;
      case 'k': keysep = optarg; break;
	  case 'T': itmax = atoi(optarg); break;
      case 'c': cmp++;     break;
      case 'd': decomp++;  break;
      case 'n': xnibble++; break;
      case 'o': xstdout++; break;
      case 'v': verbose = atoi(optarg); break;
	  
      case 'X': bwtx++;    break;
      case 'S': xsort = atoi(optarg); break;
      case 'z': forcelzp = BWT_LZP; break;
	  case 'P': prdid = atoi(optarg); if(prdid<1 || prdid>RC_PRD_LAST) prdid=RC_PRD_LAST; break;
	  case 'p': { char *p = optarg; strncpy(prids, p, 2); prids[2]=0;
	              if(p[0]=='s') {
					     if(!p[1])       prdid = 1; 
					else if(p[1] == 's') prdid = 2; 
					else if(p[1] == 'f') prdid = 3;
			      }          
	            } break;
	  case 'r': { char *p = optarg; if(*p >= '0' && *p <= '9') { prm1 = p[0]-'0'; prm2 = p[1]-'0'; } if(prm1>9) prm1=9; if(prm2>9) prm2=9; } break;
	  case 't': xtpbyte = atoi(optarg); if(xtpbyte) { if(xtpbyte < 1) xtpbyte = 1;else if(xtpbyte > 16) xtpbyte = 16; } break; 
      case 'b': _bsize = atoi(optarg); if(_bsize<1) _bsize=1; if(_bsize > BLKMAX) _bsize = BLKMAX; break;
      case 'e': scmd = optarg; dobench++; break;
      case 'I': if((tm_Rep  = atoi(optarg))<=0) tm_rep =tm_Rep =1; break;
      case 'J': if((tm_Rep2 = atoi(optarg))<=0) tm_rep =tm_Rep2=1; break;
	  case 'l': lev = atoi(optarg); if(lev>9) lev=9; break;
      case 'm': lenmin = atoi(optarg); if(lenmin && lenmin < 16) lenmin = 16; if(lenmin > 256) lenmin = 256; break;
	    #ifndef NO_BENCH
	  case 'x': { int m = atoi(optarg); if(m<4) m=4;else if(m>16) m=16; mbcset(m); /*set context bits*/} break;
	    #endif
      case '0':case '1':case '2':case '3': case '4':case '5':case '6':case '7':case '8':case '9': {
        unsigned l = atoi(optarg); decomp = 0;
        if(l >= 0 && l <= 9) {
          codec = (c-'0')*10 + l;                                  //printf("codec=%d\n", codec);fflush(stdout);
          if(codec>=0 && codec<=99) break; 
        }
      }
      case 'h':
      default: 
        usage(argv[0]);
        exit(0); 
    }
  } 																							
  #define ERR(e) do { rc = e; printf("line=%d ", __LINE__); goto err; } while(0)
  size_t bsize = _bsize * Mb;
  int  rc = 0, inlen;
  unsigned char *in = NULL, *out = NULL, *cpy = NULL; 
    #ifdef _SF
  if(prdid == RC_PRD_SF) { printf("fsm"); if(prm1<0) prm1=1;if(prm1>9) prm1=9; fsm_init(prm1); }
    #endif
    #ifndef NO_BENCH
  if(dobench) { //---------------------------------- Benchmark -----------------------------------------------------
    char _scmd[33];
    int  fno, nblk = 0, nid = 0, cminid, cminl = (unsigned)-1; 
    sprintf(_scmd, "1-%d", ID_MEMCPY);                          			if(verbose>1) printf("BENCHMARK ARGS=%d,%d,%d\n", fno, optind, argc);
    printf("   E MB/s    size     ratio%%   D MB/s   function prdid=");
    switch(prdid) {
      case 1: printf("'s(5)'\n"); break;
      case 2: printf("'ss(%u,%u)'\n", prm1, prm2); break;
      case 3: printf("'sf(%u)'\n", prm1); break;
    }
    uint64_t clen = 0;	
    for(fno = optind; fno < argc; fno++) {
      uint64_t filen;
      int      n,i;    
      char     *finame = argv[fno];                                     
      FILE     *fi = fopen(finame, "rb");                           		if(!fi ) { perror(finame); continue; }   if(verbose>1) printf("'%s'\n", finame);

      fseek(fi, 0, SEEK_END); 
      filen = ftell(fi); 
      fseek(fi, 0, SEEK_SET);
    
      size_t b = (filen < bsize && !dfmt)?filen:bsize, tpbyte=0;			if(verbose>1) printf("bsize=%zu b=%zu ", bsize, b);
      
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
	    } else tpbyte = xtpbyte;
	  }
	  for(;;) {															
        n = dfmt?befgen(fi, in, b, dfmt, isize, osize, kid, skiph, decs, divs, keysep, mdelta):fread(in, 1, b, fi); 
		if(n<=0) break; 															if(verbose>1) printf("read=%u\n", n);
	    nblk++;
        char *p = (scmd && (scmd[0] != '0' || scmd[1]))?scmd:_scmd;	
		  
		if(tid) { 
		  uint64_t l; 
          if(l = bench(in, n, out, cpy, tid, prdid, osize)) printf("\t%s\n", finame);  
		  clen += l;
		} else
        do { 
          int id = strtoul(p, &p, 10),idx = id, i;
		  uint64_t l; 
          if(id >= 0) {    
            while(isspace(*p)) p++; if(*p == '-') { if((idx = strtoul(p+1, &p, 10)) < id) idx = id; if(idx > ID_LAST) idx = ID_LAST; } //printf("ID=%d,%d ", id, idx);
            for(i = id; i <= idx; i++) {
              if(l = bench(in, n, out, cpy, i, prdid, osize)) printf("\t%s\n", finame);  
			  clen += l;
			  if(l && l < cminl) cminl = l, cminid = i;
			  nid++;
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
    else
	  if(nblk == 1 && nid > 4) printf("Best method = %d\n", cminid);														//if(fdbg) fclose(fdbg); 
	exit(0);   
  }
    #endif
  //---------------------------------- File Compression/Decompression -----------------------------------------------------
  if(!decomp && prdid != 1 && prdid !=2) { 
	fprintf(stderr,"Only predictors 's' and 'ss' are supported for file compression\n");
	exit(0);
  }
  if(argc <= optind) xstdin++;
  unsigned long long filen=0,folen=0;
  char               *finame = xstdin ?"stdin":argv[optind], *foname, _foname[1027];  if(verbose>1) printf("'%s'\n", finame);
  
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
  if(!decomp) { 
    hd_t hd; hd.magic = MAGIC; hd._bsize = _bsize; hd.codec = codec; hd.lev = lev; hd.prdid = prdid; hd.prm1 = prm1; hd.prm2 = prm2;
    if((rc = hdwr(&hd, fo)) < 0) ERR(-rc); folen = rc;
    in  = vmalloc(bsize+1024);
    out = vmalloc(bsize+1024); if(!in || !out) ERR(E_MEM); 
    unsigned clen;
    while((inlen = fread(in, 1, bsize, fi)) > 0) {        filen += inlen;
      switch(codec) {		  
        case  0: clen = inlen; memcpy(out, in, inlen);    break;
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
	      #ifdef _BWT
        case 20: clen = rcbwtenc( in, inlen, out, prdid, bwtflag(osize)); break;
	      #endif		
		case 21: clen = utf8enc( in, inlen, out, bwtflag(osize)); break; 
        default: ERR(E_CODEC); 
      }
      hdb_t hdb; hdb.inlen = inlen; hdb.bsize = bsize; hdb.clen = clen; if((rc = hdbwr(&hdb, fo)) < 0) ERR(-rc); folen += rc;
      if(fwrite(out, 1, clen, fo) != clen) ERR(E_FWR);          folen += clen;
    }                                                             if(verbose) printf("compress: '%s'  %d->%d\n", finame, filen, folen);                                                                    
  } else 
	#endif
  { // Decompress
	hd_t hd; if((rc = hdrd(&hd, fi)) < 0) ERR(-rc); filen = rc; _bsize = hd._bsize; codec = hd.codec; lev = hd.lev; prdid = hd.prdid; prm1 = hd.prm1; prm2 = hd.prm2; 
    bsize = _bsize * Mb;
    in  = vmalloc(bsize); 
    out = vmalloc(bsize); if(!in || !out) ERR(E_MEM); 
    for(;;) {
	  hdb_t hdb; if((rc = hdbrd(&hdb, fi)) < 0) break;     filen += rc;
	  unsigned outlen = hdb.inlen;
      if(fread(in, 1, hdb.clen, fi) != hdb.clen) ERR(E_FRD);   filen += hdb.clen; // read block
      if(hdb.clen == outlen) memcpy(out, in, outlen);
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
        case 20: rcbwtdec(  in, outlen, out, prdid); break;
          #endif
        case 21: utf8dec(   in, outlen, out);        break;
        default: ERR(E_CODEC); 
      }  
      if(fwrite(out, 1, outlen, fo) != outlen) ERR(E_FWR); folen += outlen;  
      if(hdb.inlen < inlen) break;	  
    }                                                        if(verbose) printf("decompress:'%s' %d->%d\n", foname, filen, folen);
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
