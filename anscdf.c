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
#include "include/turborc.h" // cdf_t
#include "include/anscdf.h"
#include "include_/conf.h"

#ifndef _NDIVTDEF32
#define _DIVTDEF32
#endif
#include "anscdf_.h"
#include "rcutil_.h"
 
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

#define ANSBLKSIZE (1<<22)

#define ANSN 2
LIBAPI size_t T2(anscdf4senc,FSUFFIX)(unsigned char *in, size_t inlen, unsigned char *out, cdf_t *cdf) {
  unsigned char *op = out, *ip = in, *ep = out+inlen;
  anscdfini(0);

  STATEDEF(st,ANSN);
  #define STE(_i_, _si_) ip--; ansenc(cdf,st[_si_],ep,ip[0])
  for(ip = in+inlen; ip > in+(inlen & ~3);) {                                  if(ep < in +   2+2*4) goto ovr;
    STE(0,0);
  }
  while(ip > in) {                                                             if(ep < in + 4*2+2*4) goto ovr;
    STE(0,1); STE(1,0); STE(2,1); STE(3, 0);
  }
  ansflush(st, ep, ANSN);
  size_t l = (out+inlen) - ep;                                                 if(op + l >= out+inlen) goto ovr;
  memcpy(op, ep, l); op += l;                                                  goto end; ovr:memcpy(out,in,inlen); op = out+inlen;
  end: return op - out;
}

LIBAPI size_t T2(anscdf4sdec,FSUFFIX)(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf) {
  unsigned char *op = out, *ip  = in;
  anscdfini(0);

  STATEDEF(st,ANSN);
  mnfill(st, ip, ANSN);
  #define STD(_i_,_si_) ansdec(cdf,st[_si_],ip,op[_i_])
  for(; op < out+(outlen&~3); op+=4) { STD(0,1); STD(1,0); STD(2,1); STD(3,0); }
  for(; op < out+outlen; op++) { STD(0,0); }
  return outlen;
}

//-- o1
LIBAPI size_t T2(anscdf1dec,FSUFFIX)(unsigned char *in, size_t outlen, unsigned char *out) {
  unsigned char *op = out, *out_ = out+outlen, *ip  = in, cx = 0;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN); 
    CDF16DEC1(mbh, 0x100);
    CDF16DEC2(mbl, 0x100, 16);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st, ip, ANSN);
    #define STD(_i_) { mbu *mh,*ml; mh = mbh[cx]; /*ml = mbl[cx];*/ mndec8(mh,mbl[cx],st,ip,op[_i_]); cx = op[_i_]; }
    for(; op < out+(oplen&~3); op += 4) { STD(0); STD(1); STD(2); STD(3); }
    for(; op < out+oplen;op++) { STD(0); }
  }
  return outlen;
}

LIBAPI size_t T2(anscdf1enc,FSUFFIX)(unsigned char *in, size_t inlen, unsigned char *out) {
  unsigned char *op = out,*ip = in,*in_ = in+inlen, *out_ = out+inlen, cx = 0;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64),*stk = _stk; if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);

  for(; in < in_; in += iplen) {
    CDF16DEC1(mbh, 0x100);
    CDF16DEC2(mbl, 0x100, 16);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    #define STE(_i_) { mbu *mh,*ml; mh = mbh[cx]; /*ml = mbl[cx];*/ mnenc8(mh,mbl[cx],ip[_i_],stk); cx = ip[_i_]; }
    for(; ip < in + (iplen&~3); ip += 4) { STE(0); STE(1); STE(2); STE(3); }
    for(; ip < in + iplen; ip++) { STE(0); }
    mnflush(op,out_,_stk,stk, ANSN);
  }                                                                             goto end; ovr:memcpy(out,in,inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdf4dec,FSUFFIX)(unsigned char *in, size_t outlen, unsigned char *out) {
  unsigned char *op = out, *out_ = out+outlen, *ip = in;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN);
    CDF16DEC0(mb);
    CDF16DEF;
    oplen = min(out_-out, blksize);
    mnfill(st,ip,ANSN);
    for(; op < out+(oplen&~3);op+=4) {
      mndec4(mb,st[0],ip,op[0]);
      mndec4(mb,st[1],ip,op[1]);
      mndec4(mb,st[0],ip,op[2]);
      mndec4(mb,st[1],ip,op[3]);
    }
    for(; op < out+oplen;op++)
      mndec4(mb,st[0],ip,op[0]);
  }
  return outlen;
}

