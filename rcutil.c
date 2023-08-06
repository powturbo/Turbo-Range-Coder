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
// TurboRC: Range Coder - misc. functions
#include <stdio.h>             
#include <string.h>
#include <ctype.h>
#include <math.h> //isnan
#include "include_/conf.h"
#include "include/turborc.h"

#include "include_/rcutil.h" // #include <float.h> //DBL_MAX
#include "rcutil_.h"

//-------------------------------- malloc ----------------------------------------
  #ifdef _WIN32
#include <windows.h>
static SIZE_T largePageSize = 0, vinit_ = 0;

static void vinit(int enable) { 
  if(vinit_) return; vinit_++;
  HANDLE hToken = 0;
  if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    TOKEN_PRIVILEGES tp = {0};
    if(LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &tp.Privileges[0].Luid)) {
      tp.PrivilegeCount           = 1;
      tp.Privileges[0].Attributes = enable?SE_PRIVILEGE_ENABLED:0;
      if(AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, 0)) {
		  #ifndef NDEBUG
	    unsigned rc = GetLastError();
		if(rc == ERROR_SUCCESS)
		  largePageSize = GetLargePageMinimum();
	    else printf("AdjustTokenPrivileges.rc=%d ", rc);
		  #endif
	  }
    }
    CloseHandle(hToken);
  }

  HMODULE hKernel;
  if(hKernel = GetModuleHandle(TEXT("kernel32.dll"))) {
    typedef SIZE_T (WINAPI * GetLargePageMinimumProcT)();

    GetLargePageMinimumProcT largePageMinimumProc = (GetLargePageMinimumProcT)GetProcAddress(hKernel, "GetLargePageMinimum");
    if(largePageMinimumProc) {
      largePageSize = largePageMinimumProc();
      if ((largePageSize & (largePageSize - 1)) != 0) largePageSize = 0;
    }
  }														  //printf("LP=%d ", (int)largePageSize);	
} 
  #endif

void *vmalloc(size_t size) {
    #ifdef _WIN32
  vinit(1);
  if(largePageSize /*&& largePageSize <= (1 << 30)*/ && size >= (size_t)(1 << 18)) { 
    void *rc = VirtualAlloc(0, (size + largePageSize - 1) & (~(largePageSize - 1)), MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
    //if(!rc) printf("VAlloc failed rc=%d ", (int)GetLastError()); else printf("LP=%lld/%zd ", largePageSize, size); 
	if(rc) return rc; //printf("LP=%d rc=%d ", (int)largePageSize, (int)rc);	
  }
  return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
    #else
  return malloc(size);
    #endif
}

void vfree(void *address) {
    #ifdef _WIN32
  VirtualFree(address, 0, MEM_RELEASE);
    #else
  free(address);
    #endif
}

//--------------------------- convert i/o to big endian -----------------------------
  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BSWAP16(a) a 
#define BSWAP32(a) a 
#define BSWAP64(a) a 
  #else
#define BSWAP16(a) bswap16(a)
#define BSWAP32(a) bswap32(a)
#define BSWAP64(a) bswap64(a)
  #endif
//--------------------------- lzp preprocessor (lenmin >= 32) ------------------------------------------------------------------------  
  #ifndef NCOMP
#define H_BITS              16										 // hash table size 
#define emitmatch(_l_,_op_) { unsigned _l = _l_-lenmin+1; *_op_++ = 255; while(_l >= 254) { *_op_++ = 254; _l -= 254; OVERFLOW(in,inlen,out,op,goto end); } *_op_++ = _l; }
#define emitch(_ch_,_op_)   { *_op_++ = _ch_; if(_ch_ == 255) *_op_++ = 0; OVERFLOW(in,inlen, out, op, goto end); }
 
#define LZPINI(_n_) if(lenmin < 32) lenmin = 32;\
  if(_n_ < lenmin) { memcpy(out, in, _n_); return _n_;}\
  if(!hbits) { hbits = _n_ >= (1<<24)?21:H_BITS; }\
  hbits = hbits>12?hbits:12;\
  if(hbits > H_BITS && !(htab = calloc(1<<hbits, 4))) { htab = _htab; hbits = H_BITS; }

size_t lzpenc(unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out, unsigned lenmin, unsigned hbits) {	//printf("m=%u ", lenmin);
  unsigned      _htab[1<<H_BITS] = {0}, *htab = _htab, cl, cx, h4 = 0;
  unsigned char *ip = in, *cp, *op = out;
  LZPINI(inlen);										//unsigned cnt = 1<<hbits, lmin = lenmin, ocnt=0;
  for(cx = ctou32(ip), ctou32(op) = cx, cx = BSWAP32(cx), op += 4, ip += 4; ip < in+inlen-lenmin;) { 
    h4       = HASH(h4, cx);  
    cp       = in + htab[h4];                                                        //cnt -= htab[h4] == 0; //if(cnt*100/(1<<hbits) != ocnt) { printf("%d ", cnt); ocnt=cnt*100/(1<<hbits); }                                                                    //unsigned p = cnt * 100 / (1<<hbits); if(lmin > 32 && lmin <= lenmin && ip - in > (1<<20)) { if(p < 70) lmin--; else lmin++; printf("l=%u ", lmin);}
    htab[h4] = ip - in;    
    if(ctou64(ip) == ctou64(cp) && ctou64(ip+8) == ctou64(cp+8) && ctou64(ip+16) == ctou64(cp+16) && ctou64(ip+24) == ctou64(cp+24)) { // match
      for(cl = 32;;) {
        if(ip+cl >= in+inlen-32) break;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl += 8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl += 8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl += 8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl += 8;
      }						                                           //unsigned x = PREDLEN(avg, cl); printf("%u,", x);												  
      if(cl >= lenmin) {
        for(; ip+cl < in+inlen && ip[cl] == cp[cl]; cl++);
        emitmatch(cl, op);
        ip += cl;				
        cx  = BSWAP32(ctou32(ip-4));		        
        continue;
      }
    }
    unsigned ch = *ip++; emitch(ch, op); cx = cx<<8 | ch; // literal
  }
  while(ip < in+inlen) { unsigned c = *ip++; emitch(c, op); }   //unsigned cnt = 0; for(int i=0; i < (1<<hbits); i++) cnt += htab[i] != 0;   printf("c=%u %.2f", cnt, (double)cnt*100.0/(1<<hbits) );	
  end:if(htab != _htab) free(htab);
  return op - out;													
}
  #endif

  #ifndef NDECOMP
size_t lzpdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lenmin, unsigned hbits) {
  unsigned      _htab[1<< H_BITS] = {0}, *htab = _htab, cx, h4 = 0;  
  unsigned char *ip = in, *op = out;
  LZPINI(outlen);
  for(cx = ctou32(ip), ctou32(op) = cx, cx = BSWAP32(cx), op += 4, ip += 4; op < out+outlen;) {
    unsigned c;    h4 = HASH(h4, cx);
    unsigned char *cp = out + htab[h4],*op_;
             htab[h4] = op - out;
    if((c = *ip++) == 255)
      if(*ip) {
        c = 0; do c += *ip; while(*ip++ == 254);
        for(op_ = op+c+lenmin-1; op < op_; *op++ = *cp++);
        cx = BSWAP32(ctou32(op-4));
        continue;
      } else ip++, c = 255;
    cx = cx << 8 | (*op++ = c);
  }
  if(htab != _htab) free(htab);
  return ip - in;
}
  #endif

  #ifndef NCOMP
// --------------------------------- QLFC - Quantized Local Frequency Coding ------------------------------------------
// QLFC=MTF-Backward: number of different symbols until the next occurrence (number of symbols will be seen before the next one)
// MTF:  number of different symbols since the last occurence  (number of symbols were seen until the current one)
// References: https://ieeexplore.ieee.org/document/1402216
uint8_t *rcqlfc(uint8_t *__restrict in, size_t n, uint8_t *__restrict out, uint8_t *__restrict r2c) {
  unsigned char f[1<<8] = {0};
  uint8_t       *ip, *op = out;
  int           m;
  for(m = 0; m < (1<<8); m++) r2c[m] = m;					//if(!in[n-1]) r2c[0] = 1, r2c[1] = 0;
  for(m = -1,ip = in+n; ip > in; ) {     
    uint8_t *p, c = *--ip;  //--------- run length ---------------------------
	MEMDEC8(cv, c); MEMRUNR8(in,ip,cv,c,goto a);
	a:;
    //-------------- mtf: move to front -----------------------------------
 	  #if 0 //combined search + move to front.
	unsigned c0, c1;
    for(c0 = r2c[0], r2c[0] = c,p = &r2c[1];;p += 4) {
      c1 = p[0]; p[0] = c0; if(c1 == c)           break;
      c0 = p[1]; p[1] = c1; if(c0 == c) { p++;    break; }
      c1 = p[2]; p[2] = c0; if(c1 == c) { p += 2; break; }
      c0 = p[3]; p[3] = c1; if(c0 == c) { p += 3; break; }
    } 
      #else	  
	uint8_t *pb=NULL,*q=NULL; p = r2c; // search
        #ifdef __AVX2__
	for(;;) { unsigned m = _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i*)p), cv)); if(m) { p += ctz32(m); break; } p += 32;}
        #elif defined(__SSE__)
    for(;;) { uint16_t m =    _mm_movemask_epi8(   _mm_cmpeq_epi8(   _mm_loadu_si128((__m128i*)p), cv)); if(m) { p += ctz16(m); break; } p += 16; }
        #else
	while(*p != c) p++;
        #endif
	for(q = p; q > r2c; ) { // move to front 	
		  #ifdef __AVX2__ 
      q -= 32; _mm256_storeu_si256(q+1,_mm256_loadu_si256((__m256i*)q)); 
          #elif defined(__SSE__)
      q -= 16; _mm_storeu_si128(q+1,   _mm_loadu_si128((__m128i*)q));
		  #else
	  q -= 16; ctou64(q+1+8) = ctou64(q+8); ctou64(q+1) = ctou64(q); 
          #endif 
	} 	
	r2c[0] = c;	
	  #endif
    if(!f[c]) *op++ = m++,f[c]++; else *op++ = p-r2c-1;
  }  
  return op;
} 
  #endif 

