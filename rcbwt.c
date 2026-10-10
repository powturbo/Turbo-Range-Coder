/** Copyright (C) powturbo 2013-2023
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
#include "libsais/include/libsais.h"            // use libsais
#include "libsais/include/libsais16.h"
typedef int32_t saidx_t;
  #endif

#ifdef LZPREVERSE   // use reverse lzp 
#define LZPREV(a) 
#define OUT       in
#else
#define LZPREV(a) a
#define OUT       out
#endif

static int bwtx, forcelzp;
static unsigned calcmod(size_t len) { return 1<<__bsr32(len); }
#define SR 16
#define LZPLEV 3
#define REVLEV 3

  #ifndef NCOMP
int histopt(const char *in, int inlen, int lev);

static int sample(char *in, size_t n, int i, int j, char *out) {
  if(n < j) { memcpy(out, in, n); return n; }
  size_t segment_size = n / (size_t)i;
  for (int k = 0; k < i; k++) {
    size_t start = (size_t)k * segment_size; // copy first j consecutive bytes of segment k
    memcpy(out + (size_t)k * (size_t)j, in + start, (size_t)j);
  }
  return n;
}

size_t rcbwtenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned lev, unsigned threads, unsigned _lenmin) {
  size_t        iplen  = inlen;
  unsigned      lenmin = _lenmin & 0x3ff, xbwt16 = (_lenmin & BWT_BWT16)?0x80:0, verbose = _lenmin & BWT_VERBOSE, nutf8 = _lenmin & BWT_NUTF8; if(!lev) lenmin = 0;
  unsigned char *op    = out, *out_ = out+inlen, *bwt   = vmalloc(inlen+1024), *ip = in;  if(!bwt) { op = out_; goto e; } // inlen + space for bwt indexes idxns  
  //lenmin = histopt(in, inlen, lev>=LZPLEV); 
  if(lenmin == 1) { 
    lenmin = sample(in, inlen, 16, 16*1024, out); 
    lenmin = histopt(out, lenmin, lev>=LZPLEV); 
  }                                                                             if(verbose) { printf("\nlev=%u MB=%zu nutf8=%d threads=%d ", lev, inlen/(1<<20), nutf8?1:0, threads); fflush(stdout); }
  if(lenmin) {                                                                  if(verbose) { printf("lenmin=%u ", lenmin);fflush(stdout); }
    ip = bwt;
    switch(lenmin) {
        #ifdef _FASTA
      case 2  : iplen = fastaenc(in, inlen, ip);                                if(verbose) { printf("GenTR %u->%u ", inlen, iplen); fflush(stdout); } break;
        #endif
      default : if(!nutf8) { iplen = utf8enc(in, inlen, ip, _lenmin);           if(verbose) { if(iplen == inlen) printf("NoUTF8 "); else printf("UTF8:%zu->%zu ", inlen, iplen); fflush(stdout); }} break;                  // try utf8 preprocessing
    }
    if(lenmin < LZPLENMIN || iplen != inlen && iplen != -1) {
      lenmin = lenmin<LZPLENMIN?128-lenmin:127;                                 if(verbose) printf("No Lzp run %d %d ", lenmin, iplen); // lenmin = 127-LM for other preprocessing ids
    } else {
      lenmin = ((lenmin>384?384:lenmin)+3)/4;                                   if(verbose) { printf("Lzp: minlen=%d ", lenmin*4); fflush(stdout); } 
      ip     = bwt;                                                             LZPREV(if(lev>=REVLEV) { memcpy(out, in, inlen); memrev(out, inlen); } );
      iplen  = lzpenc(lev>=REVLEV?OUT:in, inlen, ip, lenmin*4, lev >= LZPLEV?0:LZPHBITS); if(verbose) { printf("Lzp=%zu=%.2f%% ", iplen, (double)iplen*100.0/inlen);fflush(stdout); }
      if(iplen == inlen || iplen+(inlen>>7)+256 > inlen && !forcelzp) {         if(verbose) { printf("Not enough saving ");fflush(stdout); }  //Not enough saving
        ip = in; iplen = inlen; lenmin = 0;
      } else {                                                                  LZPREV(if(lev>=REVLEV) memrev(ip, iplen));  }
    }
  }
  *op++ = xbwt16 | lenmin;
  if(lenmin) ctou32(op) = iplen, op += 4;
    #ifdef _BWTDIV
  *op++ = 0;
  saidx_t   *sa   = (saidx_t *)vmalloc((iplen+2)*sizeof(sa[0]));                if(!sa) { op = out_; goto e; }
  *(saidx_t *)op  = divbwt(ip, bwt, sa, iplen);
              op += sizeof(sa[0]);
    #else
  unsigned idxs[256], iplen_ = xbwt16?(iplen/2):iplen,
           mod = calcmod(iplen_/SR), idxsn = (iplen_-1)/mod + 1;                //printf("bwt idxs=%d ", idxsn); //idxsn = (idxsn/SR)*SR;
  *op++ = idxsn - 1;
  saidx_t *sa = (saidx_t *)vmalloc((iplen_+2+128)*sizeof(sa[0]));               if(!sa) { op = out_; goto e; } if(verbose) { printf("bwt16=%u ", xbwt16>0);fflush(stdout); }
      #ifdef _LIBSAIS16
  if(xbwt16) {                                                                  if(verbose) { printf("-"); fflush(stdout); }
        #ifdef LIBSAIS_OPENMP
                                                                                if(verbose) { printf("omp16=%d ", threads); fflush(stdout); }
    unsigned rc = threads<=1?libsais16_bwt_aux(    (const uint16_t *)ip, (uint16_t *)bwt, sa, iplen_, 0, 0, mod, idxs): 
                             libsais16_bwt_aux_omp((const uint16_t *)ip, (uint16_t *)bwt, sa, iplen_, 0, 0, mod, idxs, threads);
        #else
    unsigned rc = libsais16_bwt_aux(  (const uint16_t *)ip, (uint16_t *)bwt, sa, iplen_, 0, 0, mod, idxs); 
        #endif     
                                                                                if(verbose) { printf("+"); fflush(stdout); }
    if(iplen & 1) bwt[iplen-1] = ip[iplen-1];
  }  else
      #endif
  {
      #ifdef LIBSAIS_OPENMP
                                                                                if(verbose) { printf("omp=%d ", threads); fflush(stdout); }
    threads<=1?libsais_bwt_aux(    ip, bwt, sa, iplen,  0, 0, mod, idxs):
               libsais_bwt_aux_omp(ip, bwt, sa, iplen,  0, 0, mod, idxs, threads);                     
      #else
    libsais_bwt_aux(ip, bwt, sa, iplen,  0, 0, mod, idxs);                      //libsais_bwt(ip, bwt, sa, iplen, fs);//if(ip == in) { memcpy(bwt, ip, iplen); ip = bwt; } memrev(ip, iplen); ip[iplen] = 0;
      #endif
  }
  memcpy(op, idxs, idxsn*sizeof(idxs[0]));
  op   +=          idxsn*sizeof(idxs[0]);
    #endif
  vfree(sa);
  switch(lev) {
    case  0: memcpy(op, bwt, iplen); op += iplen; if(op-out == inlen) *op++ = 0; if(bwt) vfree(bwt); return op - out; break; // op > out_
    case  3: op += xbwt16?becenc16((uint16_t *)bwt, iplen, op):becenc8( bwt, iplen, op); break;
    case  4: op += xbwt16?rcrlesenc16( bwt, iplen, op):      rcrlesenc( bwt, iplen, op); break;
    case  5: op += xbwt16?rcrle1senc16(bwt, iplen, op):      rcrle1senc(bwt, iplen, op); break;
    case  6: op +=        rcqlfcsenc(  bwt, iplen, op);       break;
    case  7: op +=        rcqlfcssenc( bwt, iplen, op, 4, 7); break;
    case  8: op +=        rcmrrsenc(   bwt, iplen, op);       break;
    case  9: op +=        rcmrrssenc(  bwt, iplen, op, 0, 0); break; // prm1,prm2 in mbc.h fixed
     default: 
  }                                                                         
  e:if(bwt) vfree(bwt);                                                         if(verbose) { printf("clen=%lld ", (int64_t)(op-out)); fflush(stdout); }
  if(op >= out_) { memcpy(out, in, inlen); op = out_; }
  return op - out;
}
  #endif

  #ifndef NDECOMP
size_t rcbwtdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned threads) {
  unsigned char *ip    = in;
  unsigned      lenmin = *ip++, xbwt16 = lenmin&0x80; lenmin &=0x7f;
  size_t        oplen  = outlen, rc;

  if(lenmin) oplen = ctou32(ip),ip += 4;

    #ifdef _BWTDIV
  ip++;
  saidx_t       bwtidx = *(saidx_t *)ip; ip += sizeof(saidx_t);
    #else
  unsigned idxs[256];
  int oplen_ = xbwt16?oplen/2:oplen, mod = calcmod(oplen_/SR), idxsn = (oplen_-1)/mod + 1;  //idxsn = (idxsn/SR)*SR;
  ip++; // idxsn
  memcpy(idxs, ip, idxsn*sizeof(idxs[0])); ip += idxsn*sizeof(idxs[0]);
    #endif
  unsigned char *_bwt = vmalloc(oplen+128), *op = out, *bwt = _bwt; if(!_bwt) die("malloc failed\n");
              { if(lenmin) { bwt = out; op = _bwt; } }
  switch(lev) {
    case  0: memcpy(bwt,         ip, oplen+bwtx); break;
    case  3: xbwt16?becdec16(    ip, oplen+bwtx, (uint16_t *)bwt):becdec8(ip, oplen+bwtx, bwt); break;
    case  4: xbwt16?rcrlesdec16( ip, oplen+bwtx, bwt):      rcrlesdec(    ip, oplen+bwtx, bwt); break;
    case  5: xbwt16?rcrle1sdec16(ip, oplen+bwtx, bwt):      rcrle1sdec(   ip, oplen+bwtx, bwt); break;
    case  6:        rcqlfcsdec(  ip, oplen+bwtx, bwt);       break;
    case  7:        rcqlfcssdec( ip, oplen+bwtx, bwt, 4, 7); break; 
    case  8:        rcmrrsdec(   ip, oplen+bwtx, bwt);       break;
    case  9:        rcmrrssdec(  ip, oplen+bwtx, bwt, 0, 0); break;
    default:        
  }
  saidx_t *sa = (saidx_t *)vmalloc((oplen+2+128)*sizeof(sa[0])); if(!sa) { vfree(bwt); die("malloc failed\n"); }
    #ifdef _BWTDIV
  rc = obwt_unbwt_biPSIv2(bwt, op, sa, oplen, bwtidx);
    #else
      #ifdef _LIBSAIS16
  if(xbwt16) { 
        #ifdef LIBSAIS_OPENMP
    rc = threads<=1?libsais16_unbwt_aux(    (uint16_t *)bwt, (uint16_t *)op, sa, oplen_, 0, mod, idxs):
                    libsais16_unbwt_aux_omp((uint16_t *)bwt, (uint16_t *)op, sa, oplen_, 0, mod, idxs, threads);
        #else
    rc = libsais16_unbwt_aux((uint16_t *)bwt, (uint16_t *)op, sa, oplen_, 0, mod, idxs);
        #endif 
    if(oplen & 1) op[oplen-1] = bwt[oplen-1];
  }
  else
      #endif
      #ifdef LIBSAIS_OPENMP
   rc = threads<=1?libsais_unbwt_aux(    bwt, op, sa, oplen, 0, mod, idxs):
                   libsais_unbwt_aux_omp(bwt, op, sa, oplen, 0, mod, idxs, threads);  //libsais_unbwt(bwt, op, sa, oplen, idxs[0]);  //#bwtinv(bwt, oplen, op, NULL, idxs, idxsn); memrev(op, oplen);  //op[256]=0; printf("%s ", op);
      #else
   rc = libsais_unbwt_aux(bwt, op, sa, oplen, 0, mod, idxs);  
      #endif
    #endif
  vfree(sa);

  if(lenmin) {
    switch(lenmin) {
      case 127: utf8dec(op, outlen, out);  break;
        #ifdef _FASTA
      case 126: fastadec(op, outlen, out); break;
        #endif
      default:                                                                  LZPREV(if(lev>=REVLEV) memrev(op, oplen));
        lzpdec(op, oplen, out, outlen, lenmin*4, lev >= LZPLEV?0:LZPHBITS);     LZPREV(if(lev>=REVLEV) memrev(out, outlen));
    }
  }
  vfree(_bwt);
  return rc;
}
  #endif
