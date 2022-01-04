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

// Turbo Range Coder: templates include
// Symbols:   8,16,32 bits integers 16/32 floating point, 
// Coding:    structured/segmented, gamma, rice, Turbo vlc (variable length coding for large integers)
// Order:     0,1,2,sliding context
// predictor: context,zigzag delta, context mixing

#include <string.h>

#include "turborc.h"
#include "mb_o0.h"  // order 0 
#include "rcutil_.h"

//---- order 0 byte (8 bits) adaptive - bitwise encode/decode -------------------------------
size_t T3(rc,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow);                  // range coder
  MBU_DEC1(mb,1<<8);             			// predictor
  
  for(ip = in; ip < in+inlen; ip++) {
    mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1,op, ip[0]);      OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rc,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                
  MBU_DEC1(mb,1<<8);             			
  
  for(op = out; op < out+outlen; op++) 
    mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1,ip, *op);
} 

//--- order0 : 16 bits -----------------------
size_t T3(rc,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx;
  INDEC;
  rcencdec(rcrange,rclow);                       							 
  MBU_DEC1(mb1, 1<<8); 
  MBU_DEC2(mb0, 1<<8, 1<<8);   												 
  
  for(ip = in; ip < in+inlen; ip++) {	 
	uint16_t r = ip[0];
	mb8enc(rcrange,rclow, mb1,     RCPRM0,RCPRM1, op, cx=r>>8);      
	mb8enc(rcrange,rclow, mb0[cx], RCPRM0,RCPRM1, op, (uint8_t)r); 			
  }
  rceflush(rcrange,rclow, op);
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
}

//---Order0 : 32 bits -----------------------
#define XN1 10
size_t T3(rc,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC; 
  rcencdec(rcrange,rclow);                       
  MBU_DEC1(mb3,  1<<8);  
  MBU_DEC2(mb2,  1<< 8,  1<<8);  
  MBU_DEC2(mb1,  1<<XN1, 1<<8);  
  MBU_DEC2(mb0,  1<<XN1, 1<<8);    		  
  
  for(ip = in; ip < in+inlen; ip++) { 
    uint32_t r = ip[0],x; 			    
	                               mb8enc(rcrange,rclow, mb3,RCPRM0,RCPRM1, op,        cx =   r>>24 ); 
	mbu *mb = mb2[cx];             mb8enc(rcrange,rclow, mb, RCPRM0,RCPRM1, op, x = (uint8_t)(r>>16)); cx = cx << 8 | x;
	     mb = mb1[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow, mb, RCPRM0,RCPRM1, op, x = (uint8_t)(r>> 8)); cx = cx << 8 | x;
	     mb = mb0[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow, mb, RCPRM0,RCPRM1, op,     (uint8_t)(r    ));
    																						 OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);	
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
}

//---- Order 0 Nibble (4 bits) adaptive : encode/decode low nibble ---------------------------
size_t T3(rc4,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow);                       
  MBU_DEC1(mb,1<<4);             			     
  
  for(ip = in; ip < in+inlen; ip++) {
    mb4enc(rcrange,rclow, mb,RCPRM0,RCPRM1,op, ip[0]&0xf); OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rc4,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                    
  MBU_DEC1(mb,1<<4);             			     
  
  for(op = out; op < out+outlen; op++)
    mb4dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip, *op); 
} 

// Order 0 Nibble (4 bits) --------------------------------------------------------------------
size_t T3(rc4c,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow);                       
  MBU_DEC1(mb,1<<4);             			     
  
  for(ip = in; ip < in+inlen; ip++) {
    mb4senc(rcrange,rclow, mb,RCPRM0,RCPRM1, op, ip[0]&0xf); OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rc4c,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                
  MBU_DEC1(mb,1<<4);             			
  
  for(op = out; op < out+outlen; op++)
    mb4sdec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip, *op); 
} 

