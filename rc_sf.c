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
// TurboRC: Range Coder encode/decode functions using fsm predictor 

#include <stdio.h>             
#include "conf.h"   
#include "turborc.h"   

#define RC_MACROS
#define RC_BITS 15      	// RC_SIZE=64 + RC_IO=32 : set in turborc_.h
#include "turborc_.h"
#include "mbc_sf.h"       	// fsm predictor

fsm_t fsm[N_STATES];

#define BUFSIZE (1u<<20)
void fsm_init(int id) {      					
  char     s[256], buf[BUFSIZE];
  unsigned l;
  sprintf(s, "FSM%d.txt", id);
  FILE *f = fopen(s, "r"); 					    if(!f) { fprintf(stderr, "FSM file '%s' not found\n", s); exit(0); } 
  if((l = fread(buf, 1, BUFSIZE, f)) == -1) 
	perror("fread failed\n");   				
  printf("fsm file '%s' read.length=%d\n", s, l);
  buf[l] = 0; 
  fsminit_(buf, fsm, N_STATES);
}
	
#include "rc_.c"      // template functions 
