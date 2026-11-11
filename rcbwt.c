/** Copyright (C) powturbo 2013-2026

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
#include <stdlib.h>
#include <string.h>
#include "include/turborc.h"
#include "include_/conf.h"
#include "include_/rcutil.h"
#include "include_/bec.h"
#include "include_/vlcbit.h"

#include "rcutil_.h"
#include "mb_vint.h" 

//------------------------------------------- bwt : with libdivsort or libsais ----------------------------------------------------------
  #ifdef _BWTDIV                                // use libdivsort library
#include "libdivsufsort/include/divsufsort.h"
#include "libdivsufsort/include/unbwt.h"
  #else
#ifdef _OPENMP
#define LIBSAIS_OPENMP
#endif
#include "libsais/include/libsais.h"            // use libsais
#include "libsais/include/libsais16.h"
typedef int32_t saidx_t;
  #endif

  #ifdef BWT_SINGLE
static int xbwts = 0;
  #endif

static inline unsigned calcmod(size_t len) { return len < 2?2:(1u<<__bsr32(len)); }
#define SR 16
#define LZPRLEV 3
#define BWT_BSIZE (2047u<<20)

#ifdef LZPREVERSE   // use reverse lzp 
#define LZPREV(a) 
#define OUT       in
#else
#define LZPREV(a) a
#define OUT       out
#endif
#ifndef _WIN32
#define vmallo(a) malloc(a)
#define vfree(a)  free(a)
#endif

// preprocessing id stored in the low nibble of the flag byte
#define PRE_NONE  0
#define PRE_UTF8  1
#define PRE_FASTA 2
#define PRE_FLZP  3
#define PRE_EXE   4

  #ifndef NCOMP
#define FS (1<<20)
static size_t sample(unsigned char *in, size_t n, unsigned i, unsigned j, unsigned char *out) {
  if(n < (size_t)i*j) { memcpy(out, in, n); return n; }
  size_t segment_size = n / (size_t)i;
  for(unsigned k = 0; k < i; k++)                                   // copy first j consecutive bytes of segment k
    memcpy(out + (size_t)k * j, in + (size_t)k * segment_size, j);
  return (size_t)i*j;
}

static size_t bwtenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned char *pidx, int xbwt16, int threads, int v) {
  unsigned char *op = out, *ip, *in_ = in+inlen, *oq = pidx;                                                 
  size_t iplen, bsize = inlen > BWT_BSIZE?BWT_BSIZE:inlen;             
    #ifndef _LIBSAIS16
  xbwt16 = 0;
    #endif
  saidx_t *sa = (saidx_t *)vmalloc((size_t)(bsize+FS+128)*sizeof(sa[0])); if(!sa) die("bwtenc: malloc error. size=%zu ", (size_t)(bsize+FS+128)*sizeof(sa[0])); 
                                                                                    
  for(ip = in; ip < in_; ip += iplen, op += iplen) {
    iplen = in_- ip > bsize?bsize:in_ - ip;
      #ifdef _BWTDIV
    *oq++ = 0; *(saidx_t *)oq  = divbwt(ip, op, sa, iplen); oq += sizeof(sa[0]);            
      #else
                                                                                      if(v) { printf("bwt16=%u omp ", xbwt16>0);fflush(stdout); }
    unsigned idxs[256], iplen_ = xbwt16?(iplen/2):iplen;
    unsigned mod = calcmod(iplen_/SR), 
        #ifdef BWT_SINGLE
             idxsn = xbwts?1:(iplen_-1)/mod + 1;                     
        #else
             idxsn = (iplen_-1)/mod + 1;
        #endif
        #ifndef _OPENMP
                                                                                       if(v) printf("disabled ");
        #endif
        #ifdef _LIBSAIS16
    if(xbwt16) {                                                                       if(v) { printf("-"); fflush(stdout); }
          #ifdef _OPENMP
            #ifdef BWT_SINGLE
      if(xbwts) { idxs[0] = threads<=1?libsais16_bwt((const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0): libsais16_bwt_omp((const uint16_t *)ip, (uint16_t *)op, sa, iplen_, 0, 0, threads); } else 
            #endif
      { threads<=1?libsais16_bwt_aux(    (const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0, mod, idxs): 
                   libsais16_bwt_aux_omp((const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0, mod, idxs, threads);
      }
          #else
            #ifdef BWT_SINGLE
      if(xbwts) idxs[0] = libsais16_bwt(  (const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0);  else
            #endif
                          libsais16_bwt_aux((const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0, mod, idxs); 
          #endif     
                                                                                       if(v) { printf("+"); fflush(stdout); }
      if(iplen & 1) op[iplen-1] = ip[iplen-1];
    } else
        #endif
    {
        #ifdef _OPENMP                                                                                     
          #ifdef BWT_SINGLE
      if(xbwts) { idxs[0] = threads<=1?libsais_bwt(ip, op, sa, iplen, 0, 0) : libsais_bwt_omp(ip, op, sa, iplen, 0, 0, threads); } else
          #endif 
      { threads<=1?libsais_bwt_aux(ip, op, sa, iplen, 0, 0, mod, idxs) : libsais_bwt_aux_omp(ip, op, sa, iplen, 0, 0, mod, idxs, threads); }                     
        #else
          #ifdef BWT_SINGLE
      if(xbwts) idxs[0] = libsais_bwt(ip, op, sa, iplen, 0, 0); else
          #endif
                          libsais_bwt_aux(ip, op, sa, iplen, 0, 0, mod, idxs);                          
        #endif
    }
    *oq++ = idxsn - 1;
    memcpy(oq, idxs, idxsn*sizeof(idxs[0])); oq += idxsn*sizeof(idxs[0]);
      #endif
  }
  vfree(sa);
  return oq - pidx;
}

static unsigned char hbits[] = { 16,16,16,16,16,18,18,18,20,22 };
#define RC_HBITS(lev) (inlen>(1<<24)?hbits[lev]:LZPHBITS)

size_t rcbwtenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned lev, unsigned threads, unsigned opts) {
  size_t        iplen  = inlen;
  unsigned      lenmin = opts & 0xff, xbwt16 = (opts & BWT_BWT16)?0x40:0, v = opts & BWT_VERBOSE, nutf8 = (opts & BWT_UTF8_NO)?1:0,
                lzprev = (lev>=LZPRLEV && !(opts & BWT_LZP_NREV))?0x20:0, forcelzp = (opts & BWT_LZP_FORCE)?1:0, pre = PRE_NONE; 
  unsigned char *op    = out, *out_ = out+inlen, *_bwt = vmalloc(inlen+(1<<20)), *bwt = _bwt, *ip = in;
  if(lev > 9) lev = 9;
  if(!lev) lenmin = 0;                                                               if(!bwt) { op = out_; goto e; }
  if(!threads) threads = 1;
  if(lenmin == 1) { int nrev;                                                        
    size_t slen = sample(in, inlen, 16, 16*1024, out);     
    lenmin = histopt(out, slen, lev>=8, &nrev);
    if(!(opts & BWT_LZP_NREV)) lzprev = nrev?0:0x20;
  } 
  unsigned flag = xbwt16 | lzprev;
                                                                                     if(v) { printf("\nlev=%u len(MB)=%zu nutf8=%d rev=%d threads=%d ", lev, inlen/(1<<20), nutf8, lzprev, threads); fflush(stdout); }
  if(lenmin) {                                                                       if(v) { printf("lenmin=%u ", lenmin); fflush(stdout); }
    switch(lenmin) {
        #ifdef _FASTA
      case 2: iplen = fastaenc(in, inlen, bwt, inlen); ip = bwt; pre = PRE_FASTA;    if(v) { printf("Fasta:%zu->%zu ", inlen, iplen); fflush(stdout); } 
        break;
      case 3: { size_t flen = fastaenc(in, inlen, out, inlen);                      // [4 bytes fasta length][lzp data]
        iplen = lzpenc(out, flen, bwt+4, 12, LZPHBITS) + 4;
        ctou32(bwt) = flen; ip = bwt; pre = PRE_FLZP;                                if(v) { printf("Fasta+Lzp:%zu->%zu ", inlen, iplen); fflush(stdout); }
      } break;
        #endif
        #ifdef _EXE
      case 4: if(exeenc(in, inlen, bwt)) { ip = bwt; pre = PRE_EXE; }                if(v) { printf("exe:%d ", pre == PRE_EXE); fflush(stdout); }
        break;
        #endif  
      default: 
        if(!nutf8) { 
          size_t l = utf8enc(in, inlen, bwt, opts);                                  // (size_t)-1 on failure -> not < inlen
          if(l < inlen) { ip = bwt; iplen = l; pre = PRE_UTF8; }                     if(v) { if(pre != PRE_UTF8) printf("NoUTF8 "); else printf("UTF8:%zu->%zu ", inlen, iplen); fflush(stdout); }
        }
    }
    if(pre == PRE_NONE && lenmin >= LZPLENMIN) {                                     if(v) { printf("Lzp: minlen=%d ", lenmin); fflush(stdout); } 
      if(lzprev) { memcpy(out, in, inlen); memrev(out, inlen); }
      iplen = lzpenc(lzprev?OUT:in, inlen, bwt, lenmin, RC_HBITS(lev));              if(v) { printf("Lzp=%zu->%zu=%.2f%% ", inlen, iplen, (double)iplen*100.0/inlen);fflush(stdout); }
      if(iplen >= inlen || (iplen+(inlen>>7)+256 > inlen && !forcelzp)) {            if(v) { printf("Not enough saving ");fflush(stdout); }
        ip = in; iplen = inlen; 
      } else { ip = bwt; flag |= 0x80; if(lzprev) memrev(ip, iplen); }
    }
  } else                                                                             if(v) { printf("No preprocessing. Flag=%x ", flag); fflush(stdout); }
  flag |= pre;

  *op++ = flag;                                    
  if(flag & 0x8f) { /*ctou32(op) = iplen;*/ memcpy(op, &iplen, 5); op += 5; }                                   // store bwt length whenever preprocessing changed the data
  op += bwtenc(ip, iplen, bwt, op, xbwt16, threads, v);
  switch(lev) {
    case  0: memcpy(op, bwt, iplen); op += iplen; vfree(_bwt); return op - out;
    case  3: op += xbwt16?becenc16((uint16_t *)bwt, iplen, op):    becenc8( bwt, iplen, op);       break;
    case  4: op += xbwt16 ?       rcrlesenc16( bwt, iplen, op):rcqlfc0senc( bwt, iplen, op);       break;
    case  5: op += xbwt16 ?       rcrle1senc16(bwt, iplen, op):rcqlfc0ssenc(bwt, iplen, op, 4, 7); break;
    case  6: op +=                rcqlfcsenc(  bwt, iplen, op);       break;
    case  7: op +=                rcqlfcssenc( bwt, iplen, op, 4, 7); break;
    case  8: op +=                rcmrrsenc(   bwt, iplen, op);       break;
    case  9: op +=                rcmrrssenc(  bwt, iplen, op, 0, 0); break;
    default: if(op + iplen < out_) memcpy(op, bwt, iplen); op += iplen; break;       // lev 1,2: store bwt
  }                                                                         
  e:if(_bwt) vfree(_bwt);                                                            if(v) { printf("clen=%lld ", (long long)(op-out)); fflush(stdout); }
  if(op >= out_) { memcpy(out, in, inlen); op = out_; }
  return op - out;
}
  #endif

  #ifndef NDECOMP