//------------------------------------------- utf8 preprocessing -----------------------------------------------
  #ifndef NCOMP
#include "include_/vlcbyte.h" // vsput/vsget

//-- utf-8 to code point ----------
#define UTF8_INV 0x10fe00  // with 3 bytes private area
#define UTF8LENMASK(_c_, _m_, _l_) \
       if ((_c_ & 0xe0) == 0xc0) { _l_ = 2; _m_ = 0x1f; }\
  else if ((_c_ & 0xf0) == 0xe0) { _l_ = 3; _m_ = 0x0f; }\
  else if ((_c_ & 0xf8) == 0xf0) { _l_ = 4; _m_ = 0x07; }\
  else if ((_c_ & 0xfc) == 0xf8) { _l_ = 5; _m_ = 0x03; }\
  else if ((_c_ & 0xfe) == 0xfc) { _l_ = 6; _m_ = 0x01; }\
  else { _l_ = 0; _m_ = 0xff; } /*invalid*/

#define UTF8GET(_ip_, _m_, _l_, _c_) do { int _ui;\
  _c_ &= (_m_);\
  for(_ui = 1; _ui < (_l_); _ui++) {\
    if(((_ip_)[_ui] & 0xc0) != 0x80) { _l_ = 0; break; }\
    _c_ = (_c_) << 6 | ((_ip_)[_ui] & 0x3f);\
  }\
} while(0)

#define UCGET(_ip_, _c_, _l_) {\
  if((_c_ = *_ip_) < 128) { _ip_++; _l_ = 1; }\
  else {\
    unsigned _mask, _c = _c_;\
    UTF8LENMASK(_c_, _mask, _l_);\
	UTF8GET(_ip_, _mask, _l_, _c_); _ip_ += _l_;\
	if(!_l_) { _c_ = UTF8_INV + _c; _ip_++; } /*invalid -> private area*/\
  }\
}

//------------------------- utf-8 (1-gramm)) preprocessor ---------------------------------------------------------	
#pragma pack(1)
typedef struct { unsigned c, cnt; /*unsigned short v;*/ } _PACKED sym_t;  // c=key,  cnt=count, v=value (=cid function)
#pragma pack() 

static ALWAYS_INLINE unsigned cid(unsigned c) {                            // utf-8 classification
  if(c >=      0  && c <=     0xff) return 0; // 1 byte
  if(c >=  0x2E80 && c <=   0x2F00) return 2; // cjk radicals supplement
  if(c >=  0x3400 && c <=   0x4DC0) return 2; // 2 bytes
  if(c >=  0x4E00 && c <=   0xA000) return 2;
  if(c >=  0x9FA6 && c <=   0x9FCC) return 2;
  if(c >=  0xF900 && c <=   0xFB00) return 2; // compatibility ideographs
  if(c >     0xff && c <=   0xffff) return 1; 
  if(c >= 0x20000 && c <=  0x2A6E0) return 3; // 3 bytes 
  if(c >= 0x2A700 && c <   0x2B740) return 3; 
  if(c >= 0x2B740 && c <   0x2B820) return 3;
  if(c >= 0x2B820 && c <=  0x2CEB0) return 3; //included as of Unicode 8.0
  if(c >= 0x2F800 && c <=  0x2FA20) return 3;   
  if(c >   0xffff && c <= 0xffffff) return 4; 
  return 5;                                   // 4 bytes
}

// qsort compare functions --------------------------------
#define SC(_x_) cid(_x_)

#define CMPA(_a_,_b_,_t_)         ((*(_t_ *)(_a_) > *(_t_ *)(_b_)) - (*(_t_ *)(_a_) < *(_t_ *)(_b_)))
#define CMPD(_a_,_b_,_t_)         ((*(_t_ *)(_a_) < *(_t_ *)(_b_)) - (*(_t_ *)(_a_) > *(_t_ *)(_b_)))

#define CMPSA(_a_,_b_, _t_, _v_)  (((((_t_ *)_a_)->_v_) > (((_t_ *)_b_)->_v_)) - ((((_t_ *)_a_)->_v_) < (((_t_ *)_b_)->_v_)))
#define CMPSD(_a_,_b_, _t_, _v_)  (((((_t_ *)_a_)->_v_) < (((_t_ *)_b_)->_v_)) - ((((_t_ *)_a_)->_v_) > (((_t_ *)_b_)->_v_)))

#define CMPSFA(_a_,_b_, _t_, _v_) (( SC(((_t_ *)_a_)->_v_) > SC(((_t_ *)_b_)->_v_)) - ( SC(((_t_ *)_a_)->_v_) < SC(((_t_ *)_b_)->_v_)))

// qsort compare: a=ascending  d=descending
// unsigned 
static int cmpua(const void *a, const void *b) { return CMPA(a,b,uint32_t); }
static int cmpud(const void *a, const void *b) { return CMPD(a,b,uint32_t); }

// struct sym_t : n = count, c = key.  na:count ascending, nd:count descending,  ca: key c ascending
static int cmpsna(const void *a, const void *b) { return CMPSA(a,b,sym_t,cnt); }
static int cmpsnd(const void *a, const void *b) { return CMPSD(a,b,sym_t,cnt); }
static int cmpsca(const void *a, const void *b) { return CMPSA(a,b,sym_t,c  ); }

// struct sym_t : n = count, c = key, v = value
static int cmpsndca(const void *a, const void *b) { int c; if(c = CMPSD(a,b,sym_t,cnt)) return c; return CMPSA(a,b,sym_t,c  ); }
static int cmpscand(const void *a, const void *b) { int c; if(c = CMPSFA(a,b,sym_t,c))  return c; return CMPSD(a,b,sym_t,cnt); }

// symbol table output 
static unsigned char *symsput(sym_t *stab, unsigned stabn, unsigned char *out, unsigned flag) {	
  unsigned char *op = out+1, *p; 
  out[0] = flag;

  ctou16(op) = stabn; op += 2; p = op; op += 1+2+2;                    
  unsigned i=0, v=0,u;
         while(i < stabn) { if(stab[i].c >     0xff) break;               *op++      = stab[i++].c; }                                   *p    = i;
  v = 0; while(i < stabn) { if(stab[i].c >   0xffff) break;               ctou16(op) = BSWAP16(stab[i++].c);           op += 2; } ctou16(p+1) = i;
  v = 0; while(i < stabn) { if(stab[i].c > 0xffffff) break; u = stab[i++].c; *op++   = u>>16; ctou16(op) = BSWAP16(u); op += 2; } ctou16(p+3) = i;
  v = 0; while(i < stabn) {                                               ctou32(op) = BSWAP32(stab[i++].c);           op += 4; }
  return op;
}

#define NOHASH
  #ifdef NOHASH																	
#define CGET(    _a_, _htab_,_hbits_,_hmask_, _c_, _i_)      { _i_ = _htab_[_c_]-1; }       // get index i in a for a previously added key c														
#define CFIND(   _a_, _htab_,_hbits_,_hmask_, _c_, _i_, _h_) { _i_ = _htab_[_h_ = _c_]-1; } // return index i of key c if found, otherwise -1
#define CREHASH( _a_, _htab_,_hbits_,_hmask_, _n_)           { unsigned _i; memset(_htab_, 0, (1<<_hbits_)*sizeof(_htab_[0])); for(_i = 0; _i < _n_; _i++) { unsigned _h = _a_[_i].c; _htab_[_h] = _i+1;}  }
  #endif

