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
// Bit entropy coder 
// Based on: "A quick explaination of the M99 compression algorithm" http://www.michael-maniscalco.com/blog.htm
//           https://github.com/michaelmaniscalco/m99
#define uint_t T3(uint,USIZE,_t)
#define sym_t  T2(sym_t,USIZE)
#define bec_t  T2(bec_t,USIZE)
typedef struct { unsigned n; uint16_t c; } _PACKED sym_t;						// occurrence n, symbol c
typedef struct { stm_t s; sym_t *lo, *hi, _lo[32*(1 << USIZE)], _hi[32*(1 << USIZE)]; cw_t cw[(1 << USIZE)]; } bec_t; // stream object, pointer stack low/high to symbol table cw

static void T2(BECENC_,USIZE)(bec_t *t, uint_t *in, unsigned n, unsigned lon, sym_t *r, unsigned lor) {
  if(lor >= n) r->c = *in, r->n = n;
  else if(n < 3) { 																
    if(n == 1) r->c = *in, r->n = 1;											// 1 symbol
	else { int u = (int)in[0] - (int)in[1];										// 2 symbols
            r->c = in[u >= 0];  r->n    = !u + 1; 
        (r+1)->c = in[u <  0]; (r+1)->n = 1; if(u)								stmputx(&t->s, 1, u<0);
    }
  }	else { unsigned hir, m; sym_t *c; 								            // recursive partitioning					
	if(likely(lor <= lon)) hir = T2(memrun,USIZE)(in+lon, in+n);                // run
	else                   hir = lor - lon;
    sym_t *lo = t->lo, *hi = t->hi; t->lo += (1 << USIZE); t->hi +=(1 << USIZE);// stack
    T2(BECENC_,USIZE)(t, in+lon, n-lon, (n-lon) >> 1, hi, hir);					// encode low partition 
    T2(BECENC_,USIZE)(t, in,       lon,    lon  >> 1, lo, lor);					// encode high partition
	
    sym_t *lohi[2]; cw_t *cw=t->cw; unsigned loc,hic,ln=lon,hn=n-lon; lohi[0] = lo; lohi[1] = hi; 
	do {
	  unsigned c0 = lohi[0]->c, c1 = lohi[1]->c;
      loc      = -(c0 <= c1) & lohi[0]->n; 
      hic      = -(c1 <= c0) & lohi[1]->n;	  
	  r->c     = lohi[!loc]->c;
      lohi[0] += (loc!=0);       	  
	  lohi[1] += (hic!=0);        	  
	  cw->l    = loc; cw->n = (r++)->n = loc + hic;  cw->hx = hn; hn-=hic; (cw++)->lx = ln; ln-=loc; // store in temp array cw
    } while(ln && hn); 	
	for(m = ln + hn, c = lohi[ln == 0]; m > 0;) m -= c->n, *r++ = *c++;		   
	while(cw-- != t->cw) 														stmput(&t->s,cw->l, cw->n, cw->lx, cw->hx);	// encode cw in reverse order
    t->lo -= (1 << USIZE); t->hi -= (1 << USIZE);								//stack
  }
}

unsigned T2(BECENC,USIZE)(uint_t *__restrict in, unsigned _inlen, unsigned char *__restrict out) {  
  #define OV 8 
  unsigned inlen = USIZE==16?((_inlen+1)/2):_inlen,len[(1 << USIZE)],i;         //TODO:better processing for 16bits
  sym_t stab[(1 << USIZE)] = {0};
  bec_t t; t.lo = t._lo; t.hi = t._hi; bectabini(bectab);				    	stm_t *s = &t.s; stmeini(s, out, _inlen); stmseek(s, 0, OV);
  uint_t *in_=in+inlen, *ip = in, c = *ip; while(ip < in_ && *ip == c) ip++; unsigned lor = ip - in; 	
  T2(BECENC_,USIZE)(&t, in, inlen, pow2next(inlen) >> 1, stab, lor);		    // encode
																
  for(i = 0; inlen; inlen -= stab[i++].n) len[i] = inlen;   				    // write the symbol table                 
  for(int j = i-1; j >= 0; --j) {												stmputx(s,USIZE, stab[j].c); stmput(s,stab[j].n, len[j], len[j], len[j]); }
																				return stmflush(s);
} 

static void T2(BECDEC_,USIZE)(bec_t *t, uint_t *out, unsigned n, unsigned lon, sym_t *stab) {
  if(stab->n >= n) { unsigned c = stab->c; memset_(out, c, n); }
  else if(n < 3) {
    if(n == 1) *out = stab->c;
	else { unsigned c; 														    stmgetx(&t->s, 1, c);
      out[!c] =  stab->c; 
      out[ c] = (stab+1)->c;
    }
  } else { unsigned hin = n-lon, loc,hic, ln=lon, hn=hin; sym_t *lo=t->lo, *hi=t->hi; stm_t *s = &t->s; STMSAVE(s);   
    do {																		stmget_(stab->n, ln, hn, loc); 		 		                  
	  lo->c = hi->c = stab->c; hic = (stab++)->n - loc;
	  lo += (lo->n = loc) != 0; ln -= loc; 
	  hi += (hi->n = hic) != 0; hn -= hic;		
    } while(ln && hn);														    STMREST(s); 
	for(lo = ln?lo:hi, ln += hn; ln > 0; ln -= stab->n, *lo++ = *stab++);
	  
	lo = t->lo; hi = t->hi; t->lo += (1 << USIZE); t->hi += (1 << USIZE);		// symbol table stack
    T2(BECDEC_,USIZE)(t, out,     lon, lon >> 1, lo);                           // decode low partition 
    T2(BECDEC_,USIZE)(t, out+lon, hin, hin >> 1, hi);	                        // decode high partition
    t->lo -= (1 << USIZE); t->hi -= (1 << USIZE);                               // symbol table stack
  }
} 
  
unsigned T2(BECDEC,USIZE)(unsigned char *__restrict in, unsigned _outlen, uint_t *__restrict out) {
  unsigned outlen = USIZE==16?((_outlen+1)/2):_outlen, i,n = outlen;
  sym_t stab[(1 << USIZE)];   									
  bec_t t; 	t.lo = t._lo; t.hi = t._hi; 										stm_t *s = &t.s; stmdini(s, in, 0); STMSAVE(s); // read the symbol table
  for(i = 0; n; n -= stab[i++].n) { 											stmget_(n, n, n, stab[i].n); stmgetx_(USIZE,  stab[i].c); } STMREST(s); 
  T2(BECDEC_,USIZE)(&t, out, outlen, pow2next(outlen) >> 1, stab);              // Decode
}