//-- Order1 : 8 bits -------------------------------------------------------------------------
size_t T3(rcc,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow);                    
  MBU_DEC2(mb,1<<8, 1<<8);             		  
  
  for(ip = in; ip < in+inlen; ip++) {
    mb8enc(rcrange,rclow, mb[cx], RCPRM0,RCPRM1, op, cx = ip[0]); 	 OVERFLOW(in,inlen,out, op, goto e);    
  } 
  rceflush(rcrange,rclow, op); 
  e:return op - out;
}

size_t T3(rcc,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *op; 
  unsigned      cx = 0;
  rcdecdec(rcrange,rccode, ip);                   
  MBU_DEC2(mb, 1<<8, 1<<8);             	   

  for(op = out; op < out+outlen; op++)
    mb8dec(rcrange,rccode, mb[cx],RCPRM0,RCPRM1, ip, cx = *op);
}

//---Order1 : 16 bits -----------------------
size_t T3(rcc,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0, ema = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       							 
  MBU_DEC2( mb1, 1<< 8, 1<<8); 
  MBU_NEWI2(mb0, 1<<16, 1<<8);  												 
  
  for(ip = in; ip < in+inlen; ip++) {	 
	uint16_t r = ip[0];
	mb8enc(rcrange,rclow, mb1[cx], RCPRM0,RCPRM1, op, r>>8);       cx = cx <<8 | r>>8;	
	mb8enc(rcrange,rclow, &mb0[cx*(1<<8)], RCPRM0,RCPRM1, op, (uint8_t)r); 			
	cx = (uint8_t)cx;
  }
  rceflush(rcrange,rclow, op);
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
  rcencdec(rcrange,rclow);                       
  MBU_DEC2(mb3,  1<<XN, 1<<8);  
  MBU_DEC2(mb2,  1<< 8, 1<<8);  
  MBU_DEC2(mb1, 1<<XN1, 1<<8);  
  MBU_DEC2(mb0, 1<<XN1, 1<<8);     			  
  
  for(ip = in; ip < in+inlen; ip++) { 
	uint32_t r = ip[0],x; 			    
	mbu *mb = mb3[cx = CX32(cx)];  mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op,        cx =   r>>24 ); 
	     mb = mb2[cx];             mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>>16)); cx = cx << 8 | x;
	     mb = mb1[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>> 8)); cx = cx << 8 | x;
	     mb = mb0[BZHI32(cx,XN1)]; mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op,     (uint8_t)(r    ));
    cx = r;																						 OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);														
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
}

//---Order 11bs : 32 bits integer, 32 bits floating point -----------------------
#define XNS       12
#define XN        (XNS-1)
#define CX32(_x_) (((_x_)>>(32-XNS)) & ((1u<<(XN))-1))
size_t T3(rcc2,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                     
  MBU_DEC2(mb3,  1<<XN, 1<<8);  
  MBU_DEC2(mb2,  1<< 8, 1<<8);   
  MBU_NEWI2(mb1, 1<<XN1, 1<<8); 
  MBU_NEWI2(mb0, 1<<XN1, 1<<8);

  for(ip = in; ip < in+inlen; ip++) { 
    uint32_t r = ip[0],x; 			    	
	mbu *mb = mb3[cx = CX32(cx)];            mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op,        cx =   r>>24 ); 
	     mb = mb2[cx];                       mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>>16)); cx = cx << 8 | x;
	     mb = &mb1[BZHI32(cx,XN1)*(1<<8)]; mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op, x = (uint8_t)(r>> 8)); cx = cx << 8 | x;
	     mb = &mb0[BZHI32(cx,XN1)*(1<<8)]; mb8enc(rcrange,rclow, mb,RCPRM0,RCPRM1, op,     (uint8_t)(r    ));
    cx = r;																						 OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);														
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
	mbu *mb = mb3[cx = CX32(cx)];           mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip, r);
	     mb = mb2[r];                       mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx;
         mb = &mb1[BZHI32(r,XN1)*(1<<8)]; mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); r = r << 8 | cx; 
		 mb = &mb0[BZHI32(r,XN1)*(1<<8)]; mb8dec(rcrange,rccode, mb,RCPRM0,RCPRM1, ip,cx); 
    *op = cx = r << 8 | cx; 													
  }
  free(mb1);free(mb0);
}