size_t utf8enc(unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out, unsigned flag) { 
  #define SYMBITS     16
  unsigned char *ip, *op = out, cinv = 0;
  sym_t          stab[1<<SYMBITS] = {0};
  
    #ifdef NOHASH
  #define HBITS 24    // 16*2 = 32MB
  unsigned short *stabh = calloc((size_t)(1<<HBITS), sizeof(stabh[0])); if(!stabh) die("utf8enc: calloc failed size=%z\n", 1<<HBITS);
	#else
  #define HBITS (SYMBITS+1)
  #define HMASK ((1<<HBITS)-1)
  unsigned short stabh[1<<  HBITS] = {0};        
    #endif
  unsigned       stabn = 0, xprep8 = flag & BWT_PREP8, verbose = flag & BWT_VERBOSE, itmax = (flag>>10) & 0xf, xsort = (flag>>14) & 0x3, cnt;
                                                                                if(verbose) { printf("utf8:sort=%u prep8=%d ", xsort, xprep8); fflush(stdout); } //unsigned st_crd=0;
  for(ip = in; ip < in+inlen;) {												// build the symbol dictionary
    unsigned c, ci, l = 1, h;											   	
    UCGET(ip, c, l); 			   
	  #ifdef UTF8INV
	if(!l) { op = out+inlen; 													if(verbose) { printf("invalid utf8 ");fflush(stdout); } 
	  goto e; 
	} 																			// convert to code point + utf-8 validity check //if(c > cmax) cmax = c;
	  #else
	if(!l) {                                                                    if(verbose) { printf("#"); fflush(stdout); }
	  if(++cinv > 16) { op = out+inlen;                                         if(verbose) { printf("invalid utf8 symbols"); fflush(stdout); } 
        goto e;
	  }
	} else if(c >= UTF8_INV && c <= UTF8_INV+0xff) { op = out+inlen;            if(verbose) { printf("?"); fflush(stdout); } goto e; } // symbol not allowed
	  #endif
    CFIND(stab, stabh,HBITS,HMASK, c, ci, h); 
	if(ci != -1) stab[ci].cnt++; 
	else { 	
	  if(stabn >= (1<<SYMBITS)-1
	      #ifdef NOHASH
	    || c >= (1<<HBITS)
		  #endif
	    ) { 											                        // max. number of symbols is 64k
	    op = out+inlen; 														if(verbose) { if(c >= (1<<HBITS)) printf("utf-8 overflow\n");else printf("number of symbol > 64k\n");fflush(stdout); } 
	    goto e; 
	  }  						
	  CADD(stab, stabh, stabn, c, h, cid(c)); 
	}
  }
																				if(verbose) { printf("sym=%u sort='%s' ", stabn, xsort?"freq":"cat");fflush(stdout); } 
  switch(xsort) {
    case 0: qsort(stab, stabn, sizeof(sym_t), cmpscand); break;					// sort by code group + count (bwt mode)
    case 1: qsort(stab, stabn, sizeof(sym_t), cmpsnd); break;  					// sort by count   
  }
  
  unsigned cnt8 = 0; cnt = 0; 
  for(int i = 0; i < stabn; i++) { 
    if(stab[i].c <= 0xff) cnt8 += stab[i].cnt; 
	cnt += stab[i].cnt; 
  } 
  cnt8 = (uint64_t)cnt8*128 / cnt; 						                        if(verbose) { printf("ratio=%u ", cnt8);fflush(stdout); } 		
  if(cnt8 > 64 && !(flag & BWT_RATIO)) { op = out+inlen; goto e; }                    								    // enough saving for converting to 16-bits?

  CREHASH(stab, stabh,HBITS,HMASK, stabn);		                                // rehash after sort
  op = out+4; 														
  if(itmax <= 1 || !xprep8) op = symsput(stab, stabn, op, xprep8?1:0);		    // output the dictionary 				
  unsigned hdlen = op - out; 
  if(hdlen & 1) *op++ = 0; 														// offset to data must be even for 16 bits bwt
  
  if(itmax>1 || !xprep8) { 							                            if(verbose) { printf("'16 bits output' "); fflush(stdout); } 
    if(op + cnt*2 >= out+(inlen*255)/256-8) { op = out+inlen; goto e;}; 		// check overflow in case of 16 bits
	for(ip = in; ip < in+inlen;) {                    							// fixed length encoding: 16-bits
	  unsigned c, l, ci; 
	  UCGET(ip, c, l); 															
	  CGET(stab, stabh,HBITS,HMASK, c, ci); 											
	  ctou16(op) = BSWAP16(ci); op += 2;                                        
	}
  } else  {	                               										if(verbose) { printf("'8-16 bits output' ");fflush(stdout); }
    for(ip = in; ip < in+inlen;) { 												// variable length encoding: 8-16 bits
      unsigned c, l, ci; 
      UCGET(ip, c, l); 
	  CGET(stab, stabh,HBITS,HMASK, c, ci); 
	  vsput20(op, ci); 															OVERFLOW(in,inlen,out, op, goto e);  
	}
  }	ctou32(out) = op - out;														if(verbose) { printf("len='%u' ", (unsigned)(op-out));fflush(stdout); }																				
  e: 
    #ifdef NOHASH
  if(stabh) free(stabh);
    #endif   
  if(op >= out+inlen) { 
    op = out+inlen; 
	if(flag & BWT_COPY) memcpy(out,in, inlen); 
  }
  return op - out;															
}
  #endif

  #ifndef NDECOMP
//--------------------------- decode ---------------------
#define ctoutf8(_u_, _c_, _l_) {\
  unsigned _c = _c_;\
  if(likely(_c <=   0x7f)) { _u_ = _c;                                                                                    _l_ = 1;}\
  else if(  _c <=   0x7ff) { _u_ =      0x80c0 | (_c & 0x3f) << 8 | _c >>  6;                                             _l_ = 2;}\
  else if(  _c <=  0xffff) { _u_ =    0x8080e0 | (_c & 0x3f) <<16 | ((_c & 0xfc0) >>  6) << 8 | _c >> 12;                 _l_ = 3;}\
  else if(  _c >= UTF8_INV && _c <=  UTF8_INV+0xff) { _u_ = _c - UTF8_INV;                                                _l_ = 3;}\
  else                     { _u_ = 0x808080f0u | (_c & 0x3f) <<24 |  (_c & 0xfc0) << 10 | (_c & 0x3f000) >> 4 | _c >> 18; _l_ = 4;}\
}

unsigned symsget(unsigned *sym, unsigned char **_in, unsigned *flag) {
  unsigned char *in = *_in, *ip = in;
  unsigned      m0,m1,m2,stabn,i = 0; 
  
  *flag = *ip++;  
  stabn = ctou16(ip); ip+=2;										    
  m0    = *ip++;
  m1    = ctou16(ip); ip += 2;
  m2    = ctou16(ip); ip += 2;  															
  while(i < m0) sym[i++] =                                              *ip++;                       					
  while(i < m1) sym[i++] = BSWAP16(ctou16(ip)),                         ip+=2;
  while(i < m2) sym[i++] = (unsigned)ip[0]<<16 | BSWAP16(ctou16(ip+1)), ip+=3; 		
  while(i < stabn) sym[i++] = BSWAP32(ctou32(ip)),                      ip+=4;        			

  if((ip-in) & 1) ip++; // align to even offset for 16-bits bwt. 
  *_in = ip;
  return stabn;
}

size_t utf8dec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out) {
    unsigned char* op = out, * ip = in + 4, len[1 << 16] = { 0 };
  unsigned i,flag, sym[1<<16], inlen = ctou32(in),stabn = symsget(sym, &ip, &flag); 

  for(i = 0; i < stabn; i++) { 
    unsigned u,l; 
	ctoutf8(u, sym[i], l); 
	sym[i] = u; 
	len[i] = l; 
  }
  if(flag & 1) { 																// variable length encoding: 8-16 bits
    #define ST { unsigned u; vsget20(ip, u); ctou32(op) = sym[u]; op+=len[u]; }
	while(ip < in+inlen-8-4) { ST; ST; ST; ST; }
	while(ip < in+inlen) { 
	  unsigned i,u; 
	  vsget20(ip, i); 
	  u = sym[i]; 
	  switch(len[i]) {
		case 4: ctou32(op) = u; op+=4; break;
		case 3: op[0] = (uint8_t)u; op[1] = (uint16_t)u>>8; op[2] = (uint8_t)(u>>16); op+=3; break;
		case 2: ctou16(op) = u; op+=2; break;
		case 1: *op++ = u; 
	  }
	}
  } else { 																		// fixed length encoding: 16-bits
	#define ST(_i_) { unsigned u = BSWAP16(ctou16(ip+_i_*2)); ctou32(op) = sym[u]; op+=len[u]; }
	for(; ip < in+(inlen-8-4); ip += 8) { ST(0); ST(1); ST(2); ST(3); }
	for(; ip < in+inlen   ;    ip += 2) {
      unsigned i = BSWAP16(ctou16(ip)), u = sym[i]; 
	  switch(len[i]) {
		case 4: ctou32(op) = u;                                                       op += 4; break;
		case 3: op[0] = (uint8_t)u; op[1] = (uint16_t)u>>8; op[2] = (uint8_t)(u>>16); op += 3; break;
		case 2: ctou16(op) = u;                                                       op += 2; break;
		case 1: *op++ = u; 
	  }
    }
  }
  return 0;
}
  #endif

//--------------------------- Turbohist: https://github.com/powturbo/Turbo-Histogram ----------
#define CSIZE (256 + 8)
typedef unsigned cnt_t;

#define HISTEND(_c_,_cn_,_cnt_) { int _i,_j;\
  memset(_cnt_, 0, 256*sizeof(_cnt_[0]));\
  for(_i=0; _i < 256; _i++)\
    for(_j=0; _j < _cn_;_j++) _cnt_[_i] += _c_[_j][_i];\
}	

#define HISTEND8(_c_,_cnt_) HISTEND(_c_,8,_cnt_)
#define HISTEND4(_c_,_cnt_) HISTEND(_c_,4,_cnt_)

//--------------------  64 bits ------------------------------------
  #if defined(__i386__) || defined(__x86_64__) 
#define CU64(_u_,_o_,_c_) { unsigned _x = _u_;\
  c[0    ][(unsigned char )_x    ]+=_c_;\
  c[1    ][(unsigned short)_x>> 8]+=_c_; _x>>=16;\
  c[2    ][(unsigned char )_x    ]+=_c_;\
  c[3    ][(unsigned short)_x>> 8]+=_c_; _x=(_u_)>>=32;\
  c[0+_o_][(unsigned char )_x    ]+=_c_;\
  c[1+_o_][(unsigned short)_x>> 8]+=_c_; _x>>=16;\
  c[2+_o_][(unsigned char )_x    ]+=_c_;\
  c[3+_o_][(unsigned short)_x>> 8]+=_c_;\
}
  #else
