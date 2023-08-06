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

// Turbo Range Coder: templates include
// Symbols:   8,16,32 bits integers 16/32 floating point, 
// Coding:    structured/segmented, gamma, rice, Turbo vlc (variable length coding for large integers)
// Order:     0,1,2,sliding context
// predictor: context,zigzag delta, context mixing

#include <string.h>
#include "include/turborc.h"
#include "mb_o0.h"  // order 0 
#include "rcutil_.h"

//---- order 0 byte (8 bits) adaptive - bitwise encode/decode -------------------------------
size_t T3(rc,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                
  MBU_DEC1(mb,1<<8);             			
  
  for(op = out; op < out+outlen; op++) 
    mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1,ip, *op);
  return outlen;
}

size_t T3(rc,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow,rcilow);                  // range coder
  MBU_DEC1(mb,1<<8);             			// predictor
  
  for(ip = in; ip < in+inlen; ip++) {
	unsigned x = ip[0];
    mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1,op, x);      OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}


//--- order0 : 16 bits -----------------------
size_t T3(rc,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       							 
  MBU_DEC1(mb1, 1<<8); 
  MBU_DEC2(mb0, 1<<8, 1<<8);   												 
  
  for(ip = in; ip < in+inlen; ip++) {	 
	uint16_t r = ip[0];
	mb8enc(rcrange,rclow,rcilow, mb1,     RCPRM0,RCPRM1, op, cx=r>>8);      
	mb8enc(rcrange,rclow,rcilow, mb0[cx], RCPRM0,RCPRM1, op, (uint8_t)r); 			
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rc,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned char *ip  = in;
  uint16_t      *out = (uint16_t *)_out, *op, cx;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               					
  MBU_DEC1(mb1, 1<<8);  
  MBU_DEC2(mb0, 1<<8, 1<<8); 								
 
  for(op = out; op < out+outlen; op++) { 
    unsigned ch;               								 		
	mb8dec(rcrange,rccode, mb1,    RCPRM0,RCPRM1, ip, ch); 
	mb8dec(rcrange,rccode, mb0[ch],RCPRM0,RCPRM1, ip, cx); 	
    *op = ch << 8 | cx;  									
  }
  return _outlen;
}

//---Order0 : 32 bits -----------------------
#define XN1 10
size_t T3(rc,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC; 
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC1(mb3,  1<< 8);  
  MBU_DEC2(mb2,  1<< 8,  1<<8);  
  MBU_DEC2(mb1,  1<<XN1, 1<<8);  
  MBU_DEC2(mb0,  1<<XN1, 1<<8);    		  
  
  for(ip = in; ip < in+inlen; ip++) { 
    uint32_t r = ip[0],x; 			    
	                               mb8enc(rcrange,rclow,rcilow, mb3,RCPRM0,RCPRM1, op,        cx =   r>>24 ); 
	mbu *mb = mb2[cx];             mb8enc(rcrange,rclow,rcilow, mb, RCPRM0,RCPRM1, op, x = (uint8_t)(r>>16)); cx = cx << 8 | x;
	     mb = mb1[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow,rcilow, mb, RCPRM0,RCPRM1, op, x = (uint8_t)(r>> 8)); cx = cx << 8 | x;
	     mb = mb0[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow,rcilow, mb, RCPRM0,RCPRM1, op,     (uint8_t)(r    ));
    																						 OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);	
  e:return op - out;
}

size_t T3(rc,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned char *ip = in;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, r;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		
  MBU_DEC1(mb3,  1<< 8);  
  MBU_DEC2(mb2,  1<< 8,  1<<8);  
  MBU_DEC2(mb1,  1<<XN1, 1<<8);  
  MBU_DEC2(mb0,  1<<XN1, 1<<8);  
 
  for(op = out; op < out+outlen; op++) { 							
	                              mb8dec(rcrange,rccode, mb3,RCPRM0,RCPRM1, ip, r);
	mbu *mb = mb2[r];             mb8dec(rcrange,rccode, mb, RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx;
         mb = mb1[BZHI32(r,XN1)]; mb8dec(rcrange,rccode, mb, RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx; 
		 mb = mb0[BZHI32(r,XN1)]; mb8dec(rcrange,rccode, mb, RCPRM0,RCPRM1, ip,cx); 
    *op = cx = r << 8 | cx; 													
  }
  return _outlen;
}

//---- Order 0 Nibble (4 bits) adaptive : encode/decode low nibble ---------------------------
size_t T3(rc4,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC1(mb,1<<4);             			     
  
  for(ip = in; ip < in+inlen; ip++) {
    mb4enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1,op, ip[0]&0xf); OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rc4,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                    
  MBU_DEC1(mb,1<<4);             			     
  
  for(op = out; op < out+outlen; op++)
    mb4dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip, *op); 
  return outlen;
}

// Order 0 Nibble (4 bits) --------------------------------------------------------------------
size_t T3(rc4c,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC1(mb,1<<4);             			     
  
  for(ip = in; ip < in+inlen; ip++) {
    mb4senc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op, ip[0]&0xf); OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rc4c,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                
  MBU_DEC1(mb,1<<4);             			
  
  for(op = out; op < out+outlen; op++)
    mb4sdec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip, *op); 
  return outlen;
}

//-- Order1 : 8 bits -------------------------------------------------------------------------
size_t T3(rcc,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow,rcilow);                    
  MBU_DEC2(mb,1<<8, 1<<8);             		  
  
  for(ip = in; ip < in+inlen; ip++) {
    mb8enc(rcrange,rclow,rcilow, mb[cx], RCPRM0,RCPRM1, op, cx = ip[0]); 	 OVERFLOW(in,inlen,out, op, goto e);    
  } 
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op - out;
}

size_t T3(rcc,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *op; 
  unsigned      cx = 0;
  rcdecdec(rcrange,rccode, ip);                   
  MBU_DEC2(mb, 1<<8, 1<<8);             	   

  for(op = out; op < out+outlen; op++)
    mb8dec(rcrange,rccode, mb[cx],RCPRM0,RCPRM1, ip, cx = *op);
  return outlen;
}