//-- Order 2 --------------------------------------------------------------------------
size_t T3(rcc2,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow);  
  MBU_NEWI2(mb, 1<<16, 1<<8);         		  
  
  for(ip = in; ip < in+inlen; ip++) {
    mb8enc(rcrange,rclow, &mb[cx*(1<<8)],RCPRM0,RCPRM1, op, ip[0]); 
	cx = (uint16_t)(cx<<8 | ip[0]);						    OVERFLOW(in,inlen,out, op, goto e);    
  } 
  rceflush(rcrange,rclow, op); 
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
}

//-- order N bits with sliding context ---------------------------------------------------------
#include "mb_on.h"

#define MBC_C 8     // order 8 bits context (4 <= MBC_C <= 20)
size_t T3(rcx,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow);                 
  MBC_DEC(mbc, MBC_C);                     	// predictor with MBC_C context bits 
  
  for(ip = in; ip < in+inlen; ip++) { 
    mbcenc(rcrange,rclow, cx, MBC_C, mbc,RCPRM0,RCPRM1,op, *ip); OVERFLOW(in,inlen,out, op, goto e);
  } 
  rceflush(rcrange,rclow, op); 
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
}

#define MBC_C2 15
  #if RC_PRDID == 1
int mbc_c = 15;
mbcset(unsigned m) { mbc_c = m; if(mbc_c > MBC_C2) mbc_c = MBC_C2; if(mbc_c < 4) mbc_c = 4; } // set context bits length 1-16
  #else
extern int mbc_c;
  #endif

size_t T3(rcx2,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  unsigned      cx = 0;
  rcencdec(rcrange,rclow);                       
  MBC_DEC(mbc, MBC_C2);                         
  
  for(ip = in; ip < in+inlen; ip++) { 
    mbcenc(rcrange,rclow, cx, mbc_c, mbc,RCPRM0,RCPRM1,op, *ip); OVERFLOW(in,inlen,out, op, goto e);
  } 
  rceflush(rcrange,rclow, op); 
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
}

//************************* Variable Length Coding **************************************************
#include "mb_vint.h"

#define IN0 3
#define IN1 5
#define IN2 8
size_t T3(rcu3,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  unsigned char *op = out, *ip; 
  rcencdec(rcrange,rclow);                       
  MBU3_DEC(mbf, mb0,IN0, mb1,IN1, mb2,IN2);
  
  for(ip = in; ip < in+inlen; ip++) { 
	mbu3enc(rcrange,rclow, mbf,mb0,IN0,mb1,IN1,mb2,IN2, RCPRM0,RCPRM1, op, ip[0]);  OVERFLOW(in,inlen,out, op, goto e); 
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rcu3,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                     
  MBU3_DEC(mbf, mb0,IN0, mb1,IN1, mb2,IN2);
  
  for(op = out; op < out+outlen; op++)
	mbu3dec(rcrange,rccode, mbf,mb0,IN0,mb1,IN1,mb2,IN2,RCPRM0,RCPRM1, ip, op[0]);
} 

//------------------------------ Gamma coding for 8, 16 and 32 bits integers --------------------------------------
// usage: Run length in RLE and BWT(MTF, QLFC), match/literal length in lz77, integer arrays with small values, large alphabet
size_t T3(rcg,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t *op = out, *ip;
  rcencdec(rcrange,rclow);                      
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(ip = in; ip < in+inlen; ip++) {
    mbgenc(rcrange,rclow, &mbg0c,mbguc,mbgbc, RCPRM0,RCPRM1,op, ip[0]); OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rcg,RC_PRD,dec8)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  rcdecdec(rcrange,rccode, ip);                    
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(op = out; op < out+outlen; op++)
    mbgdec(rcrange,rccode, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,ip, op[0]);
} 

size_t T3(rcg,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 32, 32); 
   
  for(ip = in; ip < in+inlen; ip++)  {      
     mbgenc(rcrange,rclow, &mbg0, mbgu,mbgb,RCPRM0,RCPRM1,op, ip[0]); OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op); 
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
}