#define CU64(_u_,_o_,_c_) { unsigned _x = _u_;\
  c[0    ][(unsigned char) _x     ]+=_c_;\
  c[1    ][(unsigned char)(_x>> 8)]+=_c_;\
  c[2    ][(unsigned char)(_x>>16)]+=_c_;\
  c[3    ][                _x>>24 ]+=_c_;  _x=(_u_)>>=32;\
  c[0+_o_][(unsigned char) _x     ]+=_c_;\
  c[1+_o_][(unsigned char)(_x>> 8)]+=_c_;\
  c[2+_o_][(unsigned char)(_x>>16)]+=_c_;\
  c[3+_o_][                _x>>24 ]+=_c_;\
}
  #endif

#define UZ 8 // Load size 1x 64 bits = 8 bytes
#define I164(_i_,_o_) { uint64_t u1 = ctou64(ip+UZ+_i_*UZ*2+ 0); CU64(u0, _o_, 1);\
                                 u0 = ctou64(ip+UZ+_i_*UZ*2+ 8); CU64(u1, _o_, 1);\
}

#define N64 64
unsigned histcalc8(unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt) {
  #define IC 4
  cnt_t c[8][CSIZE] = {0}; 
  unsigned char *ip = in; 
  
  if(inlen >= UZ+N64) {
    uint64_t u0 = ctou64(ip);
    for(; ip <= in+inlen-(UZ+N64); ip += N64) { 
	  I164(0,IC); I164(1,IC); I164(2,IC); I164(3,IC);
																	PREFETCH(ip+512, 0); 
	}
  } 
  while(ip != in+inlen) c[0][*ip++]++; 
  HISTEND8(c, cnt);
  unsigned a = 256; while(a > 1 && !cnt[a-1]) a--; 
  return a;
}

#define UZ 16 // Load size 2x 64 bits = 2*8 bytes
#define CR64(u,v,_o_,_c_) if(likely(u!=v)) { CU64(u,_o_,1); CU64(v,_o_,1); } else if((u^(v<<8)) < (1<<8)) c[_c_][(unsigned char)u]+=UZ; else CU64(u, _o_,2)
#define I2R64(_i_,_o_) { uint64_t u1 = ctou64(ip+UZ+_i_*UZ*2+ 0), v1 = ctou64(ip+UZ+_i_*UZ*2+ 8); CR64(u0,v0,_o_,_i_);\
                                  u0 = ctou64(ip+UZ+_i_*UZ*2+16); v0 = ctou64(ip+UZ+_i_*UZ*2+24); CR64(u1,v1,_o_,_i_);\
}

unsigned histrcalc8(unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt) {
  #define IC 4
  cnt_t c[8][CSIZE] = {0}; 
  unsigned char *ip = in; 
  
  if(inlen >= UZ+N64) {
    uint64_t u0 = ctou64(ip), v0 = ctou64(ip+8);
    for(; ip <= in+inlen-(UZ+N64); ip += N64) { 
	  I2R64(0,IC); I2R64(1,IC); 
																PREFETCH(ip+512, 0); 
	}
  }
  while(ip != in+inlen) c[0][*ip++]++; 
  HISTEND8(c, cnt);
  unsigned a = 256; while(a > 1 && !cnt[a-1]) a--; 
  return a;
}

void memrev(unsigned char a[], unsigned n) { 
  size_t i=0, j;
    #if defined(__AVX2__)
	__m256i cv = _mm256_set_epi8( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15	);
  for(j = i; j < n >> (1+5); ++j,i += 32) {
	__m256i lo = _mm256_shuffle_epi8(_mm256_loadu_si256((__m256i*)&a[i     ]),cv),
	        hi = _mm256_shuffle_epi8(_mm256_loadu_si256((__m256i*)&a[n-i-32]),cv);
	_mm256_storeu_si256((__m256i*)&a[i],          _mm256_permute2x128_si256(hi,hi,1));
	_mm256_storeu_si256((__m256i*)&a[n - i - 32], _mm256_permute2x128_si256(lo,lo,1));
  }
  //for( ; i < (n >> 1); ++i ) { unsigned char t = a[i]; a[i] = a[n - i - 1]; a[n - i - 1] = t; }
    #elif defined(__SSSE3__)
  __m128i cv = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  for(j = i; j < n >>(1+4); ++j, i += 16) {
	__m128i hi = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)&a[n-i-16]), cv),
	        lo = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)&a[i     ]), cv);
	_mm_storeu_si128((__m128i*)&a[i         ], hi);
	_mm_storeu_si128((__m128i*)&a[n - i - 16], lo);
  }
  //for( ; i < n >> 1; ++i )	{ unsigned char t = a[i]; a[i] = a[n - i - 1]; a[n - i - 1] = t; }
    #else 
  //unsigned i; for(--n,i=0;i < n;   ++i) { unsigned char t = a[i]; a[i] = a[n]; a[n--] = t; }
  i=0; 
    #endif
  for(;i < n/2; ++i) { unsigned char t = a[i]; a[i] = a[n-i-1]; a[n-i-1] = t; }
}

  #ifndef NCOMP
size_t bitenc(unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) { 
  unsigned cnt[256] = {0}, map[256] = {0}, a = histrcalc8(in, inlen, cnt), c = 0;
  unsigned char *ip = in, *op = out;

  *op++ = 0;
  for(int i = 0; i < 256; i++) if(cnt[i]) { map[i] = c++; *op++ = i; }
  if(c > 16) { *out = 0xff; memcpy(out+1, in, inlen); return inlen; }
  *out = c; 
       if(c <=  2) { for(; ip < in+(inlen&~(8-1)); ip+=8) *op++ = map[ip[7]]<<7 | map[ip[6]]<<6 | map[ip[5]]<<5 | map[ip[4]]<<4 | map[ip[3]]<<3 | map[ip[2]]<<2 | map[ip[1]]<<1 | map[ip[0]]; }
  else if(c <=  4) { for(; ip < in+(inlen&~(4-1)); ip+=4) *op++ = map[ip[3]]<<6 | map[ip[2]]<<4 | map[ip[1]]<<2 | map[ip[0]]; }
  else if(c <= 16) { for(; ip < in+(inlen&~(2-1)); ip+=2) *op++ = map[ip[1]]<<4 | map[ip[0]]; }
  while(ip < in+inlen) *op++ = *ip++;
  return op - out;
}
  #endif
  
  #ifndef NDECOMP
size_t bitdec(unsigned char *__restrict in, size_t outlen, unsigned char *__restrict out) {
  unsigned char *ip = in, *op = out, *p;
  if(*in == 0xff) { memcpy(out, in+1, outlen); return outlen+1; }
  unsigned c = *ip++; p = ip; ip += c; 
       if(c <= 2) { for(op = out; op < out+(outlen&~(8-1)); op+=8) { unsigned u = *ip++; op[0] = p[(u   )& 1]; op[1] = p[(u>>1)&1]; op[2] = p[(u>>2)&1]; op[3] = p[(u>>3)&1];
	                                                                                     op[4] = p[(u>>4)& 1]; op[5] = p[(u>>5)&1]; op[6] = p[(u>>6)&1]; op[7] = p[(u>>7)&1]; }}
  else if(c <= 4) { for(op = out; op < out+(outlen&~(4-1)); op+=4) { unsigned u = *ip++; op[0] = p[(u   )& 3]; op[1] = p[(u>>2)&3]; op[2] = p[(u>>4)&3]; op[3] = p[(u>>6)&3]; }}
  else if(c <=16) { for(op = out; op < out+(outlen&~(2-1)); op+=2) { unsigned u = *ip++; op[0] = p[(u   )&15]; op[1] = p[ u    &15]; op[2] = p[u>>4]; }}
  while(op < out+outlen) *op++ = *ip++;
  return ip - in;
}
  #endif 

  #ifndef _NDELTA
#define XORENC( _u_, _pu_, _usize_) ((_u_)^(_pu_))  // xor predictor
#define XORDEC( _u_, _pu_, _usize_) ((_u_)^(_pu_))
#define ZZAGENC(_u_, _pu_, _usize_)  T2(zigzagenc,_usize_)((_u_)-(_pu_)) //zigzag predictor
#define ZZAGDEC(_u_, _pu_, _usize_) (T2(zigzagdec,_usize_)(_u_)+(_pu_))
#define ECE while(ip != in+n) *out++ = *ip++
#define EDE ECE
//#define ELE for(;ip != in+n; ip++) l += (bsr8(ip[0])<<1)+1; return l;
//#define EL(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); l += (T2(bsr,_sz_)(T2(zigzagenc,_sz_)(z - u[_i_]))<<1)+1; u[_i_] = z; }
#define ELE for(;ip != in+n; ip++) l += bsr8(ip[0]); return l;

  #if 1
#define EL(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); l += T2(bsr,_sz_)(T2(zigzagenc,_sz_)(z - u[_i_])); u[_i_] = z; }
#define EC(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); T2(ctou,_sz_)(out+_i_*_sz_/8) = /*T2(zigzagenc,_sz_)*/(z - u[_i_]); u[_i_] = z; }
#define ED(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); T2(ctou,_sz_)(out+_i_*_sz_/8) = (u[_i_] += /*T2(zigzagdec,_sz_)*/(z)); }
  #elif 1 //best
#define EL(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); l += T2(bsr,_sz_)(T2(zigzagenc,_sz_)(z - u[_i_])); u[_i_] = z; }
#define EC(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); T2(ctou,_sz_)(out+_i_*_sz_/8) = T2(zigzagenc,_sz_)(z - u[_i_]); u[_i_] = z; }
#define ED(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); T2(ctou,_sz_)(out+_i_*_sz_/8) = (u[_i_] += T2(zigzagdec,_sz_)(z)); }
  #elif 0