LIBAPI size_t T2(anscdf4enc,FSUFFIX)(unsigned char *in, size_t inlen, unsigned char *out) {
  unsigned char *op = out,*ip = in,*in_ = in+inlen, *out_ = out+inlen;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64),*stk = _stk; if(!_stk) die("malloc error %d ", blksize);
  anscdfini(0);

  for(; in < in_; in += iplen) {
    CDF16DEC0(mb);
    CDF16DEF;
    iplen = min(in_-in, blksize); stk = _stk;
    for(; ip < in + (iplen&~3); ip += 4) {
      mnenc4(mb,1,ip[0],stk);
      mnenc4(mb,0,ip[1],stk);
      mnenc4(mb,1,ip[2],stk);
      mnenc4(mb,0,ip[3],stk);
    }
    for(; ip < in + iplen; ip++)
      mnenc4(mb,0,ip[0],stk);
    mnflush(op,out_,_stk,stk, ANSN);
  }
                                                                                goto end; ovr:memcpy(out,in,inlen); op = out_;
  end:free(_stk);
  return op - out;
}

//----- Turbo VLC
#include "include_/vlcbit.h"

//---------------------------------- Turbo VLC with 6 bits exponent ---------------------------------------------
LIBAPI size_t T2(anscdfuenc16,FSUFFIX)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  unsigned char *op = out+4, *out_ = out+_inlen, *bp = out_;
  uint16_t      *in = (uint16_t *)_in;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  uint16_t      *ip = in, *in_ = in+inlen;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64),*stk = _stk; if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);
  bitddef(bw, br); biteinir(bw,br,bp);

  for(; in < in_; in += iplen) {
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    for(; ip < in + iplen; ip++) {
      unsigned x = ip[0];
      bitvrput(bw,br,bp, 1, 0, x);
      cdfenc6(cdf0, cdf1, x, stk);
    }
    mnflush(op,(bp-8),_stk,stk, ANSN);                                                
  }
  bitflushr(bw,br,bp);
  unsigned l = out_ - bp;                                                       if(op+l >= out_) goto ovr;
  memmove(op, bp, l); op += l; ctou32(out) = op - out;                          goto end; ovr:memcpy(out,_in,_inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfudec16,FSUFFIX)(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *bp = in+ctou32(in);
  uint16_t      *out   = (uint16_t *)_out;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);
  bitddef(bw, br); bitdinir(bw,br,bp);

  uint16_t      *op = out, *out_ = out+outlen;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN);
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st, ip, 2);
    for(; op < out+oplen;op++) {
      unsigned r;
      cdfdec6(cdf0,cdf1, st, ip, r);
      bitvrget(bw,br,bp, 1, 0, r);
      op[0] = r;
    }
  }
  return ctou32(in)+4;
}

LIBAPI size_t T2(anscdfuzenc16,FSUFFIX)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  unsigned char *op = out+4, *out_ = out+_inlen, *bp = out_;
  uint16_t      *in = (uint16_t *)_in;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  uint16_t      *ip = in, *in_ = in+inlen, cx = 0;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*4*sizeof(_stk[0])+64), *stk = _stk;     if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);
  bitddef(bw, br); biteinir(bw,br,bp);

  for(; in < in_; in += iplen) {
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    for(; ip < in + iplen; ip++) {
      uint16_t x = zigzagenc16(ip[0] - cx);
      bitvrput(bw,br,bp, 1, 0, x);
      cdfenc6(cdf0, cdf1, x, stk);
      cx = ip[0];
    }
    mnflush(op,(bp-8),_stk,stk, ANSN);                                                //AC(op <= bp,"Fatal");
  }
  bitflushr(bw,br,bp);
  unsigned l = out_-bp;                                                         if(op+l >= out_) goto ovr;
  memmove(op, bp, l); op += l; ctou32(out) = op - out;                          goto end; ovr:memcpy(out,_in,_inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfuzdec16,FSUFFIX)(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *bp = in+ctou32(in);
  uint16_t      *out   = (uint16_t *)_out;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);
  bitddef(bw, br); bitdinir(bw,br,bp);

  uint16_t      *op = out, *out_ = out+outlen, cx = 0;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN);
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st, ip, 2);
    for(; op < out+oplen; op++) {
      uint16_t r;
      cdfdec6(cdf0,cdf1, st, ip, r);
      bitvrget(bw,br,bp, 1, 0, r);
      op[0] = (cx+= zigzagdec16(r));
    }
  }
  return ctou32(in)+4;
}