#define GQMAX32 12
size_t T3(rcg,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip;
  INDEC;
  rcencdec(rcrange,rclow);                       	
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);    
  
  for(ip = in; ip < in+inlen; ip++) {
    mbgenc32(rcrange,rclow, mbgu,mbgb,RCPRM0,RCPRM1,op, ip[0], GQMAX32); OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op); 
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
}

//----- Gamma Coding + Zigzag Delta ---------------------------------
size_t T3(rcgz,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t  *op = out, *ip, cx = 0; 
  rcencdec(rcrange,rclow);                        
  MBG_DEC(mbg0c, mbguc, mbgbc, 32, 32); 
  
  for(ip = in; ip < in+inlen; ip++) {         
    mbgenc(rcrange,rclow, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,op, zigzagenc8(ip[0] - cx));
	cx = ip[0];								                                             OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);
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
} 

size_t T3(rcgz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBG_DEC( mbg0c, mbguc, mbgbc, 32, 32); 				 
  
  for(ip = in; ip < in+inlen; ip++) {        
    mbgenc(rcrange,rclow, &mbg0c,mbguc,mbgbc,RCPRM0,RCPRM1,op, zigzagenc16(ip[0] - cx));  OVERFLOW(_in,_inlen,out, op, goto e);
	cx = ip[0];												    
  }
  rceflush(rcrange,rclow, op); 
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
}

size_t T3(rcgz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                        
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 				// predictor rice
  
  for(ip = in; ip < in+inlen; ip++) {         
    mbgenc32(rcrange,rclow, mbguc,mbgbc,RCPRM0,RCPRM1,op, zigzagenc32(ip[0] - cx), GQMAX32); OVERFLOW(_in,_inlen,out, op, goto e);
	cx = ip[0];    
  }
  rceflush(rcrange,rclow, op);
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
}

//************ Variable Length Coding : Adaptive Length limited Rice coding for 8, 16 and 32 bits integers ************************
#define RICEMAX 12

size_t T3(rcr,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t *op = out, *ip; 
  unsigned ema = 0;
  rcencdec(rcrange,rclow);                       
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(ip = in; ip < in+inlen; ip++) {         
    unsigned x = ip[0], log2m = RICEK(ema); 
    mbrenc32(rcrange,rclow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema = EMA64(ema, 63, x);					    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rcr,RC_PRD,dec8)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned char *ip = in, *op; 
  unsigned ema = 0;
  rcdecdec(rcrange,rccode, ip);                     
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema);
    mbrdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, RICEMAX,log2m); ema = EMA64(ema, 63, x);
    *op = x; 
  }
} 

size_t T3(rcr,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip;
  INDEC;
  unsigned      ema = 0;  
  rcencdec(rcrange,rclow);                         
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33); 		 
  
  for(ip = in; ip < in+inlen; ip++) {         
    unsigned x = ip[0], log2m = RICEK(ema);
    mbrenc32(rcrange,rclow, mbgu,mbgb,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema = EMA64(ema, 63, x);						    
	OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);
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
	ema = EMA64(ema, 63, x);
  }
  return _outlen;
}

size_t T3(rcr,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip;
  INDEC;
  unsigned      ema = 0, cx = 0;
  rcencdec(rcrange,rclow);                       		  
  MBG_DEC(mbg0c, mbguc, mbgbc, 46, 46); 				 

  for(ip = in; ip < in+inlen; ip++) {         
    uint32_t x = ip[0], log2m = RICEK(ema);   
    mbrenc32(rcrange,rclow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema = EMA64((uint64_t)ema, 63, (uint64_t)x);				    OVERFLOW(_in,_inlen,out, op, goto e);
	cx = ip[0];
  }
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rcr,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) { 
  unsigned char *ip = in;
  uint32_t      *out = (uint32_t *)_out, *op;
  OUTDEC;
  unsigned      ema = 0;
  rcdecdec(rcrange,rccode, ip);              
  MBG_DEC(mbg0c, mbguc, mbgbc, 46, 46);    

  for(op = out; op < out+outlen; op++) { 
    unsigned x, log2m = RICEK(ema); 
    mbrdec32(rcrange,rccode, mbguc,mbgbc,RCPRM0,RCPRM1,ip, x, RICEMAX,log2m); ema = EMA64((uint64_t)ema, 63, (uint64_t)x);
    *op = x;
  }
}

//-- Rice + delta zigzag ------------------------------------
size_t T3(rcrz,RC_PRD, enc8)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) {
  uint8_t  *op = out, *ip, cx = 0; 
  unsigned  ema = 0;
  rcencdec(rcrange,rclow);                       
  MBG_DEC(mbg0c, mbguc, mbgbc, 33, 33); 
  
  for(ip = in; ip < in+inlen; ip++) {         
    uint8_t  d = ip[0] - cx;
	unsigned x = zigzagenc8(d), log2m = RICEK(ema); 
    mbrenc32(rcrange,rclow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema  = EMA16(ema, 15, x); 
	cx = ip[0];									    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);
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
	ema = EMA16(ema, 15, x); 
  }
} 

