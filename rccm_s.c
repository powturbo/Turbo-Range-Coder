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
// TurboRC: Range Coder - CM predictor (16 bits counters)

#include <stdio.h>             
#include "include_/conf.h"   

#define RC_MACROS
#define RC_BITS 16     	    // RC_SIZE + RC_IO: set in turborc_.h
#include "turborc_.h"

void ssebinit(unsigned short sse2[1<<(1+8)][17]) {  int r,i,j;	
  for(r=0; r<2; ++r)
    for(i=0; i<256; ++i)
      for(j=0; j<=16; ++j)
        sse2[r<<8|i][j] = (j<<(RC_BITS-4))-(j==RC_BITS);
}

void sseinit( unsigned short sse[1<<8][17]) { int i,j;
  for(i=0; i<256; ++i)
    for(int j=0; j<=16; ++j)
      sse[i][j] = (j<<(RC_BITS-4))-(j==RC_BITS);
}		

#define RATE_S
#include "mbc_s.h"       	// simple predictor
#include "rccm_.c"          // template functions 