//-------------------------------------------- Turbo VLC with 7 bits exponent ------------------------------
LIBAPI size_t T2(anscdfvenc16,FSUFFIX)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  unsigned char *op = out+4, *out_ = out+_inlen, *bp = out_;
  uint16_t      *in = (uint16_t *)_in;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  uint16_t      *ip = in, *in_ = in+inlen;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64), *stk = _stk;     if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);
  bitddef(bw, br); biteinir(bw,br,bp);

  for(; in < in_; in += iplen) {
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    for(; ip < in + iplen; ip++) {
      uint16_t x = ip[0];
      bitvrput(bw,br,bp, 2, 0, x);
      cdfenc7(cdf0, cdf1, x, stk);
    }
    mnflush(op, (bp-8), _stk, stk, ANSN);                                             AS(op <= bp,"Fatal");
  }
  bitflushr(bw,br,bp);
  unsigned l = out_ - bp;                                                       if(op+l >= out_) goto ovr;
  memmove(op, bp, l); op += l; ctou32(out) = op - out;                          goto end; ovr:memcpy(out,_in,_inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfvdec16,FSUFFIX)(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *bp = in+ctou32(in);
  uint16_t      *out   = (uint16_t *)_out;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);
  bitddef(bw, br); bitdinir(bw,br,bp);

  uint16_t      *op = out, *out_ = out+outlen;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN);
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st, ip, 2);
    for(; op < out+oplen; op++) {
      uint16_t r;
      cdfdec7(cdf0,cdf1, st, ip, r);
      bitvrget(bw,br,bp, 2, 0, r);
      op[0] = r;
    }
  }
  return ctou32(in)+4;
}

LIBAPI size_t T2(anscdfvzenc16,FSUFFIX)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  unsigned char *op = out+4, *out_ = out+_inlen, *bp = out_;
  uint16_t      *in = (uint16_t *)_in;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  uint16_t      *ip = in, *in_ = in+inlen, cx = 0;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64), *stk = _stk;      if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);
  bitddef(bw, br); biteinir(bw,br,bp);

  for(; in < in_; in += iplen) {
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    iplen = min(in_-in, blksize);   
    for(; ip < in + iplen; ip++) {
      uint16_t x = zigzagenc16(ip[0] - cx);
      bitvrput(bw,br,bp, 2, 0, x);
      cdfenc7(cdf0, cdf1, x, stk);
      cx = ip[0];
    }
    mnflush(op,(bp-8),_stk,stk, ANSN);                                          AS(op <= bp,"Fatal");
  }
  bitflushr(bw,br,bp);
  unsigned l = out_ - bp;                                                       if(op+l >= out_) goto ovr;
  memmove(op, bp, l); op += l; ctou32(out) = op - out;                          goto end; ovr:memcpy(out,_in,_inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfvzdec16,FSUFFIX)(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *bp = in+ctou32(in);
  uint16_t      *out   = (uint16_t *)_out;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);
  bitddef(bw, br); bitdinir(bw,br,bp);

  uint16_t      *op = out, *out_ = out+outlen, cx = 0;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN);
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st, ip, 2);
    for(; op < out+oplen; op++) {
      uint16_t r;
      cdfdec7(cdf0,cdf1, st, ip, r);
      bitvrget(bw,br,bp, 2, 0, r);
      op[0] = (cx+= zigzagdec16(r));
    }
  }
  return ctou32(in)+4;
}