//---Order1 : 16 bits -----------------------
size_t T3(rcc,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0, ema = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       							 
  MBU_DEC2( mb1, 1<< 8, 1<<8); 
  MBU_NEWI2(mb0, 1<<16, 1<<8);  												 
  
  for(ip = in; ip < in+inlen; ip++) {	 
	uint16_t r = ip[0];
	mb8enc(rcrange,rclow,rcilow, mb1[cx], RCPRM0,RCPRM1, op, r>>8);       cx = cx <<8 | r>>8;	
	mb8enc(rcrange,rclow,rcilow, &mb0[cx*(1<<8)], RCPRM0,RCPRM1, op, (uint8_t)r); 			
	cx = (uint8_t)cx;
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:free(mb0); return op - out;
}

size_t T3(rcc,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned char *ip    = in;
  uint16_t      *out   = (uint16_t *)_out, *op, cx = 0, ema = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               					
  MBU_DEC2( mb1, 1<<8,  1<<8);  
  MBU_NEWI2(mb0, 1<<16, 1<<8); //MBU_DEC2(mb0, 1<<16, 1<<8); 								
 
  for(op = out; op < out+outlen; op++) { 
    unsigned ch;               								
	mb8dec(rcrange,rccode, mb1[cx],RCPRM0,RCPRM1, ip, ch); 
	mb8dec(rcrange,rccode, &mb0[(cx<<8 | ch)*(1<<8)],RCPRM0,RCPRM1, ip, cx); 	
    *op = ch << 8 | cx;  cx = ch; 						
  }
  free(mb0);
  return _outlen;
}

//---Order 7bs  : 32 bits integer, 32 bits floating point -----------------------
#define XNS       8
#define XN        (XNS-1) // ignore MSB/sign
#define CX32(_x_) BZHI32( (_x_)>>(32-XN), XN)
#define CX0(_x_)  (uint8_t)(_x_)
#define XN1 10
size_t T3(rcc,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb3,  1<<XN, 1<<8);  
  MBU_DEC2(mb2,  1<< 8, 1<<8);  
  MBU_DEC2(mb1, 1<<XN1, 1<<8);  
  MBU_DEC2(mb0, 1<<XN1, 1<<8);     			  
  
  for(ip = in; ip < in+inlen; ip++) { 
	uint32_t r = ip[0],x; 			    
	mbu *mb = mb3[cx = CX32(cx)];  mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op,        cx =   r>>24 ); 
	     mb = mb2[cx];             mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>>16)); cx = cx << 8 | x;
	     mb = mb1[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>> 8)); cx = cx << 8 | x;
	     mb = mb0[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op,     (uint8_t)(r    ));
    cx = r;																						 OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);														
  e:return op - out;
}

size_t T3(rcc,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned char *ip = in;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, r;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		
  MBU_DEC2(mb3,  1<<XN, 1<<8);  
  MBU_DEC2(mb2,  1<< 8,  1<<8);  
  MBU_DEC2(mb1,  1<<XN1, 1<<8);  
  MBU_DEC2(mb0,  1<<XN1, 1<<8);  
 
  for(op = out; op < out+outlen; op++) { 							
	mbu *mb = mb3[cx = CX32(cx)]; mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip, r);
	     mb = mb2[r];             mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx;
         mb = mb1[BZHI32(r,XN1)]; mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx; 
		 mb = mb0[BZHI32(r,XN1)]; mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); 
    *op = cx = r << 8 | cx; 													
  }
  return _outlen;
}

//---Order Nb : 32 bits integer + 32 bits floating point -----------------------
#define XNS       12
#define XN        (XNS-1)
#define CX32(_x_) (((_x_)>>(32-XNS)) & ((1u<<(XN))-1))
size_t T3(rcc2,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                     
  MBU_DEC2(mb3,  1<<XN,  1<<8);  
  MBU_DEC2(mb2,  1<< 8,  1<<8);   
  MBU_NEWI2(mb1, 1<<XN1, 1<<8); 
  MBU_NEWI2(mb0, 1<<XN1, 1<<8);

  for(ip = in; ip < in+inlen; ip++) { 
    uint32_t r = ip[0],x; 			    	
	mbu *mb = mb3[cx = CX32(cx)];          mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op,        cx =   r>>24 ); 
	     mb = mb2[cx];                     mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>>16)); cx = cx << 8 | x;
	     mb = &mb1[BZHI32(cx,XN1)*(1<<8)]; mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>> 8)); cx = cx << 8 | x;
	     mb = &mb0[BZHI32(cx,XN1)*(1<<8)]; mb8enc(rcrange,rclow,rcilow, mb,RCPRM0,RCPRM1, op,     (uint8_t)(r    ));
    cx = r;																						 OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);														
  e:free(mb1);free(mb0); return op - out;
}

size_t T3(rcc2,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned char *ip = in;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, r;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		
  MBU_DEC2( mb3, 1<<XN, 1<<8);  
  MBU_DEC2( mb2, 1<< 8, 1<<8);  
  MBU_NEWI2(mb1, 1<<XN1, 1<<8);  
  MBU_NEWI2(mb0, 1<<XN1, 1<<8);
 
  for(op = out; op < out+outlen; op++) { 							
	mbu *mb = mb3[cx = CX32(cx)];         mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip, r);
	     mb = mb2[r];                     mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx;
         mb = &mb1[BZHI32(r,XN1)*(1<<8)]; mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx; 
		 mb = &mb0[BZHI32(r,XN1)*(1<<8)]; mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); 
    *op = cx = r << 8 | cx; 													
  }
  free(mb1);free(mb0);
  return _outlen;
}

//-- Order 2 --------------------------------------------------------------------------
size_t T3(rcc2,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow,rcilow);  
  MBU_NEWI2(mb, 1<<16, 1<<8);         		  
  
  for(ip = in; ip < in+inlen; ip++) {
    mb8enc(rcrange,rclow,rcilow, &mb[cx*(1<<8)],RCPRM0,RCPRM1, op, ip[0]); 
	cx = (uint16_t)(cx<<8 | ip[0]);						    OVERFLOW(in,inlen,out, op, goto e);    
  } 
  rceflush(rcrange,rclow,rcilow, op); 
  e:free(mb); return op - out;
}