size_t T3(rcrz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  unsigned      ema  = 0;                                
  rcencdec(rcrange,rclow);                       
  MBG_DEC( mbg0c, mbguc, mbgbc, 33, 33); 				 
 
  for(ip = in; ip < in+inlen; ip++) {        
    uint16_t d = ip[0] - cx;
	unsigned x = zigzagenc16(d), log2m = RICEK(ema); 
    mbrenc32(rcrange,rclow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema  = EMA64(ema, 63, x); 	
	cx = ip[0];					
	OVERFLOW(_in,_inlen,out, op, goto e);
  } 
  rceflush(rcrange,rclow, op);
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
	ema = EMA64(ema, 63, x);
  }
}

size_t T3(rcrz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  unsigned      ema = 0;
  rcencdec(rcrange,rclow);                      
  MBG_DEC(mbg0c, mbguc, mbgbc, 64, 64); 		 
  
  for(ip = in; ip < in+inlen; ip++) {         
    uint32_t d = ip[0] - cx;														
	unsigned x = zigzagenc32(d), log2m = RICEK(ema);  
    mbrenc32(rcrange,rclow, mbguc,mbgbc,RCPRM0,RCPRM1,op, x, RICEMAX,log2m); 
	ema  = EMA64((uint64_t)ema, 63, (uint64_t)x); 
	cx = ip[0];						    OVERFLOW(_in,_inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op);
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
	ema = EMA64((uint64_t)ema, 63, (uint64_t)x);
  }
}

//-------------------- RLE : Run Length Encoding in gamma+rc ----------------------------------
// characters encoded using the current character as order-8bits sliding context
// Run length are encoded in gamma coding, 
size_t T3(rcrle,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *in_ = in+inlen, *op = out; 
  rcencdec(rcrange,rclow);                     
  MBU_DEC1(mb,1<<8); 
  MBG_DEC(mbg0, mbgu, mbgb, 32, 32);                    // run length predictor

  while(ip < in_) { 
    uint8_t *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; // run length

    mb8enc(rcrange,rclow,               mb,RCPRM0,RCPRM1,op, u);
    mbgenc(rcrange,rclow, &mbg0,mbgu,mbgb,RCPRM0,RCPRM1,op, r-1);
    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op); 
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
  rcencdec(rcrange,rclow);                       							 
  MBU_DEC1(mb1, 1<< 8); 
  MBU_DEC2(mb0, 1<< 8, 1<<8);   												 
  MBG_DEC1(mbg0,1<<16, mbgu, 1<<8, mbgb, 33, 33);

  uint16_t *in_ = in+inlen;																		
  for(ip = in; ip < in_;) {
	uint16_t *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; // run length			
	mb8enc(rcrange,rclow, mb1,     RCPRM0,RCPRM1, op, u>>8);       cx = u>>8;	
	mb8enc(rcrange,rclow, mb0[cx], RCPRM0,RCPRM1, op, (uint8_t)u); 			
    mbgenc(rcrange,rclow, &mbg0[u],mbgu[u>>8],mbgb,RCPRM0,RCPRM1,op, r-1);  		
  }
  rceflush(rcrange,rclow, op); 														
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
}

