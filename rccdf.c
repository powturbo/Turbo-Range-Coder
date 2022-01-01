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
// TurboRC: Range Coder - CDF functions
#include "conf.h"

  #ifdef __AVX2__      
#define RC_SIZE 32     // avx2 symbol search only with RC_SIZE=32 
#define RC_BITS 14
#define RC_IO   16     // set RC_IO to 8 for more precision
  #else
#define RC_SIZE 64
#define RC_BITS 15
  #endif
#define RC_MULTISYMBOL
#define RC_MACROS 
#include "rcutil_.h"
#include "turborc_.h"
#include "turborc.h"
#include "rccdf_.h" 
 
#define OVERFLOWI(_in_,_inlen_, _op_, _op0_, _op1_) if(_op_ >= out+(_inlen_*255)/256-8 || _op0_ >= _op1_) { memcpy(out,_in_,_inlen_); return _inlen_; }

// calculate static frequencies and build the cdf (use only for testing)
// It's using a simple normalization, that can fail for certain distributions
int cdfini(unsigned char *in, size_t inlen, cdf_t *cdf, unsigned cdfnum) {
  size_t cnt[0x100] = {0}, i, mx = 0, mxi = 0, cum = 0;

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
    cdfenc(rcrange,rclow, cdf, ch, op);                  						OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

// linear symbol search decoding 
size_t rccdfsldec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // linear search
  unsigned char *ip = in, *op = out; 
  rcdecdef(rcrange, rccode); rcdinit(rcrange, rccode, ip);             

  for(; op < out+outlen;op++) { unsigned x; cdflget(rcrange,rccode, cdf, cdfnum, x, ip); op[0] = x; }
  return outlen;
} 
 
// binary symbol search decoding 
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
  unsigned char *_op0 = out+4,*op0 = _op0, *_op1 = out+4+((inlen-4)*37/64), *op1=_op1, *ip = in;
  rcencdef(rcrange0,rclow0); rceinit(rcrange0,rclow0);
  rcencdef(rcrange1,rclow1); rceinit(rcrange1,rclow1);

  for(; ip < in+(inlen&~(2-1)); ip+=2) {
    unsigned char x0 = ip[0], x1 = ip[1];
    cdfenc(rcrange0,rclow0, cdf, x0, op0);
    cdfenc(rcrange1,rclow1, cdf, x1, op1);           							PREFETCH(ip+256,0); OVERFLOWI(in, inlen, op1, op0, _op1);
  }
  for(       ; ip < in+inlen; ip++) 
    cdfenc(rcrange0,rclow0, cdf, ip[0], op0);

  rceflush(rcrange0,rclow0, op0);
  rceflush(rcrange1,rclow1, op1); 									
  ctou32(out) = op0-_op0; memcpy(op0,_op1,op1-_op1); op0 += op1-_op1; // copy buffer op1 to end of buffer op0
																				OVERFLOW(in,inlen,out, op0, goto e); 
  e:return op0 - out;
}