size_t T3(rcc2,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *op; 
  unsigned      cx = 0;
  rcdecdec(rcrange,rccode, ip);                    
  MBU_NEWI2(mb, 1<<16, 1<<8);             		  

  for(op = out; op < out+outlen; op++) { 
    mb8dec(rcrange,rccode, &mb[cx*(1<<8)],RCPRM0,RCPRM1, ip, op[0]);
	cx  = (uint16_t)(cx<<8 | op[0]);	
  }
  free(mb);
  return outlen;
}

//-- order N bits with sliding context ---------------------------------------------------------
#include "mb_on.h"

#define MBC_C 8     // order 8 bits context (4 <= MBC_C <= 20)
size_t T3(rcx,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow,rcilow);                 
  MBC_DEC(mbc, MBC_C);                     	// predictor with MBC_C context bits 
  
  for(ip = in; ip < in+inlen; ip++) { 
    mbcenc(rcrange,rclow,rcilow, cx, MBC_C, mbc,RCPRM0,RCPRM1,op, *ip); OVERFLOW(in,inlen,out, op, goto e);
  } 
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op - out;
}

size_t T3(rcx,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned      char *ip = in, *op; 
  unsigned      cx = 0;
  rcdecdec(rcrange,rccode, ip);                     
  MBC_DEC(mbc, MBC_C);                           

  for(op = out; op < out+outlen; op++) { 
    mbcdec(rcrange,rccode, cx, MBC_C, mbc,RCPRM0,RCPRM1,ip);
    *op = cx; 
  }
  return outlen;
}

#define MBC_C2 15
  #if RC_PRDID == 1
int mbc_c = 15;
void mbcset(unsigned m) { mbc_c = m; if(mbc_c > MBC_C2) mbc_c = MBC_C2; if(mbc_c < 4) mbc_c = 4; } // set context bits length 1-16
  #else
extern int mbc_c;
  #endif

size_t T3(rcx2,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow,rcilow);                       
  MBC_DEC(mbc, MBC_C2);                         
  
  for(ip = in; ip < in+inlen; ip++) { 
    mbcenc(rcrange,rclow,rcilow, cx, mbc_c, mbc,RCPRM0,RCPRM1,op, *ip); OVERFLOW(in,inlen,out, op, goto e);
  } 
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op - out;
}

size_t T3(rcx2,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned      char *ip = in, *op; 
  unsigned      cx = 0;
  rcdecdec(rcrange,rccode, ip);                     
  MBC_DEC(mbc, MBC_C2);                          

  for(op = out; op < out+outlen; op++) { 
    mbcdec(rcrange,rccode, cx, mbc_c, mbc,RCPRM0,RCPRM1,ip);
    *op = cx; 
  }
  return outlen;
}

//*************************************** Variable Length Coding : Gamma, Rice, Turbo VLC **************************************************
#include "mb_vint.h"

#define IN0 3
#define IN1 5
#define IN2 8
size_t T3(rcu3,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow,rcilow);                       
  MBU3_DEC(mbf, mb0,IN0, mb1,IN1, mb2,IN2);
  
  for(ip = in; ip < in+inlen; ip++) { 
	mbu3enc(rcrange,rclow,rcilow, mbf,mb0,IN0,mb1,IN1,mb2,IN2, RCPRM0,RCPRM1, op, ip[0]);  OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcu3,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                     
  MBU3_DEC(mbf, mb0,IN0, mb1,IN1, mb2,IN2);
  
  for(op = out; op < out+outlen; op++)
	mbu3dec(rcrange,rccode, mbf,mb0,IN0,mb1,IN1,mb2,IN2,RCPRM0,RCPRM1, ip, op[0]);
  return outlen;
}

//------------------------------ Gamma coding for 8, 16 and 32 bits integers --------------------------------------
// usage: Run length in RLE and BWT(MTF, QLFC), match/literal length in lz77, integer arrays with small values, large alphabet
size_t T3(rcg,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t *op = out, *ip;
  rcencdec(rcrange,rclow,rcilow);                      
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(ip = in; ip < in+inlen; ip++) {
    mbgenc(rcrange,rclow,rcilow, &mbg0c,mbguc,mbgbc, RCPRM0,RCPRM1,op, ip[0]); OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcg,RC_PRD,dec8)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                    
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(op = out; op < out+outlen; op++)
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, op[0]);
  return outlen;
}

size_t T3(rcg,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 32, 32); 
   
  for(ip = in; ip < in+inlen; ip++)  {      
    mbgenc(rcrange,rclow,rcilow, &mbg0, mbgu,mbgb,RCPRM0,RCPRM1,op, ip[0]); OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op - out;
}

size_t T3(rcg,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint16_t      *out = _out, *op;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               
  MBG_DEC(mbg0c, mbguc, mbgbc, 32, 32); 
 
  for(op = out; op < out+outlen; op++)
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, op[0]);
  return _outlen;
}

#define GQMAX32 12   // Length limited gamma coding
size_t T3(rcg,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       	
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);    
  
  for(ip = in; ip < in+inlen; ip++) {
    mbgenc32(rcrange,rclow,rcilow, mbgu,mbgb,RCPRM0,RCPRM1,op, ip[0], GQMAX32); OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op - out;
}

size_t T3(rcg,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint32_t      *out = _out, *op;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 

  for(op = out; op < out+outlen; op++)  
    mbgdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, op[0], GQMAX32);
  return _outlen;
}