// Character  : encoded as order-8bits sliding context
// Run length : gamma coding with current character as context 
size_t T3(rcrle1,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) { 
  unsigned      cx = 0;
  unsigned char *ip = in, *in_ = in+inlen, *op = out; 
  rcencdec(rcrange,rclow);                         
  MBG_DEC2(mbg0, 1<<8, mbgu, 1<<8, mbgb, 1<<8, 32, 32);   
  MBU_DEC2(mb, 1<<8, 1<<8);
    
  while(ip < in_) { 
    unsigned char *p = ip, u = *ip++; 
	while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; r--; // r:run length

	mbu *m = mb[cx]; mb8enc(rcrange,rclow, m, RCPRM0,RCPRM1,op, u); cx = u;
    mbgenc(rcrange,rclow, &mbg0[u], mbgu[u], mbgb[u], RCPRM0,RCPRM1,op, r);
    OVERFLOW(in,inlen,out, op, goto e);
  }
  rceflush(rcrange,rclow, op); 
  e:return op-out;
}

size_t T3(rcrle1,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) {
  unsigned      cx = 0;
  unsigned char *ip = in, *op = out; 
  rcdecdec(rcrange,rccode, ip);                                
  MBG_DEC2(mbg0,  1<<8, mbgu, 1<<8, mbgb, 1<<8, 33, 33);   
  MBU_DEC2(mb, 1<<8, 1<<8);

  while(op < out+outlen) { 
    unsigned r;
    unsigned char ch;
    mbu *m = mb[cx]; mb8dec(rcrange,rccode,  m,                            RCPRM0,RCPRM1,ip, ch); cx = (uint8_t)ch;
    ch = cx;         mbgdec(rcrange,rccode, &mbg0[ch], mbgu[ch], mbgb[ch], RCPRM0,RCPRM1,ip, r);
    memset_(op, ch, r+1);
  }
  return ip-in;   
}

size_t T3(rcrle1,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op   = out;
  uint16_t      *in   = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       							 
  MBU_DEC2(mb1, 1<< 8, 1<<8); 
  MBU_DEC2(mb0, 1<< 8, 1<<8);   												 
  MBG_DEC1(mbg0,1<<16, mbgu, 1<<8, mbgb, 33, 33);
  uint16_t *in_ = in+inlen;	

  for(ip = in; ip < in_;) {
	uint16_t *p = ip, u = *ip++;  
    while(ip < in_ && *ip == u) ip++;
    unsigned r = ip - p > (unsigned)-1?(unsigned)-1:ip - p; 
	mb8enc(rcrange,rclow, mb1[cx], RCPRM0,RCPRM1, op, u>>8);       cx = u>>8;	
	mb8enc(rcrange,rclow, mb0[cx], RCPRM0,RCPRM1, op, (uint8_t)u); 			
    mbgenc(rcrange,rclow, &mbg0[u],mbgu[u>>8],mbgb,RCPRM0,RCPRM1,op, r-1);
  }
  rceflush(rcrange,rclow, op); 
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
}

//-------- Turbo VLC: Variable Length Code for large integers: 16+32 bits similar to µ-Law/Extra Bit Codes encoding ------------------------
// Usage: Lz77 Offsets, Large alphabet/ Integers
#define OV 4
#define OVERFLOWR(_in_, _inlen_, _p_,_p__)	if(_p_+12 > _p__) { memcpy(out, _in_, _inlen_); op = out_; goto e; }

#define VN8 3
#define CXN 8
#define CX16(_x_) ((_x_)>>8)
// 16 bits  -----------
size_t T3(rcv,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VN8));
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint16_t x = ip[0];
	bitvrput(bw,br,op_, VN8, VLC_VB8, x); 
	mb8enc(rcrange,rclow, mb1[CX16(cx)],RCPRM0,RCPRM1, op, x); 
	cx = ip[0];									    							OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow, op);
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
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);  

  for(op = out; op < out+outlen; op++) { 
    unsigned x;
    mb8dec(rcrange,rccode, mb1[CX16(cx)],RCPRM0,RCPRM1, ip,x);
	bitvrget(bw,br,ip_, VN8, VLC_VB8, x);
    *op = cx = x;
  }
}