//--- Decode using cdf / Symbol search or Division
size_t rccdfsl2dec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) { // interleaved - linear search
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);             
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);             
  for(; op != out+(outlen&~(2-1)); op+=2) {                  					//PREFETCH(ip0+512,0); PREFETCH(ip1+512,0);
    unsigned char x0,x1;
    _rccdfrange(rcrange0);
    _rccdfrange(rcrange1);
    _cdflget16(rcrange0,rccode0, cdf, x0);
    _cdflget16(rcrange1,rccode1, cdf, x1);
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
  for(; op != out+(outlen&~(2-1)); op+=2) {                  					PREFETCH(ip0+512,0); PREFETCH(ip1+512,0);
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
size_t rccdfdec(unsigned char *in, size_t outlen, unsigned char *out) {
  CDF16DEC0(cdf); CDF16DEC1(cdf1,16); 			  
  unsigned char *op = out, *ip=in;
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);   //div32init(); 
  CDF16DEF;
  for(op = out; op < out+(outlen&~3);op+=4) { PREFETCH(ip+256,0); 
    cdf8d(rcrange,rccode,cdf,cdf1,op[0],ip);
    cdf8d(rcrange,rccode,cdf,cdf1,op[1],ip);
    cdf8d(rcrange,rccode,cdf,cdf1,op[2],ip);
    cdf8d(rcrange,rccode,cdf,cdf1,op[3],ip);
  }
  for(; op < out+outlen;op++) cdf8d(rcrange,rccode,cdf,cdf1,op[0],ip);
  return outlen;
}  
size_t rccdfenc(unsigned char *in, size_t inlen, unsigned char *out) { 
  CDF16DEC0(cdf); CDF16DEC1(cdf1,16);      			                      
  unsigned char *op = out,*ip=in; 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  for(ip = in; ip < in+inlen;ip++) {
	cdf8e(rcrange,rclow,cdf,cdf1,ip[0],op);										OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;   
}       

size_t rccdfidec(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(cdf); CDF16DEC1(cdf1,16); 			  
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);				
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);				
  CDF16DEF; 
  for(; op < out+(outlen&~(4-1)); op+=4) { 										PREFETCH(ip0+384,0); PREFETCH(ip1+384,0);
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,cdf,cdf1,op[0],ip0,ip1);
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,cdf,cdf1,op[1],ip0,ip1); 	 
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,cdf,cdf1,op[2],ip0,ip1);
    cdf8d2(rcrange0,rccode0,rcrange1,rccode1,cdf,cdf1,op[3],ip0,ip1);
  }
  for(; op < out+outlen; op++) 
	cdf8d2(rcrange0,rccode0,rcrange1,rccode1,cdf,cdf1,op[0],ip0,ip1);
  return outlen;
}  

size_t rccdfienc(unsigned char *in, size_t inlen, unsigned char *out) { 
  CDF16DEC0(cdf); CDF16DEC1(cdf1,16);      			                      
  unsigned char *_op0 = out+4,*op0 = _op0, *_op1 = out+4+(inlen/2), *op1=_op1, *ip;
  rcencdef(rcrange0,rclow0); rceinit(rcrange0,rclow0);
  rcencdef(rcrange1,rclow1); rceinit(rcrange1,rclow1);
  CDF16DEF; 
  for(ip = in; ip < in+inlen;ip++) {
	cdf8e2(rcrange0,rclow0,rcrange1,rclow1,cdf,cdf1,ip[0],op0,op1);
    OVERFLOWI(in, inlen, op1, op0, _op1);
  }
  rceflush(rcrange0,rclow0, op0);
  rceflush(rcrange1,rclow1, op1);
  ctou32(out) = op0-_op0; memcpy(op0,_op1,op1-_op1); op0+=op1-_op1;  			OVERFLOW(in,inlen,out, op0, goto e);  
  e:return op0 - out;
}

size_t rccdf4dec(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(cdf);
  unsigned char *op = out, *ip=in;
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip); //div32init(); 
  CDF16DEF; // FILL;
  for(; op < out+(outlen&~3);op+=4) { 											PREFETCH(ip+256,0); 
    cdf4d(rcrange,rccode,cdf,op[0],ip); 
    cdf4d(rcrange,rccode,cdf,op[1],ip);
    cdf4d(rcrange,rccode,cdf,op[2],ip);
    cdf4d(rcrange,rccode,cdf,op[3],ip);
  }
  for(; op < out+outlen;op++) cdf4d(rcrange,rccode,cdf,op[0],ip);
  return outlen;
}  