static size_t bwtdec(unsigned char *in, size_t inlen, unsigned char *out, size_t outlen, unsigned char *pidx, int xbwt16, int threads, int v) {
  unsigned char *ip = in, *iq = pidx, *op = out, *out_ = out+outlen;                                                 
  size_t oplen, bsize = outlen > BWT_BSIZE?BWT_BSIZE:outlen;
    #ifndef _LIBSAIS16
  xbwt16 = 0;
    #endif
  saidx_t *sa = (saidx_t *)vmalloc((outlen+128)*sizeof(sa[0])); if(!sa) die("malloc: not enough memory\n");

  for(op = out; op < out_; op += oplen, ip += oplen) {
    oplen = out_ - op > bsize?bsize : out_ - op;
      #ifdef _BWTDIV
    iq++; saidx_t bwtidx = *(saidx_t *)iq; iq += sizeof(saidx_t);
    obwt_unbwt_biPSIv2(ip, op, sa, oplen, bwtidx);
      #else
    unsigned idxs[256];
    size_t oplen_ = xbwt16?oplen/2:oplen, mod = calcmod(oplen_/SR), idxsn = *iq++ + 1;
    memcpy(idxs, iq, idxsn*sizeof(idxs[0])); iq += idxsn*sizeof(idxs[0]);
        #ifdef _LIBSAIS16
    if(xbwt16) { 
          #ifdef _OPENMP
      threads<=1?libsais16_unbwt_aux(    (uint16_t *)ip, (uint16_t *)op, sa, oplen_, 0, mod, idxs):
                 libsais16_unbwt_aux_omp((uint16_t *)ip, (uint16_t *)op, sa, oplen_, 0, mod, idxs, threads);
          #else
      libsais16_unbwt_aux((uint16_t *)ip, (uint16_t *)op, sa, oplen_, 0, mod, idxs);
          #endif 
      if(oplen & 1) op[oplen-1] = ip[oplen-1];
    } else
        #endif
    {
        #ifdef _OPENMP
          #ifdef BWT_SINGLE
      if(xbwts) { threads<=1?libsais_unbwt(ip, op, sa, oplen, 0, idxs[0]) : libsais_unbwt_omp(ip, op, sa, oplen, 0, idxs[0], threads); } else
          #endif 
      { threads<=1?libsais_unbwt_aux(ip, op, sa, oplen, 0, mod, idxs): libsais_unbwt_aux_omp(ip, op, sa, oplen, 0, mod, idxs, threads); }
        #else
          #ifdef BWT_SINGLE
      if(xbwts) libsais_unbwt(ip, op, sa, oplen, 0, idxs[0]); else
          #endif
      libsais_unbwt_aux(ip, op, sa, oplen, 0, mod, idxs);
        #endif
    }
      #endif
  }
  vfree(sa);
  return op - out;
}