LIBAPI size_t T2(anscdfvenc32,FSUFFIX)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  unsigned char *op = out+4, *out_ = out+_inlen, *bp = out_;
  uint32_t      *in = (uint32_t *)_in;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  uint32_t      *ip = in, *in_ = in+inlen;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64),*stk = _stk; if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);
  bitddef(bw, br); biteinir(bw,br,bp);

  for(; in < in_; in += iplen) {
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    for(; ip < in + iplen; ip++) {
      unsigned x = ip[0];
      bitvrput(bw,br,bp, 2, 0, x);
      cdfenc7(cdf0, cdf1, x, stk);
    }
    mnflush(op,(bp-8),_stk,stk, ANSN);                                                AS(op <= bp,"Fatal");
  }
  bitflushr(bw,br,bp);
  unsigned l = out_-bp;                                                         if(op+l >= out_) goto ovr;
  memmove(op, bp, l); op += l; ctou32(out) = op - out;                          goto end; ovr:memcpy(out,_in,_inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfvdec32,FSUFFIX)(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *bp = in+ctou32(in);
  uint32_t      *out   = (uint32_t *)_out;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);
  bitddef(bw, br); bitdinir(bw,br,bp);

  uint32_t *op = out, *out_ = out+outlen;
  unsigned     oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN);
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st, ip, 2);
    for(; op < out+oplen; op++) {
      unsigned r;
      cdfdec7(cdf0,cdf1, st, ip, r);
      bitvrget(bw,br,bp, 2, 0, r);
      op[0] = r;
    }
  }
  return ctou32(in)+4;
}

LIBAPI size_t T2(anscdfvzenc32,FSUFFIX)(unsigned char *_in, size_t _inlen, unsigned char *out) {
  unsigned char *op = out+4, *out_ = out+_inlen, *bp = out_;
  uint32_t      *in = (uint32_t *)_in;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  uint32_t      *ip = in, *in_ = in+inlen, cx = 0;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64),*stk = _stk; if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);
  bitddef(bw, br); biteinir(bw,br,bp);

  for(; in < in_; in += iplen) {
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    for(; ip < in + iplen; ip++) {
      unsigned x = zigzagenc32(ip[0] - cx);
      bitvrput(bw,br,bp, 2, 0, x);
      cdfenc7(cdf0, cdf1, x, stk);
      cx = ip[0];
    }
    mnflush(op,(bp-8),_stk,stk, ANSN);                                                AS(op <= bp,"Fatal");
  }
  bitflushr(bw,br,bp);
  unsigned l = out_ - bp;                                                       if(op+l >= out_) goto ovr;
  memmove(op, bp, l); op += l; ctou32(out) = op - out;                          goto end; ovr:memcpy(out,_in,_inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfvzdec32,FSUFFIX)(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *bp = in+ctou32(in);
  uint32_t      *out   = (uint32_t *)_out;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);
  bitddef(bw, br); bitdinir(bw,br,bp);

  uint32_t      *op = out, *out_ = out+outlen, cx = 0;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSN);
    CDF16DEC0(cdf0);
    CDF16DEC0(cdf1);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st, ip, 2);
    for(; op < out+oplen; op++) {
      unsigned r;
      cdfdec7(cdf0,cdf1, st, ip, r);
      bitvrget(bw,br,bp, 2, 0, r);
      op[0] = (cx+= zigzagdec32(r));
    }
  }
  return ctou32(in)+4;
}

