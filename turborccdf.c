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
// TurboRC: Range Coder - CDF functions
#define RC_MULTISYMBOL
#define RC_SIZE 64
#define RC_BITS 15
#define RC_MACROS
#include "turborc_.h"
#include "turborc.h"

#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

#define OVERFLOW(_in_,_inlen_, _op_) if(_op_ >= out+(_inlen_*255)/256-8) { memcpy(out,_in_,_inlen_); return _inlen_; }

// calculate static frequencies and build the cdf (use only for testing)
// It's using a simple normalization, that can fail for certain distributions
int cdfini(unsigned char *in, size_t inlen, cdf_t *cdf, unsigned cdfnum) {
  size_t cnt[0x100] = {0}, i, mx = 0, mxi = 0, cum=0;

  for(i = 0; i < inlen; i++) cnt[in[i]]++; // histogram
  for(i = 0; i < cdfnum; i++) { 
    cnt[i] = (cnt[i] << RC_BITS)/inlen;    // rescale
    if(!cnt[i]) cnt[i] = 1;
    cum += cnt[i];
    if(cnt[i] > mx) mx = cnt[i],mxi = i;   // determine maximum 
  }
  cnt[mxi] -= cum - (1<<RC_BITS);          // adjust max.
    
  for(cdf[0] = i = 0; i < cdfnum; i++) cdf[i+1] = cdf[i]+cnt[i];

  for(i = 0; i < cdfnum; i++)
    if(cdf[i] >= cdf[i+1]) die("Fatal cdf %d:%d,%d\n", i, cdf[i], cdf[i+1]);
  if(cdf[cdfnum] != (1<<(RC_BITS))) die("Fatal: CDF overflow\n");
}

//------------------------------ Encode/Decode using static cdf ------------------------------------------------
size_t rccdfsenc(unsigned char *in, size_t inlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) {
  unsigned char *op = out, *ip;
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);

  for(ip = in; ip < in+inlen; ip++) {
    unsigned ch = *ip;
    cdfenc(rcrange,rclow, cdf, ch, op);
    OVERFLOW(in, inlen, op);
  }
  rceflush(rcrange,rclow, op);
  return op - out;
}

size_t rccdfsldec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // linear search
  unsigned char *ip = in, *op = out; 
  rcdecdef(rcrange, rccode); rcdinit(rcrange, rccode, ip);             

  for(; op < out+outlen;op++) { unsigned x; cdflget(rcrange,rccode, cdf, cdfnum, x, ip); op[0] = x; }
  return outlen;
} 

size_t rccdfsbdec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // binary search
  unsigned char *ip = in, *op = out; 
  rcdecdef(rcrange, rccode); rcdinit(rcrange, rccode, ip);             
  for(; op < out+outlen;op++) cdfbget(rcrange,rccode, cdf, cdfnum, op[0], ip);
} 

size_t rccdfsvbdec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // binary search+division
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode); rcdinit(rcrange, rccode, ip);             

  for(op = out; op < out+outlen; op++) { 
    unsigned ch;
    cdfvbget(rcrange,rccode, cdf, cdfnum, ch, ip);
    *op = ch;
  }
  return outlen;
} 

size_t rccdfsvldec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { //linear search+division
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode); rcdinit(rcrange, rccode, ip);             

  for(op = out; op < out+outlen; op++) { 
    unsigned ch;
    cdfvlget(rcrange,rccode, cdf, cdfnum, ch, ip);
    *op = ch;
  }
}

//------------------------------ Encode/Decode: interleaved RC using static cdf for 16 symbols ------------------------------------------------
size_t rccdfs2enc(unsigned char *in, size_t inlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // interleaved  
  unsigned char *_op0 = out+4,*op0 = _op0, *_op1 = out+4+(inlen/2), *op1=_op1, *ip;
  rcencdef(rcrange0,rclow0); rceinit(rcrange0,rclow0);
  rcencdef(rcrange1,rclow1); rceinit(rcrange1,rclow1);

  for(ip = in; ip < in+(inlen&~(2-1)); ip+=2) {
    unsigned char x0 = ip[0], x1 = ip[1];
    cdfenc(rcrange0,rclow0, cdf, x0, op0);
    cdfenc(rcrange1,rclow1, cdf, x1, op1);           PREFETCH(ip,256,0);
    OVERFLOW(in, inlen, op1);
  }
  for(       ; ip < in+inlen; ip++) 
    cdfenc(rcrange0,rclow0, cdf, ip[0], op0);

  rceflush(rcrange0,rclow0, op0);
  rceflush(rcrange1,rclow1, op1);
  ctou32(out) = op0-_op0; memcpy(op0,_op1,op1-_op1); op0+=op1-_op1; // copy buffer op1 to end of buffer op0
  return op0 - out;
}

