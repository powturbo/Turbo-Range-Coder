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
// TurboRC: Range Coder - simple predictor (16 bits counters) 

#include <stdio.h>             
#include "conf.h"   
#include "turborc.h"

#define RC_MACROS
#define RC_BITS 15      	// RC_SIZE + RC_IO: set in turborc_.h
#include "turborc_.h"
#include "mbc_s.h"       	// simple predictor

#define RC1PRM0 RCPRM0		// o8b
#define RC1PRM1 RCPRM0
#include "turborcs_.c"      // template functions 



//--------------------------- lzp preprocessor (lenmin >= 32) ------------------------------------------------------------------------
  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BSWAP32(a) a 
#define BSWAP64(a) a 
  #else
#define BSWAP32(a) bswap32(a)
#define BSWAP64(a) bswap64(a)
  #endif  

#define OVERFLOW(_in_,_inlen_) if(op >= out+(_inlen_*255)/256-16) { memcpy(out,_in_,_inlen_); op = out+_inlen_; goto e; }

#define H_BITS      16 										// hash table size 
#define HASH32(_x_) (((_x_) * 123456791) >> (32-H_BITS))

#define emitmatch(_l_,_op_) { unsigned _l = _l_-lenmin+1; *_op_++ = 255; while(_l >= 254) { *_op_++ = 254;  _l -= 254; OVERFLOW(in,inlen); } *_op_++ = _l; }
#define emitch(_ch_,_op_)   { *_op_++ = _ch_; if(_ch_ == 255) *_op_++ = 0; OVERFLOW(in,inlen); }

size_t lzpenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned lenmin) {	
  unsigned      htab[1<<H_BITS] = {0};
  unsigned char *ip = in, *cp, *op = out;
  unsigned      cl, cx;
  if(lenmin < 32) lenmin = 32;
  if(inlen  < lenmin) return inlen;

  cx = ctou32(ip); ctou32(op) = cx; cx = BSWAP32(cx); op += 4; ip += 4;  	//first context					 
  while(ip < in+inlen-lenmin) { 
    unsigned h4 = HASH32(cx);
             cp = in + htab[h4];
       htab[h4] = ip - in;    
    if(ctou64(ip) == ctou64(cp) && ctou64(ip+8) == ctou64(cp+8) && ctou64(ip+16) == ctou64(cp+16) && ctou64(ip+24) == ctou64(cp+24)) { // match
      for(cl = 32;;) {
        if(ip+cl >= in+inlen-32) break;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
      }
      if(cl >= lenmin) {
        for(; ip+cl < in+inlen && ip[cl] == cp[cl]; cl++);
        emitmatch(cl, op);
        ip += cl;				
        cx = BSWAP32(ctou32(ip-4));		        
        continue;
      }
    }
    unsigned ch = *ip++; emitch(ch, op); cx = cx<<8 | ch; // literal
  }
  while(ip < in+inlen) *op++ = *ip++;
  e:return op - out;													
}

size_t lzpdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lenmin) {
  unsigned      htab[1<< H_BITS] = {0}; 
  unsigned char *ip = in,*op = out;
  unsigned      cx;
  if(lenmin < 32) lenmin = 32;

  cx = ctou32(ip); ctou32(op) = cx; cx = BSWAP32(cx); op += 4; ip += 4;
 
  while(op < out+outlen) {
    unsigned       h4 = HASH32(cx), c;
    unsigned char *cp = out + htab[h4]; 
             htab[h4] = op - out;
    if((c = *ip++) == 255) {				
      if(*ip) {
        c = 0; do c += *ip; while(*ip++ == 254);
        unsigned char *ep = op+c+lenmin-1;
        while(op < ep) *op++ = *cp++;
        cx = BSWAP32(ctou32(op-4));
        continue;
      } else ip++, c = 255;
    }
    cx = cx << 8 | (*op++ = c);
  }
  return ip - in;
}

// QLFC - Quantized Local Frequency Coding: number of different symbols until the next occurrence
// References: https://ieeexplore.ieee.org/document/1402216
//             https://encode.su/threads/546-Move-to-Front-Implementation
unsigned char *rcqlfc(unsigned char *in, unsigned char *out, size_t n, unsigned char *b) {
  unsigned char f[0x100] = {0}, *ip, *op = out;
  unsigned m;
  for(m = 0; m < 0x100; m++) b[m] = m; 
  if(!in[n-1]) b[0] = 1, b[1] = 0;
  
  for(m = 0,ip = in+n; ip > in; ) {
    unsigned char *p, c, c0, c1; 
    for(c = *--ip; ip > in && ip[-1] == c; ip--);
    for(c0 = b[0], b[0] = c,p = &b[1];;p += 4) {
      c1 = p[0]; p[0] = c0; if(c1 == c)           break;
      c0 = p[1]; p[1] = c1; if(c0 == c) { p++;    break; }
      c1 = p[2]; p[2] = c0; if(c1 == c) { p += 2; break; }
      c0 = p[3]; p[3] = c1; if(c0 == c) { p += 3; break; }  
    }
    if(!f[c]) *op++ = m++ - 1,f[c]++; else *op++ = p-b-1;    
  }
  out[0] = 0;
  return op;
}    