//----- Gamma Coding + Zigzag Delta ---------------------------------
size_t T3(rcgz,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t  *op = out, *ip, cx = 0; 
  rcencdec(rcrange,rclow,rcilow);                        
  MBG_DEC(mbg0c, mbguc, mbgbc, 32, 32); 
  
  for(ip = in; ip < in+inlen; ip++) {         
    mbgenc(rcrange,rclow,rcilow, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,op, zigzagenc8(ip[0] - cx));
	cx = ip[0];								                                             OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcgz,RC_PRD,dec8)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint8_t       *ip = in, cx = 0; 
  rcdecdec(rcrange,rccode, ip);                
  MBG_DEC( mbg0c, mbguc, mbgbc, 32, 32);           
  
  for(op = out; op < out+outlen; op++) { 
    unsigned x;
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, x);
    *op = cx += zigzagdec8(x);
  }
  return outlen;
}

size_t T3(rcgz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC( mbg0c, mbguc, mbgbc, 32, 32); 				 
  
  for(ip = in; ip < in+inlen; ip++) {        
    mbgenc(rcrange,rclow,rcilow, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,op, zigzagenc16(ip[0] - cx));  OVERFLOW(_in,_inlen,out, op, goto e);
	cx = ip[0];												    
  }
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op - out;
}

size_t T3(rcgz,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint16_t      *out = (uint16_t *)_out, *op, cx = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);                
  MBG_DEC(mbg0c, mbguc, mbgbc, 32, 32);
 
  for(op = out; op < out+outlen; op++) { 
    unsigned x;
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, x);
    *op = (cx += zigzagdec16(x));
  }
  return _outlen;
}

size_t T3(rcgz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                        
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 				// predictor gamma
  
  for(ip = in; ip < in+inlen; ip++) {         
    mbgenc32(rcrange,rclow,rcilow, mbguc,mbgbc,RCPRM0,RCPRM1,op, zigzagenc32(ip[0] - cx), GQMAX32); OVERFLOW(_in,_inlen,out, op, goto e);
	cx = ip[0];    
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcgz,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33);

  for(op = out; op < out+outlen; op++) { 
    uint32_t x;
    mbgdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, GQMAX32);
	*op = (cx += zigzagdec32(x)); 
  }
  return _outlen;
}

//************ Variable Length Coding : Adaptive Length limited Rice coding for 8, 16 and 32 bits integers ************************
#define RICEMAX 12

size_t T3(rcr,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t *op = out, *ip; 
  unsigned ema = 0;
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(ip = in; ip < in+inlen; ip++) {         
    unsigned x = ip[0], log2m = RICEK(ema); 
    mbrenc32(rcrange,rclow,rcilow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX, log2m); 
	ema = EMA(6, ema, 63, x);					    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcr,RC_PRD,dec8)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  unsigned ema = 0;
  rcdecdec(rcrange,rccode, ip);                     
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema);
    mbrdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, RICEMAX,log2m); ema = EMA(6,ema, 63, x);
    *op = x; 
  }
  return outlen;
}

size_t T3(rcr,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip;
  INDEC;
  unsigned      ema = 0;  
  rcencdec(rcrange,rclow,rcilow);                         
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33); 		 
  
  for(ip = in; ip < in+inlen; ip++) {         
    unsigned x = ip[0], log2m = RICEK(ema);
    mbrenc32(rcrange,rclow,rcilow, mbgu,mbgb,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema = EMA(6,ema, 63, x);						    
	OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcr,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint16_t      *out = _out, *op;
  OUTDEC;
  unsigned      ema = 0;
  rcdecdec(rcrange,rccode, ip);                
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
 
  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema);
    mbrdec32(rcrange,rccode, mbgu,mbgb,RCPRM0,RCPRM1,ip, x, RICEMAX, log2m); 
    *op = x; 
	ema = EMA(6,ema, 63, x);
  }
  return _outlen;
}

#define CXR(x) (unsigned char)(x>>23)
size_t T3(rcr,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip;
  INDEC;
  unsigned      ema[1<<9] = {0}, cx = 0;
  rcencdec(rcrange,rclow,rcilow);                       		  
  MBG_DEC(mbg0c, mbguc, mbgbc, 46, 46); 				 

  for(ip = in; ip < in+inlen; ip++) {         
    uint32_t x = ip[0], log2m = RICEK(ema[CXR(cx)]);   
    mbrenc32(rcrange,rclow,rcilow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	cx = ip[0];
	ema[CXR(cx)] = EMA(8, (uint64_t)ema[CXR(cx)], 255, (uint64_t)x);				    OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcr,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint32_t      *out = (uint32_t *)_out, *op;
  OUTDEC;
  unsigned      ema[1<<9] = {0}, cx = 0;
  rcdecdec(rcrange,rccode, ip);              
  MBG_DEC(mbg0c, mbguc, mbgbc, 46, 46);    

  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema[CXR(cx)]);   
    mbrdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, RICEMAX,log2m); 
	cx = x;
	ema[CXR(cx)] = EMA(8, (uint64_t)ema[CXR(cx)], 255, (uint64_t)x);
    *op = x;
  }
  return _outlen;
}

//-- Rice + delta zigzag ------------------------------------
size_t T3(rcrz,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t  *op = out, *ip, cx = 0; 
  unsigned  ema = 0;
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(ip = in; ip < in+inlen; ip++) {         
    uint8_t  d = ip[0] - cx;
	unsigned x = zigzagenc8(d), log2m = RICEK(ema); 
    mbrenc32(rcrange,rclow,rcilow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema  = EMA(4, ema, 15, x); 
	cx = ip[0];									    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcrz,RC_PRD,dec8)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint8_t       *ip = in, cx = 0; 
  unsigned      ema = 0; 
  rcdecdec(rcrange,rccode, ip);                        
  MBG_DEC( mbg0c, mbguc, mbgbc, 33, 33);         
  
  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema);
    mbrdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, RICEMAX,log2m); 
    uint8_t d = zigzagdec8(x); 
	*op = (cx += d); 
	ema = EMA(4, ema, 15, x); 
  }
  return outlen;
}