size_t rccdf4enc(unsigned char *in, size_t inlen, unsigned char *out) { 		
  CDF16DEC0(cdf);   			                      
  unsigned char *op = out,*ip=in; 
  CDF16DEF;  
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  for(ip = in; ip < in+inlen;ip++) {
    cdf4e(rcrange,rclow, cdf, ip[0], op); 										OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;    
}  

size_t rccdf4idec(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(cdf); CDF16DEF; // FILL;
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);				
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);				
  for(; op != out+(outlen&~(2-1)); op+=2) {     			         
    unsigned x0, x1;
    //cdflget16(rcrange0,rccode0, cdf, 16, x0, ip0);  															
    //cdflget16(rcrange1,rccode1, cdf, 16, x1, ip1);
    _rccdfrange(rcrange0);
    _rccdfrange(rcrange1);
    _cdflget16(rcrange0,rccode0, cdf, x0);
    _cdflget16(rcrange1,rccode1, cdf, x1);
    _rccdfupdate(rcrange0,rccode0, cdf[x0], cdf[x0+1],ip0);
    _rccdfupdate(rcrange1,rccode1, cdf[x1], cdf[x1+1],ip1); 					//PREFETCH(ip0,512,0); PREFETCH(ip1,512,0);
    cdf16upd(cdf, x0);                                     
    cdf16upd(cdf, x1);
    op[0] = x0;
    op[1] = x1;
  }
  for(; op != out+outlen; op++)
    cdf4d(rcrange0,rccode0,cdf,op[0],ip0); 
  return outlen;
}  

size_t rccdf4ienc(unsigned char *in, size_t inlen, unsigned char *out) { 		
  CDF16DEC0(cdf); CDF16DEF; 
  unsigned char *_op0 = out+4,*op0 = _op0, *_op1 = out+4+(inlen/2), *op1=_op1, *ip;
  rcencdef(rcrange0,rclow0); rceinit(rcrange0,rclow0);
  rcencdef(rcrange1,rclow1); rceinit(rcrange1,rclow1);

  for(ip = in; ip != in+(inlen&~(2-1)); ip+=2) {
    unsigned x0 = ip[0], x1 = ip[1];
    cdfenc(rcrange0,rclow0, cdf, x0, op0); 
    cdfenc(rcrange1,rclow1, cdf, x1, op1); 
    cdf16upd(cdf,x0); 
    cdf16upd(cdf,x1); 															PREFETCH(ip+512,0); OVERFLOW(in,inlen,out, op1, goto e);
  }
  for(       ; ip != in+inlen; ip++) 
    cdf4e(rcrange0,rclow0,cdf,ip[0],op0); 

  rceflush(rcrange0,rclow0, op0);
  rceflush(rcrange1,rclow1, op1);
  ctou32(out) = op0-_op0; memcpy(op0,_op1,op1-_op1); op0+=op1-_op1;  			OVERFLOW(in,inlen,out, op0, goto e); 
  e:return op0 - out;      
}


size_t rccdfdec8(unsigned char *in, size_t outlen, unsigned char *out) {
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); CDF16DEC0(cdf2); 
  unsigned char *op = out, *ip=in;
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);
  CDF16DEF;
  for(op = out; op < out+(outlen&~3);op+=4) { PREFETCH(ip+256,0); 
    cdfd8(rcrange,rccode,rcrange,rccode,cdf0,cdf1,cdf2,op[0],ip,ip);
    cdfd8(rcrange,rccode,rcrange,rccode,cdf0,cdf1,cdf2,op[1],ip,ip);
    cdfd8(rcrange,rccode,rcrange,rccode,cdf0,cdf1,cdf2,op[2],ip,ip);
    cdfd8(rcrange,rccode,rcrange,rccode,cdf0,cdf1,cdf2,op[3],ip,ip);
  }
  for(; op < out+outlen;op++) cdfd8(rcrange,rccode,rcrange,rccode,cdf0,cdf1,cdf2,op[0],ip,ip);
  return outlen;
}  

size_t rccdfenc8(unsigned char *in, size_t inlen, unsigned char *out) { 
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); CDF16DEC0(cdf2); 
  unsigned char *op = out,*ip=in; 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  for(ip = in; ip < in+inlen;ip++) {
	cdfe8(rcrange,rclow,rcrange,rclow,cdf0,cdf1,cdf2,ip[0],op,op);				OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;   
} 


