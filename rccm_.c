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
// Turbo Range Coder: Context mixing templates include
#include "turborc.h"
#include "rcutil_.h"
#include "mb_o0.h"  // order 0 

#define mbus unsigned short

//---------- cm order 1-0 run aware base on bcm - https://github.com/encode84/bcm ---------------------
#define RUNPARM  run |
size_t T3(rcmr,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out, *ip; 
  unsigned      cx1 = 0, cx2 = 0, run = 0;
  rcencdec(rcrange,rclow);                        // range coder
  MBU_DEC1(mb0,  1<<8);							  // predictor
  MBU_DEC2(mb1,  1<<8, 1<<8);             		   
  unsigned short sse[1<<9][17]; ssebinit(sse);
  
  for(ip = in; ip < in+inlen; ip++) { 
	mbu *m1x = &mb1[cx1], *m2x = &mb1[cx2];
    unsigned y = 1<<8 | ip[0]; 
	int      i;
    for(i = 8-1; i >= 0; --i) {
	  unsigned x = y>>(i+1); 
	  mbu *m0 = &mb0[x], *m1 = &m1x[x], *m2 = &m2x[x]; mbus *msse = &sse[RUNPARM x];
	  mbur_enc(rcrange,rclow, m0, RCPRM0,RCPRM1,op, y & (1<<i), m1, m2, msse);
    }
	cx2 = cx1;
	cx1 = (unsigned char)y;
    run = cx1==cx2?0x100:0;																OVERFLOW(in,inlen,out, op, goto e);     
  } 
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rcmr,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *op; 
  unsigned      cx1 = 0, cx2 = 0, run = 0;
  rcdecdec(rcrange, rccode, ip);                      
  MBU_DEC1(mb0,  1<<8);
  MBU_DEC2(mb1,  1<<8, 1<<8);             		  
  unsigned short sse[1<<9][17]; ssebinit(sse);
  
  for(op = out; op < out+outlen; op++) { 
	mbu *m1x = &mb1[cx1], *m2x = &mb1[cx2];
    int i; 
    unsigned x = 1;
    for(i = 8-1; i >= 0; --i) {
	  mbu *m0 = &mb0[x], *m1 = &m1x[x], *m2 = &m2x[x]; mbus *msse = &sse[RUNPARM x];
      mbur_dec(rcrange,rccode, m0, RCPRM0,RCPRM1,ip, x, m1, m2, msse);
    }
	cx2   = cx1;
    op[0] = cx1 = (unsigned char)x;
	run = cx1==cx2?0x100:0;
  }
}

//-------------- Order 1-0 context mixing -------------------------------------
size_t T3(rcm,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out, *ip; 
  unsigned      cx = 0, run = 0;
  rcencdec(rcrange,rclow);                         
  MBU_DEC1(mb0,  1<<8);
  MBU_DEC2(mb1,  1<<8,  1<<8);             		  
  unsigned short sse[1<<8][17]; sseinit(sse);
  
  for(ip = in; ip < in+inlen; ip++) { 
	mbu *m1x = mb1[cx];
	int i;
    unsigned x = 1<<8 | ip[0]; 
    for(i = 8-1; i >= 0; --i) {
	  unsigned y = x>>(i+1); 
	  mbu *m0 = &mb0[y], *m1 = &m1x[y]; mbus *msse2 = &sse[y];
	  mbum_enc(rcrange,rclow, m0, RCPRM0,RCPRM1,op, x & (1<<i), m1, 0, msse2);
    }
	cx = ip[0];																	OVERFLOW(in,inlen,out, op, goto e);  
	    
  } 
  rceflush(rcrange,rclow, op);
  e:return op - out;
}

size_t T3(rcm,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *op; 
  unsigned      cx = 0, run = 0;
  rcdecdec(rcrange, rccode, ip);
  MBU_DEC1(mb0,  1<<8);
  MBU_DEC2(mb1,  1<<8, 1<<8);             		            		
  unsigned short sse[1<<8][17]; sseinit(sse);

  for(op = out; op < out+outlen; op++) { 
	mbu *m1x = mb1[(uint8_t)cx];
    int i; 
    unsigned x = 1;
    for(i = 8-1; i >= 0; --i) {
	  mbu *m0 = &mb0[x], *m1 = &m1x[x]; mbus *msse = &sse[x];
      mbum_dec(rcrange,rccode, m0, RCPRM0,RCPRM1,ip, x, m1, 0, msse);
    }
    op[0] = cx = (unsigned char)x;
  }
}

//-------------- Order 2-1-0 context mixing -------------------------------------
size_t T3(rcm2,RC_PRD,enc)(unsigned char *in, size_t inlen, unsigned char *out RCPRM) { 
  unsigned char *op = out, *ip; 
  unsigned      cx = 0, run = 0;
  rcencdec(rcrange,rclow);                         
  MBU_DEC1( mb0,  1<<8);
  MBU_DEC2( mb1,  1<<8,  1<<8);             		  
  MBU_NEWI2(mb2,  1<<16, 1<<8);  //MBU_DEC2(mb2,  1<<16, 1<<8);             		
  unsigned short sse[1<<8][17]; sseinit(sse);

  for(ip = in; ip < in+inlen; ip++) { 
	mbu *m1x = mb1[(uint8_t)cx], *m2x = &mb2[(uint16_t)cx*(1<<8)];
    unsigned x = 1<<8 | ip[0]; 
	int i;
    for(i = 8-1; i >= 0; --i) {
	  unsigned y = x>>(i+1); 
	  mbu *m0 = &mb0[y], *m1 = &m1x[y], *m2 = &m2x[y]; mbus *msse = &sse[y];
	  mbum2_enc(rcrange,rclow, m0, RCPRM0,RCPRM1,op, x & (1<<i), m1, m2, msse);
    }
	cx  = cx << 8 | ip[0];														OVERFLOW(in,inlen,out, op, goto e);     
  } 
  rceflush(rcrange,rclow, op);
  e: free(mb2);
  return op - out;
}

size_t T3(rcm2,RC_PRD,dec)(unsigned char *in, size_t outlen, unsigned char *out RCPRM) { 
  unsigned char *ip = in, *op; 
  unsigned      cx = 0, run = 0;
  rcdecdec(rcrange, rccode, ip);                      
  MBU_DEC1(mb0,  1<<8);
  MBU_DEC2(mb1,  1<<8, 1<<8);             		  
  MBU_NEWI2(mb2, 1<<16, 1<<8);  //MBU_DEC2(mb2,  1<<16, 1<<8);             		
  unsigned short sse[1<<9][17]; sseinit(sse);

  for(op = out; op < out+outlen; op++) { 
	mbu *m1x = mb1[(uint8_t)cx], *m2x = &mb2[(uint16_t)cx*(1<<8)];
    int i; 
    unsigned x = 1;
    for(i = 8-1; i >= 0; --i) {
	  mbu *m0 = &mb0[x], *m1 = &m1x[x], *m2 = &m2x[x]; mbus *msse = &sse[x];
      mbum2_dec(rcrange,rccode, m0, RCPRM0,RCPRM1,ip, x, m1, m2, msse);
    }
    op[0] = cx = cx << 8 | (unsigned char)x;
  }
  free(mb2);
}