size_t T3(rcrz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  unsigned      ema  = 0;                                
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC( mbg0c, mbguc, mbgbc, 33, 33); 				 
 
  for(ip = in; ip < in+inlen; ip++) {        
    uint16_t d = ip[0] - cx;
	unsigned x = zigzagenc16(d), log2m = RICEK(ema); 
    mbrenc32(rcrange,rclow,rcilow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema  = EMA(6,ema, 63, x); 	
	cx = ip[0];					
	OVERFLOW(_in,_inlen,out, op, goto e);
  } 
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcrz,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint16_t      *out = (uint16_t *)_out, *op, cx = 0;
  OUTDEC;
  unsigned      ema = 0;
  rcdecdec(rcrange,rccode, ip);              
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33);
 
  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema);
    mbrdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, RICEMAX,log2m); 
    uint16_t d   = zigzagdec16(x); 
	*op = (cx += d); 
	ema = EMA(6,ema, 63, x);
  }
  return _outlen;
}

size_t T3(rcrz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  unsigned      ema = 0;
  rcencdec(rcrange,rclow,rcilow);                      
  MBG_DEC(mbg0c, mbguc, mbgbc, 64, 64); 		 
  
  for(ip = in; ip < in+inlen; ip++) {         
    uint32_t d = ip[0] - cx;														
	unsigned x = zigzagenc32(d), log2m = RICEK(ema);  
    mbrenc32(rcrange,rclow,rcilow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema  = EMA(6,(uint64_t)ema, 63, (uint64_t)x); 
	cx = ip[0];						    OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op);
  e:return op - out;
}

size_t T3(rcrz,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint32_t      *out = _out, *op, cx = 0;
  OUTDEC;
  unsigned      ema = 0;
  rcdecdec(rcrange,rccode, ip);                    
  MBG_DEC(mbg0c, mbguc, mbgbc, 64, 64);
  
  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema);
    mbrdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, RICEMAX,log2m); 
    uint32_t d = zigzagdec32(x); 
	*op = (cx += d);
	ema = EMA(6, (uint64_t)ema, 63, (uint64_t)x);
  }
  return _outlen;
}

