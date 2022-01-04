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
// TurboRC: Range Coder - fsm predictor
// Reference: https://encode.su/threads/3681-GDCC-21-T5-FSM-Counter-Optimization 
#include <stdlib.h> // strtol

#define RC_PRDID 3
#define RC_PRD   sf
#define RCPRM1 0
#define RCPRM  ,fsm_t *RCPRM0
#define RCPRMC ,RCPRM0

#if RC_BITS < 15
#error "RC_BITS must be >= 15"
#endif
#define MBU_PRM 0
#define mbu_probinit() 0

#define N_STATES 32768

typedef unsigned short mbu;

#define mbu_p(_mb_,_fsm_) (_fsm_[*(_mb_)].p)										
#define mbu_init(_m_, _p0_) { *(_m_) = _p0_; }          //static inline unsigned mbu_p(mbu *mb, fsm_t *_fsm_) { return fsm[*mb].p; }

#define mbu_update0(_mb_, _mbp_, _fsm_, _prm1_) (*(_mb_) = _fsm_[*(_mb_)].s[0])
#define mbu_update1(_mb_, _mbp_, _fsm_, _prm1_) (*(_mb_) = _fsm_[*(_mb_)].s[1])

#define mbu_update( _mb_, _mbp_, _fsm_, _prm1_, _bit_) (*(_mb_) = _fsm_[*(_mb_)].s[_bit_])

  #ifndef min
#define min(x,y) (((x)<(y)) ? (x) : (y))
#define max(x,y) (((x)>(y)) ? (x) : (y))
  #endif

static inline unsigned fsmget_(unsigned char **_p) {
  unsigned char *p = *_p,*e;
  int c, r = 0;
  while(*p && (*p<'0' || *p>'9')) p++;
  r = strtoul(p, &e, 10);
  *_p = e; 														//if(r) printf("%d,", r);
  return r;
}

static inline void fsminit_(unsigned char *p, fsm_t *fsm, unsigned nstates) {
  int i,m;
  memset(fsm, 0, sizeof(fsm[0])*nstates);
  for(i = 0; i < nstates; i++) {
    m = fsmget_(&p); fsm[i].s[1] = max(0,min(nstates-1,      m));
    m = fsmget_(&p); fsm[i].s[0] = max(0,min(nstates-1,      m));
    m = fsmget_(&p); fsm[i].p    = max(1,min((1<<RC_BITS)-1, m));
	if(!*p) break;												//if(fsm[i].s[0] || fsm[i].s[1]) printf("%d,%d,%d ", fsm[i].s[0], fsm[i].s[1], fsm[i].p);
  }
}

#include "mbc.h"