#define EL(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); l += T2(bsr,_sz_)(T2(zigzagenc,_sz_)(z ^ u[_i_])); u[_i_] = z; }
#define EC(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); T2(ctou,_sz_)(out+_i_*_sz_/8) = z ^ u[_i_]; u[_i_] = z; }
#define ED(_i_,_sz_) { z = T2(ctou,_sz_)(ip+_i_*_sz_/8); T2(ctou,_sz_)(out+_i_*_sz_/8) = (u[_i_] ^= z); }
#endif
  
  //----------- 24 -----------------
  #ifndef NCOMP
//----------- 16 -----------------
unsigned  delta8l16(uint8_t *in, size_t n) { size_t l=0;   uint8_t  u[2]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~1); ip+= 2        ) { EL(0, 8);EL(1,8); } ELE; }
void      delta8e16(uint8_t *in, size_t n, uint8_t *out) { uint8_t  u[2]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~1); ip+= 2,out+= 2) { EC(0, 8);EC(1,8); } ECE; }
void      delta8d16(uint8_t *in, size_t n, uint8_t *out) { uint8_t  u[2]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~1); ip+= 2,out+= 2) { ED(0, 8);ED(1,8); } EDE; }

unsigned delta16l16(uint8_t *in, size_t n) { size_t l = 0; uint16_t u[1]={0},z; uint8_t *ip; for(ip = in; ip < in+(n&~1);  ip+= 2        )   EL(0,16); ELE; }
void     delta16e16(uint8_t *in, size_t n, uint8_t *out) { uint16_t u[1]={0},z; uint8_t *ip; for(ip = in; ip < in+(n&~1);  ip+= 2,out+= 2)   EC(0,16); ECE; }
void     delta16d16(uint8_t *in, size_t n, uint8_t *out) { uint16_t u[1]={0},z; uint8_t *ip; for(ip = in; ip < in+(n&~1);  ip+= 2,out+= 2)   ED(0,16); EDE; }

unsigned  delta8l24(uint8_t *in, size_t n) { size_t l = 0; uint8_t  u[3]={0},z; uint8_t *ip; for(ip = in; ip < in+(n-3);   ip+= 3        ) { EL(0, 8);EL(1,8);EL(2,8); } ELE; }
void      delta8e24(uint8_t *in, size_t n, uint8_t *out) { uint8_t  u[3]={0},z; uint8_t *ip; for(ip = in; ip < in+(n-3);   ip+= 3,out+= 3) { EC(0, 8);EC(1,8);EC(2,8); } ECE; }
//----------- 32 ----------------
unsigned  delta8l32(uint8_t *in, size_t n) { size_t l = 0; uint8_t  u[4]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4        ) { EL(0, 8);EL(1,8);EL(2,8);EL(3,8);} ELE; } //4D
void      delta8e32(uint8_t *in, size_t n, uint8_t *out) { uint8_t  u[4]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4,out+= 4) { EC(0, 8);EC(1,8);EC(2,8);EC(3,8);} ECE; }
void      delta8d32(uint8_t *in, size_t n, uint8_t *out) { uint8_t  u[4]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4,out+= 4) { ED(0, 8);ED(1,8);ED(2,8);ED(3,8);} EDE; }

unsigned delta16l32(uint8_t *in, size_t n) { size_t l = 0; uint16_t u[2]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4        ) { EL(0,16);EL(1,16); } ELE; }
void     delta16e32(uint8_t *in, size_t n, uint8_t *out) { uint16_t u[2]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4,out+= 4) { EC(0,16);EC(1,16); } ECE; }
void     delta16d32(uint8_t *in, size_t n, uint8_t *out) { uint16_t u[2]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4,out+= 4) { ED(0,16);ED(1,16); } EDE; }

unsigned delta32l32(uint8_t *in, size_t n) { size_t l = 0; uint32_t u[1]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4        )   EL(0,32); ELE; }
void     delta32e32(uint8_t *in, size_t n, uint8_t *out) { uint32_t u[1]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4,out+= 4)   EC(0,32); ECE; }
void     delta32d32(uint8_t *in, size_t n, uint8_t *out) { uint32_t u[1]={0},z; uint8_t *ip; for(ip = in; ip != in+(n&~3); ip+= 4,out+= 4)   ED(0,32); EDE; }

void xorenc16(uint8_t *in, size_t inlen, uint8_t *out) {
  uint16_t u0 = 0; uint8_t *ip;
  for(ip = in; ip < in+(inlen&~1); ip+=2,out+=2) {
    ctou16(out) = ctou16(ip) ^ u0; u0 = ctou16(ip);
  }
  while(ip < in+inlen) *out++ = *ip++; 
}

void xordec16(uint8_t *in, size_t inlen, uint8_t *out) {
  uint16_t u0 = 0; uint8_t *ip;
  for(ip = in; ip < in+(inlen&~1); ip += 2,out += 2) {
    ctou16(out) = u0 ^= ctou16(ip);
  }
  while(ip < in+inlen) *out++ = *ip++; 
}

void zzagenc16(uint8_t *in, size_t inlen, uint8_t *out) {
  int16_t s = 0,o; 
  uint8_t *ip;
  for(ip = in; ip < in+(inlen&~1); ip += 2, out += 2)
    o = ctou16(ip), ctou16(out) = zigzagenc16((short)ctou16(ip) - s), s = o;
  while(ip < in+inlen) *out++ = *ip++; 
}

void zzagdec16(uint8_t *in, size_t inlen, uint8_t *out) {
  int16_t s = 0; uint8_t *ip;
  for(ip = in; ip < in+(inlen&~1); ip += 2,out += 2) {
    uint16_t x = ctou16(ip);  
    ctou16(out) = (s += zigzagdec16(x));
  }
  while(ip < in+inlen) *out++ = *ip++; 
}

void nbenc16(uint8_t *in, size_t inlen, uint8_t *out) {
  uint16_t u0 = 0,x; uint8_t *ip;
  for(ip = in; ip < in+(inlen&~1); ip+=2,out+=2) {
    x = (short)ctou16(ip)-(short)u0;
    ctou16(out) = nb_enc16(x); u0 = ctou16(ip);	
  }
  while(ip < in+inlen) *out++ = *ip++; 
}

void nbdec16(uint8_t *in, size_t inlen, uint8_t *out) {
  uint16_t u0 = 0,x; uint8_t *ip;
  for(ip = in; ip < in+(inlen&~1); ip += 2,out += 2) {
    x = ctou16(ip);  
    ctou16(out) = (u0 += nb_dec16(x));
  }
  while(ip < in+inlen) *out++ = *ip++; 
}

    #endif
    #ifndef NDECOMP  
void      delta8d24(uint8_t *in, size_t n, uint8_t *out) { uint8_t  u[3]={0},z; uint8_t *ip; for(ip = in; ip < in+(n-3);   ip+= 3,out+= 3) { ED(0, 8);ED(1,8);ED(2,8); } EDE; }
    #endif
  #endif

  #ifndef _NQUANT
extern int verbose;
#ifndef min
#define min(x,y) (((x)<(y)) ? (x) : (y))
#define max(x,y) (((x)>(y)) ? (x) : (y))
#endif

//------------------------ Floating point statistics ------------------------------------------------------------------
#define BR(b) ((b/8)*100.0)/(double)(n*esize)

#define CMPA(_a_,_b_,_t_)         ((*(_t_ *)(_a_) > *(_t_ *)(_b_)) - (*(_t_ *)(_a_) < *(_t_ *)(_b_)))
static int cmpua16(const void *a, const void *b) { return CMPA(a,b,uint16_t); }
static int cmpua32(const void *a, const void *b) { return CMPA(a,b,uint32_t); }
static int cmpua64(const void *a, const void *b) { return CMPA(a,b,uint64_t); }

