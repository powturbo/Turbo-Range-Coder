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

static int bwtx;
static inline unsigned calcmod(size_t len) { return 1<<__bsr32(len); }
#define SR 16
#define LZPRLEV 3

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
  #ifndef NCOMP

#define FS (1<<20)
static int sample(char *in, size_t n, unsigned i, unsigned j, char *out) {
  if(n < j) { memcpy(out, in, n); return n; }
  size_t segment_size = n / (size_t)i;
  for (unsigned k = 0; k < i; k++) {
    size_t start = (size_t)k * segment_size; //  copy first j consecutive bytes of segment k
    memcpy(out + (size_t)k * (size_t)j, in + start, (size_t)j);
  }
  return n;
}

static size_t bwtenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned char *pidx, int xbwt16, int threads, int v) {
  unsigned char *op = out, *ip, *in_ = in+inlen, *oq = pidx;                                                 
  unsigned iplen, bsize = inlen > (2047<<20)?(2047<<20):inlen;             
  saidx_t *sa = (saidx_t *)vmalloc((size_t)(bsize+FS+128)*sizeof(sa[0])); if(!sa) die("bwtenc: malloc error. size=%zu ", (size_t)(bsize+FS+128)*sizeof(sa[0])); 
                                                                                    
  for(ip = in; ip < in_; ip += iplen, op += iplen) {
    iplen = in_- ip > bsize?bsize:in_ - ip;
      #ifdef _BWTDIV
    *oq++ = 0; *(saidx_t *)oq  = divbwt(ip, op, sa, iplen); oq += sizeof(sa[0]);            
      #else
                                                                                      if(v) { printf("bwt16=%u omp ", xbwt16>0);fflush(stdout); }
    unsigned idxs[256], iplen_ = xbwt16?(iplen/2):iplen;                              if(iplen > 2047*1024*1024) die("size of bwt input '%zu' exceed 2047m ", iplen);
    unsigned mod = calcmod(iplen_/SR), 
        #ifdef BWT_SINGLE
             idxsn = xbwts?1:(iplen_-1)/mod + 1;                     
        #else
             idxsn = (iplen_-1)/mod + 1;                                               //printf("bwt idxs=%d ", idxsn); //idxsn = (idxsn/SR)*SR;
        #endif
        #ifndef _OPENMP
                                                                                       if(v) printf("disabled ");
        #endif
        #ifdef _LIBSAIS16
    if(xbwt16) {                                                                       if(v) { printf("-"); fflush(stdout); }
          #ifdef _OPENMP
            #ifdef BWT_SINGLE
      if(xbwts) { idxs[0] = threads<=1?libsais16_bwt(   (const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0): libsais16_bwt_omp((const uint16_t *)ip, (uint16_t *)op, sa, iplen_, 0, 0, threads); } else 
            #endif
      {  unsigned rc = threads<=1?libsais16_bwt_aux(    (const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0, mod, idxs): 
                                  libsais16_bwt_aux_omp((const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0, mod, idxs, threads);
      }
          #else
      unsigned rc;
            #ifdef BWT_SINGLE
      if(xbwts) idxs[0] = libsais16_bwt(      (const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0);  else
            #endif
                          libsais16_bwt_aux(  (const uint16_t *)ip, (uint16_t *)op, sa, iplen_, FS, 0, mod, idxs); 
          #endif     
                                                                                       if(v) { printf("+"); fflush(stdout); }
      if(iplen & 1) op[iplen-1] = ip[iplen-1];
    } else
        #endif
    {
        #ifdef _OPENMP                                                                                     
          #ifdef BWT_SINGLE
      if(xbwts) { idxs[0] = threads<=1?libsais_bwt(    ip, op, sa, iplen,  0, 0) : libsais_bwt_omp(ip, op, sa, iplen,  0, 0, threads); } else
          #endif 
      { threads<=1?libsais_bwt_aux(ip, op, sa, iplen,  0, 0, mod, idxs) : libsais_bwt_aux_omp(ip, op, sa, iplen,  0, 0, mod, idxs, threads); printf("il=%zu\n", iplen);  }                     
        #else
          #ifdef BWT_SINGLE
      if(xbwts) idxs[0] = libsais_bwt(    ip, op, sa, iplen,  0, 0); else
          #endif
                          libsais_bwt_aux(ip, op, sa, iplen,  0, 0, mod, idxs);                          
        #endif
    }
    *oq++ = idxsn - 1;
    memcpy(oq, idxs, idxsn*sizeof(idxs[0])); oq += idxsn*sizeof(idxs[0]);   /*oq++=0x9a;*/              printf("idx:%d ", idxsn);           //printf("idx:");for(int i=0; i < idxsn;i++) printf("%u,", idxs[i]);
      #endif
  }
  vfree(sa);
  return oq - pidx;
}

  #ifdef BWT_SINGLE
static int xbwts = 0;
  #endif
static unsigned char hbits[] = { 16,16,16,16,16,18,18,18,20,22 };
#define RC_HBITS(lev) (inlen>(1<<24)?hbits[lev]:LZPHBITS)
size_t rcbwtenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned lev, unsigned threads, unsigned opts) {
  size_t        iplen  = inlen;
  unsigned      lenmin = opts & 0xff, xbwt16 = (opts & BWT_BWT16)?0x40:0, v = opts & BWT_VERBOSE, nutf8 = (opts & BWT_UTF8_NO)?1:0,
                lzprev = (lev>=LZPRLEV && !(opts & BWT_LZP_NREV))?0x20:0, forcelzp = (opts & BWT_LZP_FORCE)?1:0; 
  unsigned char *op    = out, *out_ = out+inlen, *_bwt = vmalloc(inlen+(1<<20)), *bwt = _bwt, *ip = in;  // inlen + space for bwt indexes idxns  
  if(!lev) lenmin = 0;                                                               if(!bwt) { op = out_; goto e; }
  if(!threads) threads = 1;                                                          //histopt(in, inlen, 0, NULL);  //if(v) printf("\nLMIN0=%d ", lenmin); 
  if(lenmin == 1) { int nrev;                                                        
    lenmin = sample(in, inlen, 16, 16*1024, out);     
    lenmin = histopt(out, lenmin, lev>=8, &nrev);                                    //if(v) printf("LMIN=%d ", lenmin);
    if(!(opts & BWT_LZP_NREV)) lzprev = nrev?0:0x20;
  } 
  unsigned flag = xbwt16 | lzprev;
                                                                                     if(v) { printf("\nlev=%u len(MB)=%zu nutf8=%d rev=%d threads=%d ", lev, inlen/(1<<20), nutf8, lzprev, threads); fflush(stdout); }
  if(lenmin) {                                                                       if(v) { printf("lenmin=%u ", lenmin); fflush(stdout); }
    ip = bwt;
    switch(lenmin) { // lenmin = 1..7 preprocess id
        #ifdef _FASTA
      case 2: iplen = fastaenc(in, inlen, ip, inlen);                                if(v) { printf("Fasta:%u->%u:%.2f%%", inlen, iplen, (double)iplen*100.0/inlen); fflush(stdout); } 
        break;
      case 3: iplen = fastaenc(in, inlen, out, inlen);                             
        if(lzprev) memrev(out, iplen);                                               if(v) { printf("Fasta:%u->%u:%.2f%%", inlen, iplen, (double)iplen*100.0/inlen); fflush(stdout); } 
        iplen = lzpenc(out, iplen, ip+4, 12, 0 lev >= LZPHLIM?0:LZPHBITS);                 
        ctou32(ip) = iplen;                                                          if(v) { printf("->Lzp:%u=%.2f%% ", iplen, (double)iplen*100.0/inlen); fflush(stdout); }
        break;
        #endif
        #ifdef _EXE
      case 4:  if(!exeenc(in, inlen, ip)) ip = in, iplen = inlen;                    if(v) { printf("exe:%zu->%zu ", inlen, iplen); fflush(stdout);}
        break;
        #endif  
      default : if(!nutf8) { iplen = utf8enc(in, inlen, ip, opts);                   if(v) { if(iplen == inlen) printf("NoUTF8 "); else printf("UTF8:%zu->%zu ", inlen, iplen); fflush(stdout); } }
    }
    if(lenmin < LZPLENMIN || iplen != inlen && iplen != -1) {
      flag |= (lenmin < LZPLENMIN?lenmin:0);                                         if(v && lenmin < 32) printf("No Lzp run %d %d ", lenmin, iplen);
    } else {                                                                         if(v) { printf("Lzp: minlen=%d ", lenmin); fflush(stdout); } 
      ip     = bwt; if(lzprev) { memcpy(out, in, inlen); memrev(out, inlen); }
      iplen  = lzpenc(lzprev?OUT:in, inlen, ip, lenmin, RC_HBITS(lev));              if(v) { printf("Lzp=%zu->%zu=%.2f%% ", inlen, iplen, (double)iplen*100.0/inlen);fflush(stdout); }
      if(iplen == inlen || iplen+(inlen>>7)+256 > inlen && !forcelzp) {              if(v) { printf("Not enough saving ");fflush(stdout); }  //Not enough saving
        ip = in; iplen = inlen; lenmin = 0;
      } else { flag |= 0x80; if(lzprev) memrev(ip, iplen); }
    }
  } else                                                                             if(v) { printf("No preprocessing. Flag=%x ", flag); fflush(stdout); }

  *op++ = flag;                                    
  if(flag & 0x80) ctou32(op) = iplen, op += 4;                                       unsigned char *p = op;
  op += bwtenc(ip, iplen, bwt, op, xbwt16, threads, v);                              printf("iplen=%zu hlen=%u ", iplen, (unsigned)(op-p)); fflush(stdout);
  switch(lev) {
    case  0: memcpy(op, bwt, iplen); op += iplen; if(_bwt) vfree(_bwt); return op - out;              // op > out_
    case  3: op += xbwt16?becenc16((uint16_t *)bwt, iplen, op):    becenc8( bwt, iplen, op);       break;
    case  4: op += xbwt16 ?       rcrlesenc16( bwt, iplen, op):rcqlfc0senc( bwt, iplen, op);       break;
    case  5: op += xbwt16 ?       rcrle1senc16(bwt, iplen, op):rcqlfc0ssenc(bwt, iplen, op, 4, 7); break;
    case  6: op +=                rcqlfcsenc(  bwt, iplen, op);       break;
    case  7: op +=                rcqlfcssenc( bwt, iplen, op, 4, 7); break;
    case  8: op +=                rcmrrsenc(   bwt, iplen, op);       break;
    case  9: op +=                rcmrrssenc(  bwt, iplen, op, 0, 0); break; // prm1,prm2 in mbc.h fixed
     default: 
  }                                                                         
  e:if(_bwt) vfree(_bwt);                                                            if(v) { printf("clen=%lld ", (int64_t)(op-out)); fflush(stdout); }
  if(op >= out_) { memcpy(out, in, inlen); op = out_; }
  return op - out;
}
  #endif

  #ifndef NDECOMP
static size_t bwtdec(unsigned char *in, size_t inlen, unsigned char *out, size_t outlen, unsigned char *pidx, int xbwt16, int threads, int v) {
  unsigned char *ip = in, *iq = pidx, *in_ = in+outlen, *op = out, *out_ = out+outlen, rc;                                                 
  unsigned oplen, bsize = outlen > (2047<<20)?(2047<<20):outlen;                         printf("bsize=%u ", bsize);fflush(stdout);
  saidx_t *sa = (saidx_t *)vmalloc((outlen+128)*sizeof(sa[0])); if(!sa) die("malloc: not enough memory\n");

  for(op = out; op < out_; op += oplen, ip += oplen) {
    oplen = out_ - op > bsize?bsize : out_ - op;
      #ifdef _BWTDIV
    iq++; saidx_t bwtidx = *(saidx_t *)iq; iq += sizeof(saidx_t);
      #else
    unsigned idxs[256];
    int oplen_ = xbwt16?oplen/2:oplen, mod = calcmod(oplen_/SR), 
                 #ifdef BWT_SINGLE
               idxsn = xbwts?1:(oplen_-1)/mod + 1;  
                 #else
               idxsn = (oplen_-1)/mod + 1;
                 #endif
    iq++; // idxsn
    memcpy(idxs, iq, idxsn*sizeof(idxs[0])); iq += idxsn*sizeof(idxs[0]);    //printf("idx:");for(int i=0; i < idxsn;i++) printf("%u,", idxs[i]);
      #endif

      #ifdef _BWTDIV
    rc = obwt_unbwt_biPSIv2(ip, op, sa, oplen, bwtidx);
      #else
        #ifdef _LIBSAIS16
    if(xbwt16) { 
          #ifdef _OPENMP
      rc = threads<=1?libsais16_unbwt_aux(    (uint16_t *)ip, (uint16_t *)op, sa, oplen_, 0, mod, idxs):
                      libsais16_unbwt_aux_omp((uint16_t *)ip, (uint16_t *)op, sa, oplen_, 0, mod, idxs, threads);
          #else
      rc = libsais16_unbwt_aux((uint16_t *)ip, (uint16_t *)op, sa, oplen_, 0, mod, idxs);
          #endif 
      if(oplen & 1) op[oplen-1] = ip[oplen-1];
    } else
        #endif
      #ifdef _OPENMP
        #ifdef BWT_SINGLE
    if(xbwts) { rc = threads<=1?libsais_unbwt(ip, op, sa, oplen, 0, idxs[0])  : libsais_unbwt_omp(    ip, op, sa, oplen, 0,   idxs[0], threads); } else
        #endif 
    { rc = threads<=1?libsais_unbwt_aux(      ip, op, sa, oplen, 0, mod, idxs): libsais_unbwt_aux_omp(ip, op, sa, oplen, 0, mod, idxs, threads); printf("OL=%zu\n", oplen);  }
      #else
        #ifdef BWT_SINGLE
    if(xbwts) rc = xbwts?libsais_unbwt(ip, op, sa, oplen, 0, idxs[0]); else
        #endif
    {         rc = libsais_unbwt_aux(ip, op, sa, oplen, 0, mod, idxs); printf("ol=%zu\n", oplen); }  
      #endif
    #endif
  }
  vfree(sa);
  return op - out;
}

size_t rcbwtdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned threads) {
  unsigned char *ip    = in, *out_ = out+outlen, *iq;
  unsigned      lenmin = *ip++, lzp = lenmin & 0x80, xbwt16 = lenmin & 0x40, lzprev = lenmin & 0x20;           //printf("\nL=%d,%x\n ", lenmin & 0xf, lzp);  fflush(stdout);                
  size_t        oplen  = outlen, rc;                                            //printf("ol =%zu lzp=%d lenmin=%x ", oplen, lzp?1:0, lenmin);
  unsigned      bsize = outlen > (2047<<20)?(2047<<20):outlen;                         printf("bsize=%u ", bsize);fflush(stdout);

  if(!threads) threads = 1;
  if(lzp) { oplen = ctou32(ip), ip += 4;                                        /*printf("lzplen=%zu ", oplen);*/ }
  unsigned char *_bwt = vmalloc(oplen+128), *op = out, *bwt = _bwt; if(!_bwt) die("malloc failed\n");
  if(lenmin & 0x8f) { op = bwt; bwt = out; }
  iq = ip;  
  unsigned olen;
  for(unsigned char *op = out; op < out_; op += olen) {
    olen = out_- op > bsize?bsize:out_ - op;
    unsigned oplen_ = xbwt16?(olen/2):olen, mod = calcmod(oplen_/SR), 
               #ifdef BWT_SINGLE
             idxsn = xbwts?1:(oplen_-1)/mod + 1;                     
               #else
             idxsn = (oplen_-1)/mod + 1;                                              
               #endif
    ip += 1+idxsn*sizeof(saidx_t);                          
  }
                                                                                      /*if(*ip++ != 0x9a) die("Fatal"); */printf("Hlen=%u ", ip-iq);fflush(stdout);
  switch(lev) {
    case  0: bwt = ip; break;
    case  3: xbwt16?becdec16(    ip, oplen, (uint16_t *)bwt):becdec8(      ip, oplen, bwt); break;
    case  4: xbwt16?rcrlesdec16( ip, oplen,             bwt):rcqlfc0sdec(  ip, oplen, bwt); break;
    case  5: xbwt16?rcrle1sdec16(ip, oplen,             bwt):rcqlfc0ssdec( ip, oplen, bwt, 4, 7); break;
    case  6:        rcqlfcsdec(  ip, oplen,             bwt);       break;
    case  7:        rcqlfcssdec( ip, oplen,             bwt, 4, 7); break; 
    case  8:        rcmrrsdec(   ip, oplen,             bwt);       break;
    case  9:        rcmrrssdec(  ip, oplen,             bwt, 0, 0); break;
    default:        
  }
  rc = bwtdec(bwt, 0, op, oplen, iq, xbwt16, threads, 0);  if(!lev) return rc;                             
    #if 1                                                                         //printf("lenmin=%x", lenmin);  fflush(stdout); 
  switch(lenmin & 0xf) {
    case 1: utf8dec(op, outlen, out); break;
      #ifdef _FASTA
    case 2: fastadec(op, 0, out, outlen); break;
    case 3: { 
      unsigned l = ctou32(op); op += 4;                                    //if(lzprev) memrev(op, l);        //printf("\nlzplen=%u ", l);  fflush(stdout);
      lzpdec(op, inlen, _bwt, l);                                              //if(lzprev) memrev(_bwt, l);         //printf("fasta len=%u ", l); fflush(stdout);
      fastadec(_bwt, 0, out, outlen); 
    } break;
      #ifdef _EXE
    case 4: exedec(op, out, outlen); break;
      #endif
      #endif
  }  
    #endif
  if(lzp) { if(lzprev) memrev(op, oplen);   lzpdec(op, oplen, out, outlen); //printf("LZP");  fflush(stdout); 
            if(lzprev) memrev(out, outlen); 
  } 
  vfree(_bwt);
  return rc;
}
  #endif