size_t rccdfidec8(unsigned char *in, size_t outlen, unsigned char *out) { 
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); CDF16DEC0(cdf2); 
  unsigned char *ip0 = in+4, *ip1 = (in+4)+ctou32(in), *op = out;
  rcdecdef(rcrange0, rccode0);     rcdinit(rcrange0, rccode0, ip0);				
  rcdecdef(rcrange1, rccode1);     rcdinit(rcrange1, rccode1, ip1);				
  CDF16DEF; 
  for(; op < out+(outlen&~(4-1));op+=4) { 										PREFETCH(ip0+384,0); PREFETCH(ip1+384,0);
    cdfd8(rcrange0,rccode0,rcrange1,rccode1,cdf0,cdf1,cdf2,op[0],ip0,ip1);
    cdfd8(rcrange0,rccode0,rcrange1,rccode1,cdf0,cdf1,cdf2,op[1],ip0,ip1);
    cdfd8(rcrange0,rccode0,rcrange1,rccode1,cdf0,cdf1,cdf2,op[2],ip0,ip1);
    cdfd8(rcrange0,rccode0,rcrange1,rccode1,cdf0,cdf1,cdf2,op[3],ip0,ip1);
  }
  for(; op < out+outlen; op++) 
	cdfd8(rcrange0,rccode0,rcrange1,rccode1,cdf0,cdf1,cdf2,op[0],ip0,ip1);
  return outlen;
}  

size_t rccdfienc8(unsigned char *in, size_t inlen, unsigned char *out) { 
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); CDF16DEC0(cdf2); 
  unsigned char *_op0 = out+4,*op0 = _op0, *_op1 = out+4+(inlen*37/64), *op1=_op1, *ip=in;
  rcencdef(rcrange0,rclow0); rceinit(rcrange0,rclow0);
  rcencdef(rcrange1,rclow1); rceinit(rcrange1,rclow1);
  CDF16DEF; 
  for(; ip < in+(inlen&~(4-1)); ip+= 4) {
	cdfe8(rcrange0,rclow0,rcrange1,rclow1,cdf0,cdf1,cdf2,ip[0],op0,op1);
	cdfe8(rcrange0,rclow0,rcrange1,rclow1,cdf0,cdf1,cdf2,ip[1],op0,op1);
	cdfe8(rcrange0,rclow0,rcrange1,rclow1,cdf0,cdf1,cdf2,ip[2],op0,op1);
	cdfe8(rcrange0,rclow0,rcrange1,rclow1,cdf0,cdf1,cdf2,ip[3],op0,op1);	    OVERFLOWI(in, inlen, op1, op0, _op1);				
  }
  for(; ip < in+inlen; ip++)
	cdfe8(rcrange0,rclow0,rcrange1,rclow1,cdf0,cdf1,cdf2,ip[0],op0,op1);
  rceflush(rcrange0,rclow0, op0);
  rceflush(rcrange1,rclow1, op1);          				
  ctou32(out) = op0-_op0; memcpy(op0,_op1,op1-_op1); op0+=op1-_op1; 			OVERFLOW(in,inlen,out, op0, goto e);  
  e:return op0 - out;
}

//----- Turbo VLC with 7 bits exponent : 16-bits 
size_t rccdfvenc16(unsigned char *_in, size_t _inlen, unsigned char *out) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  bitedef(bw,br); biteinir(bw,br,op_);

  for(ip = in; ip < in+inlen; ip++) {
	unsigned x = ip[0];
	bitvrput(bw,br,op_, 2, 0, x);
	cdfe7(rcrange,rclow,rcrange,rclow, cdf0,cdf1,     x,op,op);
	if(op+8 >= op_) { memcpy(out, _in, _inlen); op = out_; goto e; }
  }
  rceflush(rcrange,rclow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   OVERFLOW(_in,_inlen,out, op, goto e); 
  e:return op - out;   
}       

size_t rccdfvdec16(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *ip_ = in+ctou32(in);
  uint16_t      *out   = (uint16_t *)_out, *op;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);	
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);
  CDF16DEF;
  bitddef(bw, br); bitdinir(bw,br,ip_);
  
  for(op = out; op < out+outlen; op++) {
	unsigned r;
    cdfd7(rcrange,rccode,rcrange,rccode, cdf0,cdf1,      r,ip,ip);	//	    r = BZHI64(r, vlcbits(2));
	bitvrget(bw,br,ip_, 2, 0, r);
	op[0] = r;
  }
  return outlen;
}  