//---- Turbo vlc: 16 bits zigzag delta --------------
size_t T3(rcvz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VN8));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint16_t x = zigzagenc16(ip[0] - cx);  
	bitvrput(bw,br,op_, VN8, VLC_VB8, x); 
	mb8enc(rcrange,rclow, mb1[CX16(cx)],RCPRM0,RCPRM1, op, x); 
	cx = ip[0];									  OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow, op);
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
  MBU_DEC2(mb1, 1<<CXN, 1<<vlcbits(VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);  

  for(op = out; op < out+outlen; op++) { 
    unsigned x;               			
    mb8dec(rcrange,rccode, mb1[CX16(cx)],RCPRM0,RCPRM1, ip,x);
	bitvrget(bw,br,ip_, VN8, VLC_VB8, x);
    *op = (cx += zigzagdec16(x));
  }
}

// Turbo vlc 32 bits -----------
#define CX32(_x_) ((_x_)>>24)

size_t T3(rcv,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VN8));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = ip[0];
	bitvrput(bw,br,op_, VN8, VLC_VB8, x); 
	mb8enc(rcrange,rclow, mb3[CX32(cx)],RCPRM0,RCPRM1, op, x); 
	cx = ip[0];									    OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow, op);
  bitflushr(bw,br,op_); unsigned l = out_-op_; memmove(op, op_, l); op += l; ctou32(out) = op-out;   
  OVERFLOW(_in,_inlen,out, op, goto e);
  e:return op - out;
}

size_t T3(rcv,RC_PRD,dec32)(unsigned char *in, size_t _outlen, unsigned char *_out RCPRM) {
  unsigned inlen = ctou32(in);
  unsigned char *ip = in+4,*ip_ = in+inlen;
  uint32_t      *out = (uint32_t *)_out, *op, cx = 0, x;
  OUTDEC;
  rcdecdec(rcrange,rccode, ip);               				 
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) { 
    mb8dec(rcrange,rccode, mb3[CX32(cx)],RCPRM0,RCPRM1, ip,x);
	bitvrget(bw,br,ip_, VN8, VLC_VB8, x);
    *op = cx = x;
  }
}

// Turbo vlc 32 bits + zigzag delta -----------
size_t T3(rcvz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;  
  rcencdec(rcrange,rclow);                       
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VN8));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = zigzagenc32(ip[0] - cx); 					
	bitvrput(bw,br,op_, VN8, VLC_VB8, x); 
	mb8enc(rcrange,rclow, mb3[CX32(cx)],RCPRM0,RCPRM1, op, x);   
	cx = ip[0];									    OVERFLOWR(_in, _inlen, op,op_);
  }
  rceflush(rcrange,rclow, op);
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
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VN8));   
  bitddef(bw, br); bitdinir(bw,br,ip_);  	

  for(op = out; op < out+outlen; op++) { 
    mb8dec(rcrange,rccode, mb3[CX32(cx)],RCPRM0,RCPRM1, ip,x);
	bitvrget(bw,br,ip_, VN8, VLC_VB8, x);
    *op = (cx += zigzagdec32(x));														
  }
}

//---- Turbo VLC with exponent coded in gamma and mantissa in bitio -----------------------------------
size_t T3(rcvg,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;  
  rcencdec(rcrange,rclow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33); 			 
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint16_t x = ip[0];
	mbvenc(rcrange,rclow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VN8, VLC_VB8); bitenormr(bw,br,op_);
	cx = ip[0];																		  OVERFLOWR(_in, _inlen,op,op_);										    
  }
  rceflush(rcrange,rclow, op);
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
	bitdnormr(bw,br,ip_); mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VN8, VLC_VB8);
    *op = cx = x;
  }
}

size_t T3(rcvgz,RC_PRD,enc16)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint16_t      *in = (uint16_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                      
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
  bitedef(bw,br); biteinir(bw,br,op_); 
  
  for(ip = in; ip < in+inlen; ip++) {
	uint16_t x = zigzagenc16(ip[0] - cx);
	mbvenc(rcrange,rclow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VN8, VLC_VB8); bitenormr(bw,br,op_);	
	cx = ip[0];																		  OVERFLOWR(_in, _inlen,op,op_);			    
  }
  rceflush(rcrange,rclow, op);
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
	bitdnormr(bw,br,ip_); mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VN8, VLC_VB8);
    *op = (cx += zigzagdec16(x));												
  }
}