//-------------------- RLE : Run Length Encoding in gamma+rc ----------------------------------
// characters encoded using the current character as order-8bits sliding context
// Run length are encoded in gamma coding, 
size_t T3(rcrle,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *in_ = in+inlen, *op = out; 
  rcencdec(rcrange,rclow,rcilow);                     
  MBU_DEC1(mb,1<<8); 
  MBG_DEC(mbg0, mbgu, mbgb, 32, 32);                    // run length predictor

  while(ip < in_) { 
    uint8_t *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; // run length

    mb8enc(rcrange,rclow,rcilow,              mb,RCPRM0,RCPRM1,op, u);
    mbgenc(rcrange,rclow,rcilow, &mbg0,mbgu,mbgb,RCPRM0,RCPRM1,op, r-1);
    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op-out;
}

size_t T3(rcrle,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op = out; 
  rcdecdec(rcrange,rccode, ip);                     
  MBU_DEC1(mb,1<<8); 
  MBG_DEC(mbg0, mbgu, mbgb, 32, 32);                    

  while(op < out+outlen) { 
    unsigned r,ch;
    mb8dec(rcrange,rccode,              mb,RCPRM0,RCPRM1,ip, ch);
    mbgdec(rcrange,rccode, &mbg0,mbgu,mbgb,RCPRM0,RCPRM1,ip, r);
    memset_(op, ch, r+1);
  }
  return ip-in;   
}

size_t T3(rcrle,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op   = out;
  uint16_t      *in   = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       							 
  MBU_DEC1(mb1, 1<< 8); 
  MBU_DEC2(mb0, 1<< 8, 1<<8);   												 
  MBG_DEC1(mbg0,1<<16, mbgu, 1<<8, mbgb, 33, 33);

  uint16_t *in_ = in+inlen;																		
  for(ip = in; ip < in_;) {
	uint16_t *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; // run length			
	mb8enc(rcrange,rclow,rcilow, mb1,     RCPRM0,RCPRM1, op, u>>8);       cx = u>>8;	
	mb8enc(rcrange,rclow,rcilow, mb0[cx], RCPRM0,RCPRM1, op, (uint8_t)u); 			
    mbgenc(rcrange,rclow,rcilow, &mbg0[u],mbgu[u>>8],mbgb,RCPRM0,RCPRM1,op, r-1);  		
  }
  rceflush(rcrange,rclow,rcilow, op); 														
  e:return op - out;
}

size_t T3(rcrle,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned char *ip    = in;
  uint16_t      *out   = (uint16_t *)_out, *op, cx = 0, ema = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               					 
  MBU_DEC1(mb1,  1<< 8);  
  MBU_DEC2(mb0,  1<< 8, 1<<8); 								
  MBG_DEC1(mbg0, 1<<16, mbgu, 1<<8, mbgb, 33, 33);
 
  for(op = out; op < out+outlen;) { 
    unsigned ch,r;
	mb8dec(rcrange,rccode, mb1,    RCPRM0,RCPRM1, ip, cx); 
	mb8dec(rcrange,rccode, mb0[cx],RCPRM0,RCPRM1, ip, ch);
    uint16_t u = cx << 8 | ch;
    mbgdec(rcrange,rccode, &mbg0[u], mbgu[u>>8], mbgb, RCPRM0,RCPRM1,ip, r);
	memset_(op, u, r+1);
  }
  return _outlen;
}

// Character  : encoded as order-8bits sliding context
// Run length : gamma coding with current character as context 
enum { RU0=5+8,RU=5+8,RB=8 };                                                                         
#define HISTPRED(_x_)      RICEK(_x_)
#define HISTUPD(_avg_,_x_) EMA(5, _avg_,23,(_x_)>31?31:(_x_))                  // max. avg 31

size_t T3(rcrle1,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) { 
  unsigned      cx = 0;
  unsigned char *ip = in, *in_ = in+inlen, *op = out; 
  rcencdec(rcrange,rclow,rcilow);                         
  MBU_DEC2(mb, 1<<8, 1<<8);
  MBG_DEC2(mbg0r, 1<<RU0, mbgur, 1<<RU, mbgbr, 1<<RB, 33, 33);  				//run length //MBG_DEC2(mbg0, 1<<8, mbgu, 1<<8, mbgb, 1<<8, 32, 32); 
  uint8_t hist[1<<8] = {1};
    
  while(ip < in_) { 
    unsigned char *p = ip, u = *ip++; 
	while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p,cr; r--; // r:run length
	mbu *m = mb[cx];        mb8enc(rcrange,rclow,rcilow, m,                                          RCPRM0,RCPRM1,op, u); cx = u; //cx = (cx<<8|u)&0xfff; cx = (cx<<8|(uint8_t)ch)&0xfff;
	cr = HISTPRED(hist[u]); mbgxenc(rcrange,rclow,rcilow, &mbg0r[cr<<8|u], mbgur[cr<<8|u], mbgbr[u], RCPRM0,RCPRM1,op, r); hist[u] = HISTUPD(hist[u],r);//mbgxenc(rcrange,rclow,rcilow, &mbg0[u], mbgu[u], mbgb[u], RCPRM0,RCPRM1,op, r);
    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op-out;
}

size_t T3(rcrle1,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned      cx = 0;
  unsigned char *ip = in, *op = out; 
  rcdecdec(rcrange,rccode, ip);                                
  MBU_DEC2(mb, 1<<8, 1<<8);
  MBG_DEC2(mbg0r, 1<<RU0, mbgur, 1<<RU, mbgbr, 1<<RB, 33, 33);  				//run length  MBG_DEC2(mbg0,  1<<8, mbgu, 1<<8, mbgb, 1<<8, 33, 33);  
  uint8_t hist[1<<8] = {1};

  while(op < out+outlen) { 
    unsigned r,cr;
    unsigned char ch;
    mbu *m = mb[cx];                   mb8dec(rcrange,rccode,  m,                                            RCPRM0,RCPRM1,ip, ch); cx = (uint8_t)ch;
    ch = cx; cr = HISTPRED(hist[ch]); mbgxdec(rcrange,rccode, &mbg0r[cr<<8|ch], mbgur[cr<<8|ch], mbgbr[ch], RCPRM0,RCPRM1,ip, r); hist[ch] = HISTUPD(hist[ch],r); //ch = cx;         mbgdec(rcrange,rccode, &mbg0[ch], mbgu[ch], mbgb[ch], RCPRM0,RCPRM1,ip, r);
    memset_(op, ch, r+1);
  }
  return ip-in;   
}

size_t T3(rcrle1,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op   = out;
  uint16_t      *in   = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       							 
  MBU_DEC2(mb1, 1<< 8, 1<<8); 
  MBU_DEC2(mb0, 1<< 8, 1<<8);   												 
  MBG_DEC1(mbg0,1<<16, mbgu, 1<<8, mbgb, 33, 33);
  uint16_t *in_ = in+inlen;	

  for(ip = in; ip < in_;) {
	uint16_t *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; 
	mb8enc(rcrange,rclow,rcilow, mb1[cx], RCPRM0,RCPRM1, op, u>>8);       cx = u>>8;	
	mb8enc(rcrange,rclow,rcilow, mb0[cx], RCPRM0,RCPRM1, op, (uint8_t)u); 			
    mbgenc(rcrange,rclow,rcilow, &mbg0[u],mbgu[u>>8],mbgb,RCPRM0,RCPRM1,op, r-1);
  }
  rceflush(rcrange,rclow,rcilow, op); 
  e:return op - out;
}

size_t T3(rcrle1,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned char *ip    = in;
  uint16_t      *out   = (uint16_t *)_out, *op, cx = 0, ema = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               					 
  MBU_DEC2(mb1,  1<< 8, 1<<8);  
  MBU_DEC2(mb0,  1<< 8, 1<<8); 								
  MBG_DEC1(mbg0, 1<<16, mbgu, 1<<8, mbgb, 33, 33);
 
  for(op = out; op < out+outlen;) { 
    unsigned ch,r;
	mb8dec(rcrange,rccode, mb1[cx],RCPRM0,RCPRM1, ip, cx); 
	mb8dec(rcrange,rccode, mb0[cx],RCPRM0,RCPRM1, ip, ch);
    uint16_t u = cx << 8 | ch;
    mbgdec(rcrange,rccode, &mbg0[u], mbgu[u>>8], mbgb, RCPRM0,RCPRM1,ip, r);
	memset_(op, u, r+1);
  }
  return _outlen;
}


//-------- Turbo VLC: Variable Length Code for large integers: 16+32 bits similar to µ-Law/Extra Bit Codes encoding ------------------------
// Usage: Lz77 Offsets, Large alphabet/ Integers
#include "include_/vlcbit.h"

#define OV 4
#define OVERFLOWR(_in_, _inlen_, _p_,_p__)	if(_p_+12 > _p__) { memcpy(out, _in_, _inlen_); op = out_; goto e; }

#define CXN 8
#define CX16(_x_) ((_x_)>>8)

// 16 bits  -----------
size_t T3(rcv,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0, vb=VLC_VB8;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VLC_VN8));
  bitedef(bw,br); biteinir(bw,br,op_);
 
  for(cx=0,ip = in; ip < in+inlen; ip++) {
	uint16_t x = ip[0];
	bitvrput(bw,br,op_, VLC_VN8, VLC_VB8, x); 
	mb8enc(rcrange,rclow,rcilow, mb1[CX16(cx)],RCPRM0,RCPRM1, op, x); 
	cx = ip[0];									    							OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; 
  memmove(op, op_, l); op += l; ctou32(out) = op-out;   						OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcv,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint16_t      *out = (uint16_t *)_out, *op, cx = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		 
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VLC_VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);  

  for(op = out; op < out+outlen; op++) { 
    unsigned x;
    mb8dec(rcrange,rccode, mb1[CX16(cx)],RCPRM0,RCPRM1, ip,x);
	bitvrget(bw,br,ip_, VLC_VN8, VLC_VB8, x);
    *op = cx = x;
  }
  return _outlen;
}