#if 0 // 2x interleaved
LIBAPI size_t T2(anscdfenc,FSUFFIX)(unsigned char *in, size_t inlen, unsigned char *out) {
  unsigned char *op = out,*ip = in,*in_ = in+inlen, *out_ = out+inlen;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64),*stk = _stk; if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);

  for(; in < in_; in += iplen) {
    CDF16DEC0(mbh);
    CDF16DEC1(mbl,16);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    #define STE(_i_) mnenc8(mbh,mbl,ip[_i_],stk);
    for(; ip < in + (iplen&~3); ip += 4) { STE(0); STE(1); STE(2); STE(3); }
    for(; ip < in + iplen; ip++) { STE(0); }
    mnflush(op,out_,_stk,stk, ANSN);
  }                                                                             goto end; ovr:memcpy(out,in,inlen); op = out_;
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfdec,FSUFFIX)(unsigned char *in, size_t outlen, unsigned char *out) {
  unsigned char *op = out, *out_ = out+outlen, *ip  = in;
  unsigne       oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);

  for(;out < out_; out += oplen) {
    STATEDEF(st,ANSN);
    CDF16DEC0(mbh);
    CDF16DEC1(mbl,16);
    CDF16DEF;
    oplen = min(out_ - out, blksize); 
    mnfill(st, ip, ANSN);
    #define STD(_i_) mndec8(mbh,mbl,st,ip,op[_i_])
    for(; op < out+(oplen&~3); op += 4) { STD(0); STD(1); STD(2); STD(3); }
    for(; op < out+oplen;op++) { STD(0); }
  }
  return outlen;
}
#else
#define ANSNX 4  // 4x interleaved
LIBAPI size_t T2(anscdfenc,FSUFFIX)(unsigned char *in, size_t inlen, unsigned char *out) {
  unsigned char *op = out,*ip = in,*in_ = in+inlen, *out_ = out+inlen;
  unsigned      iplen, blksize = min(ANSBLKSIZE,inlen),
                *_stk = malloc(blksize*2*sizeof(_stk[0])+64),*stk = _stk; if(!_stk) die("malloc error %d ", blksize); // 2 x stk[0] per nibble
  anscdfini(0);

  for(; in < in_; in += iplen) {
    CDF16DEC0(mbh);
    CDF16DEC1(mbl,16);
    CDF16DEF;
    iplen = min(in_-in, blksize);
    #define STE(_i_) mnenc8x2(mbh,mbl,ip[_i_],ip[_i_+1],stk);
    for(; ip < in + (iplen&~3); ip += 4) { STE(0); STE(2); }
    if(ip < in + (iplen&~1)) { STE(0); ip += 2; }
    if(ip < in +  iplen)     { mnenc8x2(mbh,mbl,ip[0],0,stk); ip++; }
    mnflush(op,out_,_stk,stk, ANSNX);
  }                                                                             goto end; ovr:memcpy(out,in,inlen); op = out_; 
  end:free(_stk);
  return op - out;
}

LIBAPI size_t T2(anscdfdec,FSUFFIX)(unsigned char *in, size_t outlen, unsigned char *out) {
  unsigned char *op = out, *out_ = out+outlen, *ip  = in;
  unsigned      oplen, blksize = min(ANSBLKSIZE,outlen);
  anscdfini(0);
  for(;out < out_; out += oplen) {
    STATEDEF(st, ANSNX); 
    CDF16DEC0(mbh);
    CDF16DEC1(mbl,16);
    CDF16DEF;
    oplen = min(out_ - out, blksize);
    mnfill(st,ip,ANSNX);  
    #define STD(_i_) mndec8x2(mbh,mbl,st,ip,op[_i_], op[_i_+1])
    for(; op < out+(oplen&~3); op += 4) { STD(0); STD(2); }
    if(op < out+(oplen&~1))  { STD(0); op +=2; }
    if(op < out+oplen) { unsigned x; mndec8x2(mbh,mbl,st,ip,op[0], x); op++; }
  }
  return outlen;
}
#endif

#if !defined(__SSE3__) && !defined(__ARM_NEON)
typedef unsigned short mbu; 
#define mbu_q(_p_)          (_p_)
#define mbu_probinit()      (1<<(ANS_BITS-1))
#define mbu_init(__m, __p0) { *(__m) = __p0; }

#define mbu_update1(_mb_, _mbp_)     (*(_mb_) = (_mbp_) + (((1u<<ANS_BITS) - (_mbp_)) >> 5))
#define mbu_update0(_mb_, _mbp_)     (*(_mb_) = (_mbp_) - ((_mbp_) >> 5))
#define mbu_update( _mb_, _mbp_,_b_) _b_?mbu_update1(_mb_,_mbp_):mbu_update0(_mb_,_mbp_)