size_t T3(rcvg,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33); 
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = ip[0];
	mbvenc(rcrange,rclow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VN8, VLC_VB8); bitenormr(bw,br,op_);
	cx = ip[0];					OVERFLOWR(_in, _inlen,op,op_);				    
  }
  rceflush(rcrange,rclow, op);
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
	bitdnormr(bw,br,ip_); mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VN8, VLC_VB8);
    *op = cx = x;
  }
}

size_t T3(rcvgz,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBG_DEC(mbg0, mbgu, mbgb, 33, 33);
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = zigzagenc32(ip[0] - cx);
	mbvenc(rcrange,rclow, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, op, x, bw, br, VN8, VLC_VB8); bitenormr(bw,br,op_);	
	cx = ip[0];																	      OVERFLOWR(_in, _inlen,op,op_);			    
  }
  rceflush(rcrange,rclow, op);
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
	bitdnormr(bw,br,ip_); mbvdec(rcrange,rccode, &mbg0, mbgu, mbgb, RCPRM0,RCPRM1, ip, x, bw, br, VN8, VLC_VB8);
    *op = (cx += zigzagdec32(x));
  }
}

//---- Turbo VLC - 32 bits: Large exponent -----------------------
#define VNE 7 // 12 bits (7 = 12-5) Exponent
#define VBE VLC_VB12

size_t T3(rcve,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC;
  rcencdec(rcrange,rclow);                       
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VNE));   
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {
	uint32_t x = ip[0];
	bitvrput(bw,br,op_, VNE, VBE, x); 						
	mbnenc(rcrange,rclow, mb3[CX32(cx)],RCPRM0,RCPRM1, op, x, vlcbits(VNE)); 	
	cx = ip[0];						OVERFLOWR(_in, _inlen,op,op_);			    
  }
  rceflush(rcrange,rclow, op);
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
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VNE));   
  bitddef(bw, br); bitdinir(bw,br,ip_);

  for(op = out; op < out+outlen; op++) { 
    unsigned x;
    mbndec(rcrange,rccode, mb3[CX32(cx)],RCPRM0,RCPRM1, ip,x, vlcbits(VNE)); x = BZHI64(x, vlcbits(VNE));
	bitvrget(bw,br,ip_, VNE, VBE, x);
    *op = cx = x;
  }
}

//----  Turbo VLC 32 bits: zigzag delta
size_t T3(rcvez,RC_PRD,enc32)(unsigned char *_in, size_t _inlen, unsigned char *out RCPRM) {
  unsigned char *op = out+4, *out_ = out+_inlen, *op_=out_;
  uint32_t      *in = (uint32_t *)_in, *ip, cx = 0;
  INDEC; 
  rcencdec(rcrange,rclow);                       
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VNE));             	    
  bitedef(bw,br); biteinir(bw,br,op_);
  
  for(ip = in; ip < in+inlen; ip++) {					
	uint32_t x = zigzagenc32(ip[0] - cx);   
	bitvrput(bw,br,op_, VNE, VBE, x); 						
	mbnenc(rcrange,rclow, mb3[CX32(cx)],RCPRM0,RCPRM1, op, x, vlcbits(VNE)); 
	cx = ip[0];									    OVERFLOWR(_in, _inlen,op,op_);
  }
  rceflush(rcrange,rclow, op);
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
  MBU_DEC2(mb3, 1<<CXN, 1<<vlcbits(VNE));             	   	 
  bitddef(bw, br); bitdinir(bw,br,ip_);  

  for(op = out; op < out+outlen; op++) { 
    mbndec(rcrange,rccode, mb3[CX32(cx)],RCPRM0,RCPRM1, ip,x, vlcbits(VNE)); x = BZHI64(x, vlcbits(VNE));
	bitvrget(bw,br,ip_, VNE, VBE, x);
    *op = (cx += zigzagdec32(x)); 											
  }
}