//--- Decode using cdf / Symbol search or Division
size_t rccdfsl2dec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // interleaved - linear search
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);             
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);             
  for(; op != out+(outlen&~(2-1)); op+=2) {                  PREFETCH(ip0,512,0); PREFETCH(ip1,512,0);
    unsigned char x0,x1;
    _rccdfrange(rcrange0);
    _rccdfrange(rcrange1);
    _cdflget16(rcrange0,rccode0, cdf, x0, l0);
    _cdflget16(rcrange1,rccode1, cdf, x1, l1);
    _rccdfupdate(rcrange0,rccode0, cdf[x0], cdf[x0+1],ip0);
    _rccdfupdate(rcrange1,rccode1, cdf[x1], cdf[x1+1],ip1);
    op[0] = x0;
    op[1] = x1;
  }
  for(; op != out+outlen; op++)
    cdfbget(rcrange0,rccode0, cdf, cdfnum, op[0], ip0);
} 

size_t rccdfsb2dec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // interleaved - linear search
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);             
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);             
  for(; op != out+(outlen&~(2-1)); op+=2) {                  PREFETCH(ip0,512,0); PREFETCH(ip1,512,0);
    unsigned char x0,x1;
    _rccdfrange(rcrange0);
    _rccdfrange(rcrange1);
    _cdfbget(rcrange0,rccode0, cdf, cdfnum, x0);
    _cdfbget(rcrange1,rccode1, cdf, cdfnum, x1);
    _rccdfupdate(rcrange0,rccode0, cdf[x0], cdf[x0+1],ip0);
    _rccdfupdate(rcrange1,rccode1, cdf[x1], cdf[x1+1],ip1);
    op[0] = x0;
    op[1] = x1;
  }
  for(; op != out+outlen; op++)
    cdfbget(rcrange0,rccode0, cdf, cdfnum, op[0], ip0);
} 

//------------------------------ Encode/Decode using adaptive cdf ------------------------------------------------
#include "turborccdf_.h"
#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

size_t rccdfdec(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(mb0); CDF16DEC1(mb,16); 			  
  unsigned char *op = out, *ip=in;
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);   //div32init(); 
  CDF16DEF;
  for(op = out; op < out+(outlen&~3);op+=4) { PREFETCH(ip,256,0); 
    cdf8d(rcrange,rccode,mb0,mb,op[0],ip);
    cdf8d(rcrange,rccode,mb0,mb,op[1],ip);
    cdf8d(rcrange,rccode,mb0,mb,op[2],ip);
    cdf8d(rcrange,rccode,mb0,mb,op[3],ip);
  }
  for(; op < out+outlen;op++) cdf8d(rcrange,rccode,mb0,mb,op[0],ip);
  return outlen;
}  
size_t rccdfenc(unsigned char *in, size_t inlen, unsigned char *out) { 
  CDF16DEC0(mb0); CDF16DEC1(mb,16);      			                      
  unsigned char *op = out,*ip=in; 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  for(ip = in; ip < in+inlen;ip++) cdf8e(rcrange,rclow,mb0,mb,ip[0],op);
  rceflush(rcrange,rclow, op);
  return op - out;    
}       

size_t rccdfidec(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(mb0); CDF16DEC1(mb,16); 			  
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);				
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);				
  CDF16DEF; 
  for(op = out; op < out+(outlen&~3);op+=4) { PREFETCH(ip0,256,0); PREFETCH(ip1,256,0); 
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,mb0,mb,op[0],ip0,ip1);
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,mb0,mb,op[1],ip0,ip1);
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,mb0,mb,op[2],ip0,ip1);
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,mb0,mb,op[3],ip0,ip1);
  }
  for(; op < out+outlen;op++) cdf8d2(rcrange0,rccode0,rcrange1,rccode1,mb0,mb,op[0],ip0,ip1);
  return outlen;
}  

size_t rccdfienc(unsigned char *in, size_t inlen, unsigned char *out) { 
  CDF16DEC0(mb0); CDF16DEC1(mb,16);      			                      
  unsigned char *_op0 = out+4,*op0 = _op0, *_op1 = out+4+(inlen/2), *op1=_op1, *ip;
  rcencdef(rcrange0,rclow0); rceinit(rcrange0,rclow0);
  rcencdef(rcrange1,rclow1); rceinit(rcrange1,rclow1);
  CDF16DEF; 
  for(ip = in; ip < in+inlen;ip++) cdf8e2(rcrange0,rclow0,rcrange1,rclow1,mb0,mb,ip[0],op0,op1);
  rceflush(rcrange0,rclow0, op0);
  rceflush(rcrange1,rclow1, op1);
  ctou32(out) = op0-_op0; memcpy(op0,_op1,op1-_op1); op0+=op1-_op1;  
  return op0 - out;
}