//----- Turbo VLC with 7 bits exponent : 16-bits + zigzag delta
size_t rccdfvzenc16(unsigned char *_in, size_t _inlen, unsigned char *out) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  bitedef(bw,br); biteinir(bw,br,op_);

  for(ip = in; ip < in+inlen; ip++) {
	unsigned x = zigzagenc16(ip[0] - cx);
	bitvrput(bw,br,op_, 2, 0, x);
	cdfe7(rcrange,rclow,rcrange,rclow, cdf0,cdf1,     x,op,op);
	if(op+8 >= op_) { memcpy(out, _in, _inlen); op = out_; goto e; }
	cx = ip[0];
  }
  rceflush(rcrange,rclow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   OVERFLOW(_in,_inlen,out, op, goto e); 
  e:return op - out;   
}       

size_t rccdfvzdec16(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *ip_ = in+ctou32(in);
  uint16_t      *out   = (uint16_t *)_out, *op, cx = 0;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);	
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);
  CDF16DEF;
  bitddef(bw, br); bitdinir(bw,br,ip_);
  
  for(op = out; op < out+outlen; op++) {
	unsigned r;
    cdfd7(rcrange,rccode,rcrange,rccode, cdf0,cdf1, r,ip,ip);					
	bitvrget(bw,br,ip_, 2, 0, r);
	op[0] = (cx+= zigzagdec16(r));
  }
  return outlen;
}

//----- Turbo VLC with 7 bits exponent : 32-bits
size_t rccdfvenc32(unsigned char *_in, size_t _inlen, unsigned char *out) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  bitedef(bw,br); biteinir(bw,br,op_);

  for(ip = in; ip < in+inlen; ip++) {
	unsigned x = ip[0];
	bitvrput(bw,br,op_, 2, 0, x);								
	cdfe7(rcrange,rclow,rcrange,rclow, cdf0,cdf1, x,op,op);
	if(op+8 >= op_) { memcpy(out, _in, _inlen); op = out_; goto e; }
  }
  rceflush(rcrange,rclow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out; 	OVERFLOW(_in,_inlen,out, op, goto e); 
  e:return op - out;   
}       

size_t rccdfvdec32(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *ip_ = in+ctou32(in);
  uint32_t      *out   = (uint32_t *)_out, *op;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);	
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);
  CDF16DEF;
  bitddef(bw, br); bitdinir(bw,br,ip_);
  for(op = out; op < out+outlen; op++) {
	unsigned r; 																//cdfd8(rcrange,rccode,rcrange,rccode, cdf0,cdf1,cdf2, r,ip,ip); 
    cdfd7(rcrange,rccode,rcrange,rccode, cdf0,cdf1, r,ip,ip); 	
	bitvrget(bw,br,ip_, 2, 0, r); 							
	op[0] = r;
  }
  return outlen;
}  

//----- Turbo VLC with 7 bits exponent : 32-bits + zigzag delta
size_t rccdfvzenc32(unsigned char *_in, size_t _inlen, unsigned char *out) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  CDF16DEC0(cdf0); CDF16DEC0(cdf1); //CDF16DEC0(cdf2);
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  bitedef(bw,br); biteinir(bw,br,op_);

  for(ip = in; ip < in+inlen; ip++) {
	unsigned x = zigzagenc32(ip[0] - cx);
	bitvrput(bw,br,op_, 2, 0, x);								
	cdfe7(rcrange,rclow,rcrange,rclow, cdf0,cdf1, x,op,op);
	cx = ip[0];
	if(op+8 >= op_) { memcpy(out, _in, _inlen); op = out_; goto e; }
  }
  rceflush(rcrange,rclow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out; OVERFLOW(_in,_inlen,out, op, goto e); 
  e:return op - out;   
}       

size_t rccdfvzdec32(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *ip_ = in+ctou32(in);
  uint32_t      *out   = (uint32_t *)_out, *op, cx = 0;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);	
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); //CDF16DEC0(cdf2);
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);
  CDF16DEF;
  bitddef(bw, br); bitdinir(bw,br,ip_);
  for(op = out; op < out+outlen; op++) {
	unsigned r; 												
    cdfd7(rcrange,rccode,rcrange,rccode, cdf0,cdf1, r,ip,ip); 	
	bitvrget(bw,br,ip_, 2, 0, r); 							
	op[0] = (cx+= zigzagdec32(r));
   }
  return outlen;
}  