void fpstat(unsigned char *in, size_t n, unsigned char *out, int s, unsigned char *_tmp) {												if(verbose>0) printf("\nFloating point statistics\n");
  double        imin  = DBL_MAX, imax  = DBL_MIN, isum  = 0,               //original data (input)             : minimum,maximum,sum
                eamin = DBL_MAX, eamax = DBL_MIN, easum = 0, easumsqr = 0, //absolute error                    : abs(input-output)
                ermin = DBL_MAX, ermax = DBL_MIN, ersum = 0, ersumsqr = 0, //relative error                    : abs(input-output)/abs(input)
                osum  = 0;                                                 //transformed lossy data (output)   : sum
  long long     xtb = 0, xlb = 0, zlb = 0, tb = 0, lb = 0, elb = 0, mtb = 0, itb = 0;
  size_t        idn = 0;
  unsigned char *ip, *op;
  unsigned      esize = s<0?-s:s, t = 0, uni = 0, zero=0;
  long long     mant = 0;
  int           expo = 0,e;
  if(_tmp || verbose > 4) {
    unsigned char *tmp = _tmp;
    if(!tmp) { tmp = malloc(n*esize);  if(!tmp) die("malloc failed\n"); }  memcpy(tmp, out, n*esize);
    switch(esize) {
      case 2: { uint16_t *p,*t = tmp; qsort(tmp, n, 2, cmpua16); for(uni=zero=0,p = t; p < t+n-1; p++) { if(p[0] != p[1]) uni++; if(!p[0]) zero++; } } break;
	  case 4: { uint32_t *p,*t = tmp; qsort(tmp, n, 4, cmpua32); for(uni=zero=0,p = t; p < t+n-1; p++) { if(p[0] != p[1]) uni++; if(!p[0]) zero++; } } break;
	  case 8: { uint64_t *p,*t = tmp; qsort(tmp, n, 8, cmpua64); for(uni=zero=0,p = t; p < t+n-1; p++) { if(p[0] != p[1]) uni++; if(!p[0]) zero++; } } break;
	  default: die("#fpstat");
    } 
    if(!_tmp) free(tmp);
  }
  for(ip = in, op = out; ip < in+n*esize; ip += esize, op += esize)
    switch(s) {
        #if defined(FLT16_BUILTIN)
      case -2: isum += ctof16(ip); osum += ctof16(op); break;
        #endif
      case -4: isum += ctof32(ip); osum += ctof32(op); break;
      case -8: isum += ctof64(ip); osum += ctof64(op); break;
      case  1: isum += ctou8( ip); osum += ctou8( op); break;
      case  2: isum += ctou16(ip); osum += ctou16(op); break;
      case  4: isum += ctou32(ip); osum += ctou32(op); break;
      case  8: isum += ctou64(ip); osum += ctou64(op); break;
    }
  double iavg = isum/n, oavg = osum/n, isumpavg = 0, osumpavg = 0, iosumpavg = 0; uint64_t xstart = 0, zstart = 0;
  #define EXPO16(u) ((u>>10 &  0x1f) - 15 )
  #define EXPO32(u) ((u>>23 &  0xff) - 0x7e )
  #define EXPO64(u) ((u>>52 & 0x7ff) - 0x3fe)
  #define MANT16(u) (u & 0x83ffu)                //SeeeeeMMMMMMMMMM
  #define MANT32(u) (u & 0x807fffffu)
  #define MANT64(u) (u & 0x800fffffffffffffull)
  #define U(s) T3(uint, s, _t) u = T2(ctou,s)(op), v = T2(ctou,s)(ip);\
    itb +=  v?T2(ctz,s)(v):s;           tb +=      u?T2(ctz,s)(u):s;       lb += u?T2(clz,s)(u):s;          AC(t<=s,"Fatal t=%d ", t); \
    xstart ^= u;                       xtb += xstart?T2(ctz,s)(xstart):s; xlb += xstart?T2(clz,s)(xstart):0; xstart = u;\
    zstart  = T2(zigzagenc,s)(u - zstart);                       zlb += zstart?T2(clz,s)(zstart):s; zstart = u

  for(ip = in, op = out; ip < in+n*esize; ip += esize, op += esize) { 
    double id, od;
	unsigned e;	uint64_t m;
    switch(s) {
        #if defined(FLT16_BUILTIN)
      case -2: { unsigned e; uint16_t m;id = ctof16(ip); od = ctof16(op); U(16); e = EXPO16(u); expo = clz16(zigzagenc16(e-expo))/*-(16-(16-MANTF16-1))*/; elb+=expo; expo = e;
                                                          m = MANT16(u); mant = ctz16(            m^mant)                     ;     mtb+=mant; mant = m;//ctz16(zigzagenc16(m-mant))
                                                         } break;
        #endif                                                          
      case -4: { unsigned e; uint32_t m;id = ctof32(ip); od = ctof32(op); U(32); e = EXPO32(u); expo = clz32(zigzagenc32(e-expo))/*-(32-(32-MANTF32-1))*/; elb+=expo; expo = e;
                                                          m = MANT32(u); mant = ctz32(            m^mant)                     ;     mtb+=mant; mant = m;//ctz32(zigzagenc32(m-mant))
                                                         } break;
      case -8: { unsigned e; uint64_t m;id = ctof64(ip); od = ctof64(op); U(64); e = EXPO64(u); expo = clz32(zigzagenc32(e-expo))/*-(32-(64-MANTF64-1))*/; elb+=expo; expo = e;
                                                          m = MANT64(u); mant = ctz64(            m^mant)                     ; mtb+=mant; mant = m;//ctz64(zigzagenc64(m-mant))
                                                         } break;
      case  1: { id = ctou8( ip); od = ctou8( op); U( 8);} break;
      case  2: { id = ctou16(ip); od = ctou16(op); U(16);} break;
      case  4: { id = ctou32(ip); od = ctou32(op); U(32);} break;
      case  8: { id = ctou64(ip); od = ctou64(op); U(64);} break;
    }

    imax = max(id, imax);
    imin = min(id, imin);

      double ea = fabs(id - od); eamax = max(eamax,ea);  eamin = min(eamin,ea);  easum += ea;  easumsqr += ea*ea;  // absolute error
    if(id) { idn++;
      double er = ea/fabs(id);   ermax = max(ermax,er);  ermin = min(ermin,er);  ersum += er;  ersumsqr += er*er;   // relative error
    }
    isumpavg  += (id - iavg)*(id - iavg);
    osumpavg  += (od - oavg)*(od - oavg);
    iosumpavg += (id - iavg)*(od - oavg);   //bits      += ctz64(ctou64(&od)) - ctz64(ctou64(&id));
  } 
  double fb = 0;
       if(s == -2) fb = (double)elb*100/((double)n*5);
  else if(s == -4) fb = (double)elb*100/((double)n*8);
  else if(s == -8) fb = (double)elb*100/((double)n*11);

  double mse = easumsqr/n, irange = imax - imin;
  if(verbose >= 2) printf("\n");
  //printf("Leading/Trailing bits [%.2f%%,%.2f%%=%.2f%%]. XOR[%.2f%%,%.2f%%=%.2f%%] Zigzag[%.2f%%]\n", BR(lb), BR(tb), BR(lb+tb), BR(xlb), BR(xtb), BR(xlb+xtb), BR(zlb)/*BR(elb), BR(mtb), BR(elb+mtb)*/ );
  if(verbose >= 2)         printf("Range: [min=%g / max=%g] = %g\n", imin, imax, irange);
  if(verbose >  3 || _tmp) printf("zeros=[%u,%.2f%%], Distinct=[%u=%.4f%%] ctz=%.1f%%\n", zero,(double)zero*100.0/(double)n, uni, (double)uni*100.0/(double)n, (double)((tb-itb)/8)*100.0/(double)(n*esize));
  //if(verbose > 2) printf("Min error: Absolute = %g, Relative = %g, pointwise relative(PWE) = %g\n", eamin,      eamin/irange,      eamax/irange, ermax);
  //if(verbose > 2) printf("Avg error: Absolute = %g, Relative = %g, pointwise relative(PWE) = %g\n", easum/idn, (easum/idn)/irange,          ersum/idn);
  if(verbose > 2) printf("Max error: Absolute = %g, Relative = %g, pointwise relative(PWE) = %g\n", eamax,      eamax/irange,               ermax); else if(verbose==2) printf("e=%g ", ermax);
  double psnr=20*log10(irange)-10*log10(mse); 
  if(verbose > 2) printf("Peak Signal-to-Noise Ratio: PSNR         = %.1f\n", psnr);            else if(verbose==2) printf("PSNR=%.0f ", psnr);
  if(verbose > 2) printf("Normalized Root Mean Square Error: NRMSE = %g\n", sqrt(mse)/irange);  else if(verbose==2) printf("NRMSE=%g ", sqrt(mse)/irange);
  double std1 = sqrt(isumpavg/n), std2 = sqrt(osumpavg/n), ee = iosumpavg/n, acEff = (iosumpavg/n)/sqrt(isumpavg/n)/sqrt(osumpavg/n);
  if(verbose > 2) printf("Pearson Correlation Coefficient          = %f\n",    (iosumpavg/n)/sqrt(isumpavg/n)/sqrt(osumpavg/n));
}

//----------- Quantization -----------------------------------
#define ROUND16(x) roundf(x)
#define ROUND32(x) roundf(x)
#define ROUND64(x) round(x)                                                                             //#include "include_/vlcbyte.h"

#define QUANTE(t_s, _x_, _fmin_, _delta_) T2(ROUND,t_s)(((_x_) - _fmin_)*_delta_)                       //T2(ROUND,t_s)((_x_) * 100)
#define _FPQUANTE( t_s, _op_, _x_, _fmin_, _delta_) *_op_++  = QUANTE(t_s, _x_, _fmin_, _delta_)        //#define _FPQUANTVE(t_s, _op_, _x_, _fmin_, _delta_) { uint16_t _u = QUANTE(t_s, _x_, _fmin_, _delta_); vsput20(_op_,_u); }

#define FPQUANTE(t_t, _in_, inlen, _op_, qmax, t_s, pfmin, pfmax, _zmin_, _fpquante_) {\
  t_t fmin = _in_[0], fmax = _in_[0], *_ip;\
  for(_ip = _in_; _ip < _in_+(inlen/(t_s/8)); _ip++)\
    if(*_ip > fmax) fmax = *_ip; else if(*_ip < fmin) fmin = *_ip;\
  *pfmin = fmin; *pfmax = fmax;\
  t_t _delta = (fmax - fmin <= _zmin_)?(t_t)0.0:qmax/(fmax - fmin);\
  for(_ip = _in_; _ip < _in_+(inlen/(t_s/8)); _ip++)\
    _fpquante_(t_s, _op_, _ip[0], fmin, _delta);\
}

