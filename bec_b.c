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
// Bit entropy coder with bitio
#include "conf.h"
#include "rcutil_.h"
#define IN_BECTAB
#include "bec_.h"
#include "bec.h"
#include "bec_bstm.h"     

ectab_t bectab[ECN][ECN][ECN][ECN]
#if ECN == 8
=
#include "bectab8_.h"
#elif ECN == 12
=
#include "bectab12_.h"
#elif ECN == 16
=
#include "bectab16_.h"
#else
;
#endif 
static unsigned becini_;
void bectabini() { unsigned lx,hx,li,hi,lhi; if(becini_) return; becini_++; 
    #if ECN != 8 && ECN != 12 && ECN != 16
  for(      lx = 0; lx < ECN; lx++) 
    for(    hx = 0; hx < ECN; hx++) 
      for(  li = 0; li <= lx; li++) 
        for(hi = 0; hi <= hx; hi++) 
          if((lhi = li + hi) < ECN) {
            unsigned l = li, r = hi, ml = lx, mr = hx, t = (l + r), cl, msb, cw;
            if(t > ml) { unsigned ih = t - ml; mr -= ih; t -= ih; }
            if(t > mr) { unsigned il = t - mr; l  -= il; t -= il; } 
			cl  = __bsr32(t|1); 
			msb = (l | (1u << cl)) <= t; 
			cw  = l << msb | l >> cl; 
			cl += msb; cw &= (1u << cl) - 1;
            ectab_t *e = &bectab[lx][hx][li][lhi]; e->cw = cw; e->cl = cl;
          }
	  #if 0  // generate LUT table 8,12,16: "turborc file 2>becttabN_.h"
  fprintf(stderr,"{");
  for(  unsigned i = 0; i < ECN; ++i) { fprintf(stderr,"\n{");
    for(    unsigned j = 0; j < ECN; ++j) { fprintf(stderr,"\n{");
      for(  unsigned k = 0; k < ECN; ++k) { fprintf(stderr,"\n{");
        for(unsigned l = 0; l < ECN; ++l) { 
		  ectab_t *e = &bectab[i][j][k][l];
    	  fprintf(stderr,"{%u,%u}%c", e->cw, e->cl,l==ECN-1?' ':',');
		} fprintf(stderr,"}%c",                    k==ECN-1?' ':',');
	  } fprintf(stderr,"}%c",                      j==ECN-1?' ':',');
    } fprintf(stderr,"}%c",                        i==ECN-1?' ':',');
  }  fprintf(stderr,"};/*%d*/\n", 1<<ECN);  
   exit(0);
      #endif
    #endif
}
 
#define BECENC  becenc
#define BECENC_ becenc_
#define BECDEC  becdec
#define BECDEC_ becdec_

#define USIZE 8
#include "bec_.c"

#define USIZE 16
#include "bec_.c"