#define DIV(dividend, divisor) ((dividend)/(divisor))
#define ecbe_(_st_, _p0, _b_, _out_) do { state_t l_s = _b_?_p0:((1u<<ANS_BITS) - _p0); ecenorm(_st_, l_s, _out_); _st_ += DIV(_st_, l_s)*((1u<<ANS_BITS) - l_s) + (_b_?0:_p0); } while(0)
#define ecbe(_st_, _mbp_, _out_) { unsigned _b = (_mbp_)>>ANS_BITS, _mbp = BZHI32(_mbp_,ANS_BITS); ecbe_(_st_, _mbp, _b, _out_); } 
 
#define ecbd(_st_, _mbp_, _act0_, _act1_, _mb_, _x_) do { \
  unsigned _p0 = mbu_q(_mbp_);\
  state_t  _r  = BZHI32(_st_,ANS_BITS), _rcx = (_st_ >> ANS_BITS) *_p0;\
  if(_r <_p0) { _st_  = _rcx + _r;  _act1_(_mb_, _p0); _x_ += _x_+1; }\
  else {        _st_ -= _rcx + _p0; _act0_(_mb_, _p0); _x_ += _x_; }\
} while(0)

#define ABSN  4
#define BBITS 8
#define BSIZE (1<<16)

LIBAPI size_t ansbc(unsigned char *in,  size_t inlen, unsigned char *out) { 
  mbu           ins[BSIZE], mb[1<<BBITS]; 
  unsigned      osize = inlen;
  unsigned char *op = out, *ip, *in_ = in+inlen;
  int i; 
  for(i = 0; i < (1<<BBITS); i++) 
    mbu_init(&mb[i], 1u << (ANS_BITS-1));
  for(ip = in; ip < in_; ) { 
    unsigned char *eip = ip+BSIZE/8; if(eip > in_) eip = in_; 
    mbu *inp = ins;
    for(; ip < eip; ) { 
      unsigned cx = 1<<8 | (*ip++), q, b; 
      mbu *m;
      m = &mb[cx>>8]; q = *m; b = (cx>>7)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b); 
      m = &mb[cx>>7]; q = *m; b = (cx>>6)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b); 
      m = &mb[cx>>6]; q = *m; b = (cx>>5)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b); 
      m = &mb[cx>>5]; q = *m; b = (cx>>4)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b); 
      m = &mb[cx>>4]; q = *m; b = (cx>>3)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b); 
      m = &mb[cx>>3]; q = *m; b = (cx>>2)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b); 
      m = &mb[cx>>2]; q = *m; b = (cx>>1)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b);
      m = &mb[cx>>1]; q = *m; b = (cx>>0)&1; *inp++ = b<<ANS_BITS|mbu_q(q); mbu_update(m,q,b);
    }  
    unsigned char *eop = out + osize; 
    state_t st[ABSN]; 
    for(i = 0; i < ABSN; i++) st[i] = ANS_LOW; 
    
    #define STE(x) { unsigned q = *--inp; ecbe(st[x], q, eop); }
    while(inp != ins) { STE(0);STE(1);STE(2);STE(3); STE(0);STE(1);STE(2);STE(3); } 
    for(i = 0; i < ABSN; i++) eceflush(st[i],eop); 
    
    int l = (out+osize) - eop;                                                  if(op+l > out+osize) die("overflow"); 
    memcpy(op, eop, l); op += l;
  }
  return op - out;
}
 
LIBAPI size_t ansbd(unsigned char *in, size_t outlen, unsigned char *out) { 
  unsigned      i;
  mbu           mb[1<<BBITS]; 
  state_t       st[ABSN];  
  unsigned char *op, *out_ = out + outlen; 
  for(i = 0; i < (1<<BBITS); i++) mbu_init(&mb[i], 1u << (ANS_BITS-1));
  for(i = 0; i < ABSN;       i++) st[i] = ANS_LOW;
  
  for(op = out; op < out_; ) { 
    unsigned char *eop = op + BSIZE/8; 
    if(eop > out_) eop = out_;
    for(i = 0; i < ABSN; i++) { if(st[i] != ANS_LOW) die(stderr, "Fatal error - data corrupted: st=%x\n", st); ecdini(st[i], in); }
    
    for(; op < eop;) { 
      unsigned cx = 1;
      #define ST(c,_x_) { mbu *m = &mb[c]; ecdnorm(st[_x_], in); ecbd(st[_x_], *m, mbu_update0, mbu_update1, m, cx); }
      ST(1, 0);ST(cx,1);ST(cx,2);ST(cx,3); 
      ST(cx,0);ST(cx,1);ST(cx,2);ST(cx,3); 
      *op++= cx; 
    }
  }
  return outlen;
}