//---- Turbo vlc: 16 bits zigzag delta --------------
size_t T3(rcvz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VLC_VN8));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint16_t x = zigzagenc16(ip[0] - cx);  
	bitvrput(bw,br,op_, VLC_VN8, VLC_VB8, x); 
	mb8enc(rcrange,rclow,rcilow, mb1[CX16(cx)],RCPRM0,RCPRM1, op, x); 
	cx = ip[0];									  OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcvz,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint16_t      *out = (uint16_t *)_out, *op, cx = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		 
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VLC_VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);  

  for(op = out; op < out+outlen; op++) { 
    unsigned x;               			
    mb8dec(rcrange,rccode, mb1[CX16(cx)],RCPRM0,RCPRM1, ip,x);
	bitvrget(bw,br,ip_, VLC_VN8, VLC_VB8, x);
    *op = (cx += zigzagdec16(x));
  }
  return _outlen;
}

// Turbo vlc 32 bits -----------

  #ifndef max
#define max(x,y) (((x)>(y)) ? (x) : (y))
  #endif
#define EXPVB(_l_,_vn_,_vm_)  (_vm_-1-vlcexpo(_l_,_vn_))
static unsigned expvb32(uint32_t *in, size_t inlen, unsigned vn, unsigned vm) {
  uint32_t *ip;
  unsigned cx = 0;
  for(ip = in; ip < in+inlen; ip++) cx = max(ip[0],cx);
  return EXPVB(cx, vn, vm);
}

#define CX32(_x_) ((_x_)>>24)
//#define VLC_VN8 2
size_t T3(rcv,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+5, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0, vb = VLC_VB8;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN8));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  vb = expvb32(in, inlen, VLC_VN8, 256);  if(vb > 0xff) vb = 0xff;
  out[4] = vb;
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = ip[0];
	bitvrput(bw,br,op_, VLC_VN8, vb, x);	
	mb8enc(rcrange,rclow,rcilow, mb[cx],RCPRM0,RCPRM1, op, x);   AC(x < 256, "rcv/F: expo");
	//cx = x; //vlcmbits(x, VLCXN); //x - (x < vlcfirst(VLC_VN8)+vb)?0:x;									    
	OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcv,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+5,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, x, vb = in[4];
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               				 
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) { 
    mb8dec(rcrange,rccode, mb[cx],RCPRM0,RCPRM1, ip, x);  
	//cx = x; // - (x < vlcfirst(VLC_VN8)+vb)?0:x;
	bitvrget(bw,br,ip_, VLC_VN8, vb, x);
    *op = x;
  }
  return _outlen;
}

// Turbo vlc 32 bits + zigzag delta -----------
size_t T3(rcvz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;  
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN8));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = zigzagenc32(ip[0] - cx); 					
	bitvrput(bw,br,op_, VLC_VN8, VLC_VB8, x); 
	mb8enc(rcrange,rclow,rcilow, mb[CX32(cx)],RCPRM0,RCPRM1, op, x);   
	cx = ip[0];									    OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcvz,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, x;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);  	

  for(op = out; op < out+outlen; op++) { 
    mb8dec(rcrange,rccode, mb[CX32(cx)],RCPRM0,RCPRM1, ip,x);
	bitvrget(bw,br,ip_, VLC_VN8, VLC_VB8, x);
    *op = (cx += zigzagdec32(x));														
  }
  return _outlen;
}

//---- Turbo VLC with exponent coded in gamma and mantissa in bitio -----------------------------------
#define VNG VLC_VN8
#define VBG VLC_VB8
size_t T3(rcvg,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;  
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33); 			 
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint16_t x = ip[0];
	mbvenc(rcrange,rclow,rcilow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VNG, VBG); bitenormr(bw,br,op_);
	cx = ip[0];																		  OVERFLOWR(_in, _inlen,op,op_);										    
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcvg,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint16_t      *out = (uint16_t *)_out, *op, cx = 0, x;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		 
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) { 
	bitdnormr(bw,br,ip_); mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VNG, VBG);
    *op = cx = x;
  }
  return _outlen;
}

size_t T3(rcvgz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                      
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
  bitedef(bw,br); biteinir(bw,br,op_); 
  
  for(ip = in; ip < in+inlen; ip++) {
	uint16_t x = zigzagenc16(ip[0] - cx);
	mbvenc(rcrange,rclow,rcilow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VNG, VBG); bitenormr(bw,br,op_);	
	cx = ip[0];																		  OVERFLOWR(_in, _inlen,op,op_);			    
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcvgz,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint16_t      *out = (uint16_t *)_out, *op, cx = 0, x;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);              		 
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33); 
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) {    									
	bitdnormr(bw,br,ip_); mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VNG, VBG);
    *op = (cx += zigzagdec16(x));												
  }
  return _outlen;
}

size_t T3(rcvg,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33); 
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = ip[0];
	mbvenc(rcrange,rclow,rcilow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VNG, VBG); bitenormr(bw,br,op_);
	cx = ip[0];					OVERFLOWR(_in, _inlen,op,op_);				    
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcvg,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, x;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		 
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) { 
	bitdnormr(bw,br,ip_); mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VNG, VBG);
    *op = cx = x;
  }
  return _outlen;
}

size_t T3(rcvgz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = zigzagenc32(ip[0] - cx);
	mbvenc(rcrange,rclow,rcilow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VNG, VBG); 
	bitenormr(bw,br,op_);	
	cx = ip[0];																	      OVERFLOWR(_in, _inlen,op,op_);			    
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcvgz,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, x;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		 
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
  bitddef(bw, br); bitdinir(bw,br,ip_); 

  for(op = out; op < out+outlen; op++) { 
	bitdnormr(bw,br,ip_); 
	mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VNG, VBG);
    *op = (cx += zigzagdec32(x));
  }
  return _outlen;
}

//---- Turbo VLC - 32 bits: Large exponent -----------------------
size_t T3(rcv10,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN10));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = ip[0];
	bitvrput(bw,br,op_, VLC_VN10, VLC_VB10, x); 						
	mbnenc(rcrange,rclow,rcilow, mb[CX32(cx)],RCPRM0,RCPRM1, op, x, vlcbits(VLC_VN10)); 	
	cx = ip[0];						OVERFLOWR(_in, _inlen,op,op_);			    
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcv10,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		 
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN10));   
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) { 
    unsigned x;
    mbndec(rcrange,rccode, mb[CX32(cx)],RCPRM0,RCPRM1, ip,x, vlcbits(VLC_VN10)); x = BZHI64(x, vlcbits(VLC_VN10));
	bitvrget(bw,br,ip_, VLC_VN10, VLC_VB10, x);
    *op = cx = x;
  }
  return _outlen;
}