size_t rcbwtdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned threads) {
  unsigned char *ip   = in, *iq;
  unsigned      flag  = *ip++, lzp = flag & 0x80, xbwt16 = flag & 0x40, lzprev = flag & 0x20, pre = flag & 0x0f;
  size_t        oplen = outlen;                                                 // length of the bwt data

  if(lev > 9) lev = 9;
  if(!threads) threads = 1;
  if(flag & 0x8f) { oplen = 0; memcpy(&oplen, ip, 5); ip += 5; }                              // preprocessed length (lzp/utf8/...)
  unsigned char *_bwt = vmalloc(oplen+128), *op = out, *bwt = _bwt; if(!_bwt) die("malloc failed\n");
  if(flag & 0x8f) { op = _bwt; bwt = out; }                                     // unbwt -> _bwt, then postprocess -> out

  iq = ip;                                                                      // skip bwt index header (computed with the BWT length!)
  { size_t bsize = oplen > BWT_BSIZE?BWT_BSIZE:oplen, l, olen;
    for(l = 0; l < oplen; l += olen) {
      olen = oplen - l > bsize?bsize:oplen - l;
      ip += 1 + ((size_t)ip[0]+1)*4;                                            // [idxsn-1][idxsn * 4 bytes]
    }
  }
  switch(lev) {
    case  3: xbwt16?becdec16(    ip, oplen, (uint16_t *)bwt):becdec8(      ip, oplen, bwt); break;
    case  4: xbwt16?rcrlesdec16( ip, oplen,             bwt):rcqlfc0sdec(  ip, oplen, bwt); break;
    case  5: xbwt16?rcrle1sdec16(ip, oplen,             bwt):rcqlfc0ssdec( ip, oplen, bwt, 4, 7); break;
    case  6:        rcqlfcsdec(  ip, oplen,             bwt);       break;
    case  7:        rcqlfcssdec( ip, oplen,             bwt, 4, 7); break; 
    case  8:        rcmrrsdec(   ip, oplen,             bwt);       break;
    case  9:        rcmrrssdec(  ip, oplen,             bwt, 0, 0); break;
    default:        bwt = ip; break;                                            // lev 0,1,2: stored bwt
  }
  bwtdec(bwt, 0, op, oplen, iq, xbwt16, threads, 0);

  switch(pre) {
    case PRE_NONE: break;
    case PRE_UTF8: utf8dec(op, outlen, out); break;
      #ifdef _FASTA
    case PRE_FASTA: fastadec(op, oplen, out, outlen); break;
    case PRE_FLZP: { 
      size_t l = 0; memcpy(&l, op, 5); unsigned char *t = vmalloc(l+128); if(!t) die("malloc failed\n");
      lzpdec(op+5, oplen-5, t, l);
      fastadec(t, l, out, outlen); 
      vfree(t);
    } break;
      #endif
      #ifdef _EXE
    case PRE_EXE: exedec(op, out, outlen); break;
      #endif
  }  
  if(lzp) { 
    if(lzprev) memrev(op, oplen);
    lzpdec(op, oplen, out, outlen);
    if(lzprev) memrev(out, outlen); 
  } 
  vfree(_bwt);
  return outlen;
}
  #endif