fanscdf4senc _anscdf4senc = anscdf4sencs; //set sse2
fanscdf4sdec _anscdf4sdec = anscdf4sdecs;
fanscdfenc   _anscdf4enc  = anscdf4encs;
fanscdfdec   _anscdf4dec  = anscdf4decs;
fanscdfenc   _anscdfenc   = anscdfencs;
fanscdfdec   _anscdfdec   = anscdfdecs;
fanscdfenc   _anscdf1enc  = anscdf1encs;
fanscdfdec   _anscdf1dec  = anscdf1decs;

fanscdfenc _anscdfuenc16  = anscdfuenc16s;
fanscdfdec _anscdfudec16  = anscdfudec16s;
fanscdfenc _anscdfuzenc16 = anscdfuzenc16s;
fanscdfdec _anscdfuzdec16 = anscdfuzdec16s;

fanscdfenc _anscdfvenc16  = anscdfvenc16s;
fanscdfdec _anscdfvdec16  = anscdfvdec16s;
fanscdfenc _anscdfvzenc16 = anscdfvzenc16s;
fanscdfdec _anscdfvzdec16 = anscdfvzdec16s;

fanscdfenc _anscdfvenc32  = anscdfvenc32s;
fanscdfdec _anscdfvdec32  = anscdfvdec32s;
fanscdfenc _anscdfvzenc32 = anscdfvzenc32s;
fanscdfdec _anscdfvzdec32 = anscdfvzdec32s;

static int anscdfset;

void anscdfini(unsigned id) {
  if(anscdfset && !id) return;
    #ifdef _DIVLUT
  div32init();
    #endif
  anscdfset = 1;
  id = id?id:cpuisa();          //printf("cpu=id=%d,%s\n", id, cpustr(id) );
    #ifndef NAVX2
  if(id >= 0x60) {
    _anscdf4senc   = anscdf4sencx;   _anscdf4sdec   = anscdf4sdecx;
    _anscdf4enc    = anscdf4encx;    _anscdf4dec    = anscdf4decx;
    _anscdfenc     = anscdfencx;     _anscdfdec     = anscdfdecx;
    _anscdf1enc    = anscdf1encx;    _anscdf1dec    = anscdf1decx;
    _anscdfuenc16  = anscdfuenc16x;  _anscdfudec16  = anscdfudec16x;
    _anscdfuzenc16 = anscdfuzenc16x; _anscdfuzdec16 = anscdfuzdec16x;
    _anscdfvenc16  = anscdfvenc16x;  _anscdfvdec16  = anscdfvdec16x;
    _anscdfvzenc16 = anscdfvzenc16x; _anscdfvzdec16 = anscdfvzdec16x;
    _anscdfvenc32  = anscdfvenc32x;  _anscdfvdec32  = anscdfvdec32x;
    _anscdfvzenc32 = anscdfvzenc32x; _anscdfvzdec32 = anscdfvzdec32x;
  }  else
    #endif
    #ifndef NSSE
  if(id >= 0x20) {
    _anscdf4senc   = anscdf4sencs;   _anscdf4sdec   = anscdf4sdecs;
    _anscdf4enc    = anscdf4encs;    _anscdf4dec    = anscdf4decs;
    _anscdfenc     = anscdfencs;     _anscdfdec     = anscdfdecs;
    _anscdf1enc    = anscdf1encs;    _anscdf1dec    = anscdf1decs;
    _anscdfuenc16  = anscdfuenc16s;  _anscdfudec16  = anscdfudec16s;
    _anscdfuzenc16 = anscdfuzenc16s; _anscdfuzdec16 = anscdfuzdec16s;
    _anscdfvenc16  = anscdfvenc16s;  _anscdfvdec16  = anscdfvdec16s;
    _anscdfvzenc16 = anscdfvzenc16s; _anscdfvzdec16 = anscdfvzdec16s;
    _anscdfvenc32  = anscdfvenc32s;  _anscdfvdec32  = anscdfvdec32s;
    _anscdfvzenc32 = anscdfvzenc32s; _anscdfvzdec32 = anscdfvzdec32s;
 } else
    #endif
  {
    #ifndef NSCALAR
    _anscdf4senc   = anscdf4sencs;   _anscdf4sdec   = anscdf4sdecs;
    _anscdf4enc    = anscdf4encs;    _anscdf4dec    = anscdf4decs;
    _anscdfenc     = anscdfencs;     _anscdfdec     = anscdfdecs;   //TODO:replace to scalar for non SIMD systems
    _anscdf1enc    = anscdf1encs;    _anscdf1dec    = anscdf1decs;
    _anscdfuenc16  = anscdfuenc16s;  _anscdfudec16  = anscdfudec16s;
    _anscdfuzenc16 = anscdfuzenc16s; _anscdfuzdec16 = anscdfuzdec16s;
    _anscdfvenc16  = anscdfvenc16s;  _anscdfvdec16  = anscdfvdec16s;
    _anscdfvzenc16 = anscdfvzenc16s; _anscdfvzdec16 = anscdfvzdec16s;
    _anscdfvenc32  = anscdfvenc32s;  _anscdfvdec32  = anscdfvdec32s;
    _anscdfvzenc32 = anscdfvzenc32s; _anscdfvzdec32 = anscdfvzdec32s;
    #endif
  }
}