size_t T3(rcve,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN12));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = ip[0];
	bitvrput(bw,br,op_, VLC_VN12, VLC_VB12, x); 						
	mbnenc(rcrange,rclow,rcilow, mb[CX32(cx)],RCPRM0,RCPRM1, op, x, vlcbits(VLC_VN12)); 	
	cx = ip[0];						OVERFLOWR(_in, _inlen,op,op_);			    
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcve,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               		 
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN12));   
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) { 
    unsigned x;
    mbndec(rcrange,rccode, mb[CX32(cx)],RCPRM0,RCPRM1, ip,x, vlcbits(VLC_VN12)); x = BZHI64(x, vlcbits(VLC_VN12));
	bitvrget(bw,br,ip_, VLC_VN12, VLC_VB12, x);
    *op = cx = x;
  }
  return _outlen;
}

//----  Turbo VLC 32 bits: zigzag delta
size_t T3(rcvez,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC; 
  rcencdec(rcrange,rclow,rcilow);                       
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN12));             	    
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {					
	uint32_t x = zigzagenc32(ip[0] - cx);   
	bitvrput(bw,br,op_, VLC_VN12, VLC_VB12, x); 						
	mbnenc(rcrange,rclow,rcilow, mb[CX32(cx)],RCPRM0,RCPRM1, op, x, vlcbits(VLC_VN12)); 
	cx = ip[0];									    OVERFLOWR(_in, _inlen,op,op_);
  }
  rceflush(rcrange,rclow,rcilow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcvez,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, x;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               					 
  MBU_DEC2(mb, 1<<CXN, 1<<vlcbits(VLC_VN12));             	   	 
  bitddef(bw, br); bitdinir(bw,br,ip_);  

  for(op = out; op < out+outlen; op++) { 
    mbndec(rcrange,rccode, mb[CX32(cx)],RCPRM0,RCPRM1, ip,x, vlcbits(VLC_VN12)); x = BZHI64(x, vlcbits(VLC_VN12));
	bitvrget(bw,br,ip_, VLC_VN12, VLC_VB12, x);
    *op = (cx += zigzagdec32(x)); 											
  }
  return _outlen;
}

//--------------------------------------------------------------------------------------
#ifdef _V8
#include "include_/vint.h"

size_t T3(rcv8,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  
  unsigned char *tmp = malloc(inlen*4); if(!tmp) die("malloc %u failed\n", (unsigned)(inlen*5/4));
  unsigned len  = v8enc16(in, inlen, tmp)-tmp;
  ctou32(out) = len;
  len = T3(rc,RC_PRD,enc)(tmp, len, out+4 RCPRMC)+4;
  free(tmp);
  return len;
}

size_t T3(rcv8,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint16_t      *out = (uint16_t *)_out, *op;
  OUTDEC;
  
  unsigned char *tmp = malloc(_outlen); if(!tmp) die("malloc %u failed\n", (unsigned)inlen);

  T3(rc,RC_PRD,dec)(in+4, inlen, tmp RCPRMC);
  v8dec16(tmp, outlen, out);
  free(tmp);
  return inlen;
} 

size_t T3(rcv8z,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  
  unsigned char *tmp = malloc(inlen*4); if(!tmp) die("malloc %u failed\n", (unsigned)(inlen*5/4));
  unsigned len  = v8zenc16(in, inlen, tmp, 0)-tmp;
  ctou32(out) = len;
  len = T3(rc,RC_PRD,enc)(tmp, len, out+4 RCPRMC)+4;
  free(tmp);
  return len;
}

size_t T3(rcv8z,RC_PRD,dec16)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint16_t      *out = (uint16_t *)_out, *op;
  OUTDEC;
  
  unsigned char *tmp = malloc(_outlen); if(!tmp) die("malloc %u failed\n", (unsigned)inlen);

  T3(rc,RC_PRD,dec)(in+4, inlen, tmp RCPRMC);
  v8zdec16(tmp, outlen, out, 0);
  free(tmp);
  return inlen;
} 

size_t T3(rcv8,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip;
  INDEC; 
  
  unsigned char *tmp = malloc(inlen*4); if(!tmp) die("malloc %u failed\n", (unsigned)(inlen*5/4));
  unsigned len  = v8enc32(in, inlen, tmp)-tmp;
  ctou32(out) = len;
  len = T3(rc,RC_PRD,enc)(tmp, len, out+4 RCPRMC)+4;
  
  free(tmp);
  return len;
}

size_t T3(rcv8,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op;
  OUTDEC;
  
  unsigned char *tmp = malloc(_outlen); if(!tmp) die("malloc %u failed\n", (unsigned)inlen);

  T3(rc,RC_PRD,dec)(in+4, inlen, tmp RCPRMC);
  v8dec32(tmp, outlen, out);
  free(tmp);
  return inlen;
} 

size_t T3(rcv8z,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip;
  INDEC; 
  
  unsigned char *tmp = malloc(inlen*4); if(!tmp) die("malloc %u failed\n", (unsigned)(inlen*5/4));
  unsigned len  = v8zenc32(in, inlen, tmp, 0)-tmp;
  ctou32(out) = len;
  len = T3(rc,RC_PRD,enc)(tmp, len, out+4 RCPRMC)+4;
  free(tmp);
  return len;
}

size_t T3(rcv8z,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op;
  OUTDEC;
  
  unsigned char *tmp = malloc(_outlen); 
  if(!tmp) die("malloc %u failed\n", (unsigned)inlen);

  T3(rc,RC_PRD,dec)(in+4, inlen, tmp RCPRMC);
  v8zdec32(tmp, outlen, out, 0);
  free(tmp);
  return inlen;
}
#endif