size_t rccdf4dec(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(mb0);
  unsigned char *op = out, *ip=in;
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip); //div32init(); 
  CDF16DEF; // FILL;
  for(; op < out+(outlen&~3);op+=4) { 				PREFETCH(ip,256,0); 
    cdf4d(rcrange,rccode,mb0,op[0],ip); 
    cdf4d(rcrange,rccode,mb0,op[1],ip);
    cdf4d(rcrange,rccode,mb0,op[2],ip);
    cdf4d(rcrange,rccode,mb0,op[3],ip);
  }
  for(; op < out+outlen;op++) cdf4d(rcrange,rccode,mb0,op[0],ip);
  return outlen;
}  

size_t rccdf4enc(unsigned char *in, size_t inlen, unsigned char *out) { 		
  CDF16DEC0(mb0);   			                      
  unsigned char *op = out,*ip=in; 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  for(ip = in; ip < in+inlen;ip++) cdf4e(rcrange,rclow,mb0,ip[0],op); 
  rceflush(rcrange,rclow, op);
  return op - out;    
}

size_t rccdf4idec(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(mb0); CDF16DEF; // FILL;
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);				
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);				
  for(; op != out+(outlen&~(2-1)); op+=2) {              PREFETCH(ip0,512,0);
    unsigned x0, x1;
    cdflget16(rcrange0,rccode0, mb0, 16, x0, ip0, l0);  
    cdf16upd(mb0, x0);                                     PREFETCH(ip1,512,0);
    cdflget16(rcrange1,rccode1, mb0, 16, x1, ip1, l1);
    cdf16upd(mb0, x1);
    op[0] = x0;
    op[1] = x1;
  }
  for(; op != out+outlen; op++)
    cdf4d(rcrange0,rccode0,mb0,op[0],ip0); 
  return outlen;
}  

size_t rccdf4ienc(unsigned char *in, size_t inlen, unsigned char *out) { 		
  CDF16DEC0(mb0); CDF16DEF; 
  unsigned char *_op0 = out+4,*op0 = _op0, *_op1 = out+4+(inlen/2), *op1=_op1, *ip;
  rcencdef(rcrange0,rclow0); rceinit(rcrange0,rclow0);
  rcencdef(rcrange1,rclow1); rceinit(rcrange1,rclow1);

  for(ip = in; ip != in+(inlen&~(2-1)); ip+=2) {
    unsigned x0 = ip[0], x1 = ip[1];
    cdfenc(rcrange0,rclow0, mb0, x0, op0); 
    cdf16upd(mb0,x0); 
    cdfenc(rcrange1,rclow1, mb0, x1, op1); 
    cdf16upd(mb0,x1); 							PREFETCH(ip,512,0); 
  }
  for(       ; ip != in+inlen; ip++) 
    cdf4e(rcrange0,rclow0,mb0,ip[0],op0); 

  rceflush(rcrange0,rclow0, op0);
  rceflush(rcrange1,rclow1, op1);
  ctou32(out) = op0-_op0; memcpy(op0,_op1,op1-_op1); op0+=op1-_op1;  
  return op0 - out;
}
//------------------------------------------------------------------------------------------------
#undef RC_SIZE
#undef RC_IO
#undef RC_BITS
#undef mbu
#undef mbu_p
#undef mbu_init
#undef mbu_update0
#undef mbu_update1
#undef div32init
#undef DIV_BITS
#undef DIVTDIV32
#undef _LOOP
#undef VCALC

//--- Decode using cdf / Division free with lookup table ------------------------------
#define RC_SIZE 32  //32 bit RC only for division free w/ reciprocal multiplication
#define RC_BITS 15
#define RC_IO   16
#define DIV_BITS (32-RC_BITS)  //=17 include division free coder
#include "turborc_.h"

//--- Encode using cdf ------------------------------
size_t rccdfsmenc(unsigned char *in, size_t inlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) {
  unsigned char *op = out, *ip;
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);

  for(ip = in; ip < in+inlen; ip++) {
    unsigned ch = *ip;
    cdfenc(rcrange,rclow, cdf, ch, op);
    OVERFLOW(in, inlen, op);
  }
  rceflush(rcrange,rclow, op);
  return op - out;
}

size_t rccdfsmbdec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) {
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode);         // range coder
  
  rcdinit(rcrange, rccode, ip);             div32init();
  for(op = out; op < out+outlen; op++) { 
    unsigned ch;
    cdfvbget(rcrange,rccode, cdf, cdfnum, ch, ip);
    *op = ch;
  }
  return outlen;
} 

size_t rccdfsmldec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) {
  unsigned char *ip = in, *op=out; 
  rcdecdef(rcrange, rccode);         // range coder
  
  rcdinit(rcrange, rccode, ip);             div32init();
  for(; op < out+outlen;op++) cdfvlget(rcrange,rccode, cdf, cdfnum, op[0], ip);
  return outlen;
} 