#define FPQUANTE8(t_t, _in_, _inlen_, _out_, qmax, t_s, pfmin, pfmax, _zmin_, _fpquante_) {\
  t_t           fmin = *pfmin, fmax = *pfmax, *_ip;\
  unsigned char *_op = _out_, *_ep_ = _out_ + _inlen_, *_ep = _ep_;              /*unsigned cm = 0,cx = 0;*/\
  if(fmin == 0.0 && fmax == 0.0) {\
	fmax = _in_[0], fmin = _in_[0];\
    for(_ip = _in_; _ip < _in_ + (_inlen_/(t_s/8)); _ip++)\
      if(*_ip > fmax) fmax = *_ip; else if(*_ip < fmin) fmin = *_ip;\
    *pfmin = fmin; *pfmax = fmax;\
  } else qmax--;\
  t_t _delta = (fmax - fmin <= _zmin_)?(t_t)0.0:qmax/(fmax - fmin);\
  \
  for(_ip = _in_; _ip < _in_+(_inlen_/(t_s/8)); _ip++) {\
    t_t _f = _ip[0];\
    if(_f < fmin || _f > fmax) { *_op++ = qmax+1; _ep -= (t_s/8); T2(ctof,t_s)(_ep) = _f; /*_f > fmax?cx++:cm++;*/ } /*store outliers w/o compression at the buffer end*/\
    else _fpquante_(t_s, _op, _f, fmin, _delta); \
                                                                                if(_op+8 >= _ep) goto ovr;\
  }                                                                             /*if(verbose > 2) printf("qmax=%u outliers:%u+%u=%u ",qmax, cm, cx, cm+cx);*/\
  unsigned _l = _ep_ - _ep; 													if(_op+_l >= _ep_) goto ovr;\
  memcpy(_op, _ep, _l); _op += _l;\
  return _op - _out_;\
  ovr:                                                                          if(verbose>2) printf("overflow:%zu ", _inlen_); \
    memcpy(_out_, _in_, _inlen_); return _inlen_;\
}

#define QUANTD(_x_) fmin + (_x_) * fmax
#define _FPQUANTD(t_t, _ip_, _x_) _x_ = fmin + (t_t)(*_ip_++) * fmax            //#define _FPQUANTVD(_ip_, _x_) { unsigned _u; vsget20(_ip_,_u); _x_ = QUANTD(_u); }

#define FPQUANTD(t_t, _ip_, outlen, _out_, qmax, fmin, fmax, _fpquantd_) { t_t *_op;\
  fmax = (fmax - fmin) / qmax;\
  for(_op = _out_; _op < _out_+(outlen/sizeof(_out_[0])); _op++) _fpquantd_(t_t, _ip_, _op[0]);\
}

#define FPQUANTD8(t_t, _ip_, outlen, _out_, qmax, t_s, fmin, fmax, _fpquantd_) { \
  t_t *_op; unsigned char *_ep = _ip_ + inlen;\
  qmax--; fmax = (fmax - fmin) / qmax;\
  for(_op = _out_; _op < _out_+(outlen/(t_s/8)); _op++)\
    if(*_ip_ == qmax+1) { _ip_++; _ep -= t_s/8; *_op = T2(ctof,t_s)(_ep); }\
	else _fpquantd_(t_t, _ip_, _op[0]);\
}

  #if defined(FLT16_BUILTIN) 