//----- Turbo VLC with 6 bits exponent : 16-bits 
size_t rccdfuenc16(unsigned char *_in, size_t _inlen, unsigned char *out) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  bitedef(bw,br); biteinir(bw,br,op_);

  for(ip = in; ip < in+inlen; ip++) {
	unsigned x = ip[0];
	bitvrput(bw,br,op_, 1, 0, x);
	cdfe6(rcrange,rclow,rcrange,rclow, cdf0,cdf1,     x,op,op);
	if(op+8 >= op_) { memcpy(out, _in, _inlen); op = out_; goto e; }
  }
  rceflush(rcrange,rclow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   OVERFLOW(_in,_inlen,out, op, goto e); 
  e:return op - out;   
}       

size_t rccdfudec16(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *ip_ = in+ctou32(in);
  uint16_t      *out   = (uint16_t *)_out, *op;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);	
  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);
  CDF16DEF;
  bitddef(bw, br); bitdinir(bw,br,ip_);
  
  for(op = out; op < out+outlen; op++) {
	unsigned r;
    cdfd6(rcrange,rccode,rcrange,rccode, cdf0,cdf1,      r,ip,ip);				//r = BZHI64(r, vlcbits(2));
	bitvrget(bw,br,ip_, 1, 0, r);
	op[0] = r;
  }
  return outlen;
}  

//----- Turbo VLC with 6 bits exponent : 32-bits
size_t rccdfuenc32(unsigned char *_in, size_t _inlen, unsigned char *out) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip;
  size_t        inlen = (_inlen+sizeof(in[0])-1)/sizeof(in[0]);

  CDF16DEC0(cdf0); CDF16DEC0(cdf1); 
  CDF16DEF; 
  rcencdef(rcrange,rclow); rceinit(rcrange,rclow);
  bitedef(bw,br); biteinir(bw,br,op_);

  for(ip = in; ip < in+inlen; ip++) {
	unsigned x = ip[0];
	bitvrput(bw,br,op_, 1, 0, x);
	cdfe6(rcrange,rclow,rcrange,rclow, cdf0,cdf1,     x,op,op);
	if(op+8 >= op_) { memcpy(out, _in, _inlen); op = out_; goto e; }
  }
  rceflush(rcrange,rclow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;  OVERFLOW(_in,_inlen,out, op, goto e);  
  e:return op - out;   
}       

size_t rccdfudec32(unsigned char *in, size_t _outlen, unsigned char *_out) {
  unsigned char *ip    = in+4, *ip_ = in+ctou32(in);
  uint32_t      *out   = (uint32_t *)_out, *op;
  size_t        outlen = (_outlen+sizeof(out[0])-1)/sizeof(out[0]);	
  CDF16DEC0(cdf0); CDF16DEC0(cdf1);
  rcdecdef(rcrange,rccode);   rcdinit(rcrange, rccode, ip);
  CDF16DEF;
  bitddef(bw, br); bitdinir(bw,br,ip_);
  
  for(op = out; op < out+outlen; op++) {
	unsigned r;
    cdfd6(rcrange,rccode,rcrange,rccode, cdf0,cdf1, r,ip,ip);
	bitvrget(bw,br,ip_, 1, 0, r);
	op[0] = r;
  }
  return outlen;
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
    cdfenc(rcrange,rclow, cdf, ch, op);											OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t rccdfsmbdec(unsigned char *in, size_t outlen, unsigned char *out, cdf_t *cdf, unsigned cdfnum) {
  unsigned char *ip = in, *op; 
  rcdecdef(rcrange, rccode);         
  
  rcdinit(rcrange, rccode, ip);             div32init();
  
  /*for(int i=0; i <  (1<<(32-RC_BITS)); i++) {
	if(!(i%8)) printf("\n"); 
	printf("{%x,%d},", _div32lut[i].m, _div32lut[i].s);
  }	
  exit(0);*/
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