LIBAPI size_t anscdf4senc(   unsigned char *in, size_t inlen,  unsigned char *out, cdf_t *cdf) { anscdfini(0); return _anscdf4senc(   in, inlen,  out, cdf); };
LIBAPI size_t anscdf4sdec(   unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf) { anscdfini(0); return _anscdf4sdec(   in, outlen, out, cdf); };

LIBAPI size_t anscdf4enc(    unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdf4enc(   in, inlen,  out); };
LIBAPI size_t anscdf4dec(    unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdf4dec(   in, outlen, out); };

LIBAPI size_t anscdfenc(     unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdfenc(    in, inlen,  out); };
LIBAPI size_t anscdfdec(     unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdfdec(    in, outlen, out); };
LIBAPI size_t anscdf1enc(    unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdf1enc(   in, inlen,  out); };
LIBAPI size_t anscdf1dec(    unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdf1dec(   in, outlen, out); };


LIBAPI size_t anscdfuenc16(  unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdfuenc16( in, inlen,  out); };
LIBAPI size_t anscdfudec16(  unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdfudec16( in, outlen, out); };
LIBAPI size_t anscdfuzenc16( unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdfuzenc16(in, inlen,  out); };
LIBAPI size_t anscdfuzdec16( unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdfuzdec16(in, outlen, out); };

LIBAPI size_t anscdfvenc16(  unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdfvenc16( in, inlen,  out); };
LIBAPI size_t anscdfvdec16(  unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdfvdec16( in, outlen, out); };
LIBAPI size_t anscdfvzenc16( unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdfvzenc16(in, inlen,  out); };
LIBAPI size_t anscdfvzdec16( unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdfvzdec16(in, outlen, out); };

LIBAPI size_t anscdfvenc32(  unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdfvenc32( in, inlen,  out); };
LIBAPI size_t anscdfvdec32(  unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdfvdec32( in, outlen, out); };
LIBAPI size_t anscdfvzenc32( unsigned char *in, size_t inlen,  unsigned char *out) { anscdfini(0); return _anscdfvzenc32(in, inlen,  out); };
LIBAPI size_t anscdfvzdec32( unsigned char *in, size_t outlen, unsigned char *out) { anscdfini(0); return _anscdfvzdec32(in, outlen, out); };
#endif