size_t fpquant8e16( _Float16 *in, size_t inlen, uint8_t  *out, unsigned qmax, _Float16 *pfmin, _Float16 *pfmax, _Float16 zmin) { FPQUANTE8(_Float16, in, inlen, out, qmax, 16, pfmin, pfmax, zmin,_FPQUANTE); }
size_t fpquant16e16(_Float16 *in, size_t inlen, uint16_t *out, unsigned qmax, _Float16 *pfmin, _Float16 *pfmax, _Float16 zmin) { FPQUANTE( _Float16, in, inlen, out, qmax, 16, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }
//size_t fpquantv8e16(_Float16 *in, size_t inlen, uint8_t  *out, unsigned qmax, _Float16 *pfmin, _Float16 *pfmax, _Float16 zmin) { unsigned char *op = out; FPQUANTE(_Float16, in, inlen, op,  qmax, 16, pfmin, pfmax, zmin,_FPQUANTVE); return op - out; }
  #endif

size_t fpquant8e32(     float *in, size_t inlen, uint8_t  *out, unsigned qmax, float    *pfmin,    float *pfmax,   float zmin) { FPQUANTE8(  float, in, inlen, out, qmax, 32, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }
size_t fpquant16e32(    float *in, size_t inlen, uint16_t *out, unsigned qmax, float    *pfmin,    float *pfmax,   float zmin) { FPQUANTE(   float, in, inlen, out, qmax, 32, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }
size_t fpquant32e32(    float *in, size_t inlen, uint32_t *out, unsigned qmax, float    *pfmin,    float *pfmax,   float zmin) { FPQUANTE(   float, in, inlen, out, qmax, 32, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }

size_t fpquant8e64(    double *in, size_t inlen, uint8_t  *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin) { FPQUANTE8( double, in, inlen, out, qmax, 64, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }
size_t fpquant16e64(   double *in, size_t inlen, uint16_t *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin) { FPQUANTE(  double, in, inlen, out, qmax, 64, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }
size_t fpquant32e64(   double *in, size_t inlen, uint32_t *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin) { FPQUANTE(  double, in, inlen, out, qmax, 64, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }
size_t fpquant64e64(   double *in, size_t inlen, uint64_t *out, unsigned qmax, double   *pfmin,   double *pfmax,  double zmin) { FPQUANTE(  double, in, inlen, out, qmax, 64, pfmin, pfmax, zmin,_FPQUANTE);  return inlen; }

    #if defined(FLT16_BUILTIN) 
size_t fpquant8d16(  uint8_t  *in, size_t outlen, _Float16 *out, unsigned qmax, _Float16   fmin, _Float16   fmax, size_t inlen) { FPQUANTD8(_Float16, in, outlen, out, qmax, 16, fmin, fmax, _FPQUANTD); return outlen;}
size_t fpquant16d16( uint16_t *in, size_t outlen, _Float16 *out, unsigned qmax, _Float16   fmin, _Float16   fmax) { FPQUANTD(_Float16, in, outlen, out, qmax,  fmin, fmax, _FPQUANTD); return outlen;}
//size_t fpquantv8d16( uint8_t  *in, size_t outlen, _Float16 *out, unsigned qmax, _Float16   fmin, _Float16   fmax) { unsigned char *ip = in; FPQUANTD(_Float16, in, outlen, out, qmax, fmin, fmax, _FPQUANTVD); return ip - in; }
    #endif

size_t fpquant8d32(  uint8_t  *in, size_t outlen, float    *out, unsigned qmax, float      fmin,    float   fmax, size_t inlen) { FPQUANTD8(  float, in, outlen, out, qmax, 32, fmin,  fmax, _FPQUANTD); return outlen;}
size_t fpquant16d32( uint16_t *in, size_t outlen, float    *out, unsigned qmax, float      fmin,    float   fmax) { FPQUANTD(   float, in, outlen, out, qmax,  fmin,  fmax, _FPQUANTD); return outlen;}
size_t fpquant32d32( uint32_t *in, size_t outlen, float    *out, unsigned qmax, float      fmin,    float   fmax) { FPQUANTD(   float, in, outlen, out, qmax,  fmin,  fmax, _FPQUANTD); return outlen;}

size_t fpquant8d64(  uint8_t  *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax, size_t inlen) { FPQUANTD8( double, in, outlen, out, qmax, 64, fmin,  fmax, _FPQUANTD); return outlen;}
size_t fpquant16d64( uint16_t *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax) { FPQUANTD(  double, in, outlen, out, qmax,  fmin,  fmax, _FPQUANTD); return outlen;}
size_t fpquant32d64( uint32_t *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax) { FPQUANTD(  double, in, outlen, out, qmax,  fmin,  fmax, _FPQUANTD); return outlen;}
size_t fpquant64d64( uint64_t *in, size_t outlen, double   *out, unsigned qmax, double     fmin,   double   fmax) { FPQUANTD(  double, in, outlen, out, qmax,  fmin,  fmax, _FPQUANTD); return outlen;}

//----------- Lossy floating point conversion: pad the trailing mantissa bits with zero bits according to the relative error e (ex. 0.00001)  ----------
  #if defined(FLT16_BUILTIN) 
// https://clang.llvm.org/docs/LanguageExtensions.html#half-precision-floating-point
_Float16 _fprazor16(_Float16 d, float e, int lg2e) {
  uint16_t du = ctou16(&d), sign, u;
  int      b  = (du>>10 & 0x1f) - 15; // exponent=[5 bits,bias=15], mantissa=10 bits SeeeeeMMMMMMMMMM
  _Float16 ed;
  if ((b = 12 - b - lg2e) <= 0) 
	return d;
  b     = b > 10?10:b;
  sign  = du & (1<<15); 
  du   &= 0x7fff;       
  for(d = ctof16(&du), ed = e * d;;) {
    u = du & (~((1u<<(--b))-1)); if(d - ctof16(&u) <= ed) break;
    u = du & (~((1u<<(--b))-1)); if(d - ctof16(&u) <= ed) break;
  }
  u |= sign;
  return ctof16(&u);
}

void fprazor16(_Float16 *in, unsigned n, _Float16 *out, float e) { 
  int lg2e = -log(e)/log(2.0); _Float16 *ip; 
  
  for (ip = in; ip < in+n; ip++,out++)
    *out = _fprazor16(*ip, e, lg2e); 
}
  #endif

float _fprazor32(float d, float e, int lg2e) {
  uint32_t du = ctou32(&d), sign, u;
  int      b  = (du>>23 & 0xff) - 0x7e;
  float    ed;
 
  if((b = 25 - b - lg2e) <= 0)
    return d;                                         AS(!isnan(d), "_fprazor32: isnan");
  b    = b > 23?23:b;
  sign = du & (1<<31);
  du  &= 0x7fffffffu;
  
  for(d = ctof32(&du), ed = e * d;;) {
    u = du & (~((1u<<(--b))-1)); if(d - ctof32(&u) <= ed) break;
    u = du & (~((1u<<(--b))-1)); if(d - ctof32(&u) <= ed) break;
    u = du & (~((1u<<(--b))-1)); if(d - ctof32(&u) <= ed) break;
  }
  u |= sign;
  return ctof32(&u);
}

void fprazor32(float *in, unsigned n, float *out, float e) { 
  int   lg2e = -log(e)/log(2.0); 
  float *ip; 
  for(ip = in; ip < in+n; ip++,out++) 
	*out = _fprazor32(*ip, e, lg2e); 
}

double _fprazor64(double d, double e, int lg2e) { //if(isnan(d)) return d;
  uint64_t du = ctou64(&d), sign, u;
  int      b  = (du>>52 & 0x7ff) - 0x3fe;
  double   ed;
  
  if((b = 54 - b - lg2e) <= 0)
    return d;
  b     = b > 52?52:b;
  sign  = du & (1ull<<63); 
  du   &= 0x7fffffffffffffffull;

  for(d = ctof64(&du), ed = e * d;;) {
    u = du & (~((1ull<<(--b))-1)); if(d - ctof64(&u) <= ed) break;
    u = du & (~((1ull<<(--b))-1)); if(d - ctof64(&u) <= ed) break;
  }
  u |= sign; 
  return ctof64(&u);
}

void fprazor64(double *in, unsigned n, double *out, double e) { 
  int    lg2e = -log(e)/log(2.0); 
  double *ip; 
  
  for(ip = in; ip < in+n; ip++,out++) 
	*out = _fprazor64(*ip, e, lg2e); 
}

  #endif  
  
#ifndef _NCPUISA
static unsigned _cpuisa;
//--------------------- CPU detection -------------------------------------------
  #if defined(__i386__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
    #if (_MSC_VER >=1300) && !defined(__clang__)
#include <intrin.h>
    #elif defined (__INTEL_COMPILER)
#include <x86intrin.h>
    #endif

static inline void cpuid(int reg[4], int id) {
    #if defined (_MSC_VER) && !defined(__clang__) //|| defined (__INTEL_COMPILER)
  __cpuidex(reg, id, 0);
    #elif defined(__i386__) || defined(__x86_64__)
  __asm("cpuid" : "=a"(reg[0]),"=b"(reg[1]),"=c"(reg[2]),"=d"(reg[3]) : "a"(id),"c"(0) : );//  __cpuid(0, reg[0], reg[1], reg[2], reg[3]);
    #endif
}

static inline uint64_t xgetbv (int ctr) {
    #if(defined _MSC_VER && (_MSC_FULL_VER >= 160040219) && !defined(__clang__) || defined __INTEL_COMPILER)
  return _xgetbv(ctr);
    #elif defined(__i386__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
  unsigned a, d;
  __asm("xgetbv" : "=a"(a),"=d"(d) : "c"(ctr) : );
  return (uint64_t)d << 32 | a;
    #else
  unsigned a = 0, d = 0;
  return (uint64_t)d << 32 | a;
    #endif
}
  #endif

#define AVX512F     0x001
#define AVX512DQ    0x002
#define AVX512IFMA  0x004
#define AVX512PF    0x008
#define AVX512ER    0x010
#define AVX512CD    0x020
#define AVX512BW    0x040
#define AVX512VL    0x080
#define AVX512VNNI  0x100
#define AVX512VBMI  0x200
#define AVX512VBMI2 0x400

#define IS_SSE       0x10
#define IS_SSE2      0x20
#define IS_SSE3      0x30
#define IS_SSSE3     0x32
#define IS_POWER9    0x34 // powerpc
#define IS_NEON      0x38 // arm neon
#define IS_SSE41     0x40
#define IS_SSE41x    0x41 //+popcount
#define IS_SSE42     0x42
#define IS_AVX       0x50
#define IS_AVX2      0x60
#define IS_AVX512    0x800

unsigned cpuisa(void) {
  int c[4] = {0};
  if(_cpuisa) return _cpuisa;
  _cpuisa++;
    #if defined(__i386__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
  cpuid(c, 0);
  if(c[0]) {
    cpuid(c, 1);
    //family = ((c >> 8) & 0xf) + ((c >> 20) & 0xff)
    //model  = ((c >> 4) & 0xf) + ((c >> 12) & 0xf0)
    if( c[3] & (1 << 25)) {         _cpuisa  = IS_SSE;
    if( c[3] & (1 << 26)) {         _cpuisa  = IS_SSE2;
    if( c[2] & (1 <<  0)) {         _cpuisa  = IS_SSE3;
      //                            _cpuisa  = IS_SSE3SLOW; // Atom SSSE3 slow
    if( c[2] & (1 <<  9)) {         _cpuisa  = IS_SSSE3;
    if( c[2] & (1 << 19)) {         _cpuisa  = IS_SSE41;
    if( c[2] & (1 << 23)) {         _cpuisa  = IS_SSE41x; // +popcount
    if( c[2] & (1 << 20)) {         _cpuisa  = IS_SSE42;  // SSE4.2
    if((c[2] & (1 << 28)) &&
       (c[2] & (1 << 27)) &&                           // OSXSAVE
       (c[2] & (1 << 26)) &&                           // XSAVE
       (xgetbv(0) & 6)==6) {        _cpuisa  = IS_AVX; // AVX
      if(c[2]& (1 <<  3))           _cpuisa |= 1;      // +FMA3
      if(c[2]& (1 << 16))           _cpuisa |= 2;      // +FMA4
      if(c[2]& (1 << 25))           _cpuisa |= 4;      // +AES
      cpuid(c, 7);
      if(c[1] & (1 << 5)) {         _cpuisa = IS_AVX2;
        if(c[1] & (1 << 16)) {
          cpuid(c, 0xd);
          if((c[0] & 0x60)==0x60) { _cpuisa = IS_AVX512;
            cpuid(c, 7);
            if(c[1] & (1<<16))      _cpuisa |= AVX512F;
            if(c[1] & (1<<17))      _cpuisa |= AVX512DQ;
            if(c[1] & (1<<21))      _cpuisa |= AVX512IFMA;
            if(c[1] & (1<<26))      _cpuisa |= AVX512PF;
            if(c[1] & (1<<27))      _cpuisa |= AVX512ER;
            if(c[1] & (1<<28))      _cpuisa |= AVX512CD;
            if(c[1] & (1<<30))      _cpuisa |= AVX512BW;
            if(c[1] & (1u<<31))     _cpuisa |= AVX512VL;
            if(c[2] & (1<< 1))      _cpuisa |= AVX512VBMI;
            if(c[2] & (1<<11))      _cpuisa |= AVX512VNNI;
            if(c[2] & (1<< 6))      _cpuisa |= AVX512VBMI2;
      }}}
    }}}}}}}}}
    #elif defined(__powerpc64__)

  _cpuisa = IS_POWER9; // power9
    #elif defined(__ARM_NEON)
  _cpuisa = IS_NEON; // ARM_NEON
    #endif
  return _cpuisa;
}

unsigned cpuini(unsigned cpuisa) { if(cpuisa) _cpuisa = cpuisa; return _cpuisa; }

char *cpustr(unsigned cpuisa) {
  if(!cpuisa) cpuisa = _cpuisa;
    #if defined(__i386__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
  if(cpuisa >= IS_AVX512) {
    if(cpuisa & AVX512VBMI2) return "avx512vbmi2";
    if(cpuisa & AVX512VBMI)  return "avx512vbmi";
    if(cpuisa & AVX512VNNI)  return "avx512vnni";
    if(cpuisa & AVX512VL)    return "avx512vl";
    if(cpuisa & AVX512BW)    return "avx512bw";
    if(cpuisa & AVX512CD)    return "avx512cd";
    if(cpuisa & AVX512ER)    return "avx512er";
    if(cpuisa & AVX512PF)    return "avx512pf";
    if(cpuisa & AVX512IFMA)  return "avx512ifma";
    if(cpuisa & AVX512DQ)    return "avx512dq";
    if(cpuisa & AVX512F)     return "avx512f";
    return "avx512";
  }
  else if(cpuisa >= IS_AVX2)    return "avx2";
  else if(cpuisa >= IS_AVX)
    switch(cpuisa&0xf) {
      case 1: return "avx+fma3";
      case 2: return "avx+fma4";
      case 4: return "avx+aes";
      case 5: return "avx+fma3+aes";
      default:return "avx";
    }
  else if(cpuisa >= IS_SSE42)   return "sse4.2";
  else if(cpuisa >= IS_SSE41x)  return "sse4.1+popcnt";
  else if(cpuisa >= IS_SSE41)   return "sse4.1";
  else if(cpuisa >= IS_SSSE3)   return "ssse3";
  else if(cpuisa >= IS_SSE3)    return "sse3";
  else if(cpuisa >= IS_SSE2)    return "sse2";
  else if(cpuisa >= IS_SSE)     return "sse";
    #elif defined(__powerpc64__)
  if(cpuisa >= IS_POWER9)       return "power9";
    #elif defined(__ARM_NEON)
  if(cpuisa >= IS_NEON)         return "arm_neon";
    #endif
  return "none";
}
#endif
