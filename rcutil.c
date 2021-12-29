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
// TurboRC: Range Coder - misc. functions
#include <stdio.h>             
#include <string.h>
#include "conf.h"
#include "rcutil_.h"

#include "turborc.h"
#include "rcutil.h"

  #if defined(_WIN32)
#include <windows.h>
//-------------------------------- malloc ----------------------------------------
static SIZE_T largePageSize = 0, vinit_ = 0;

static void vinit() { if(vinit_) return; vinit_++;
  HANDLE hToken = 0;
  if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
    LUID luid;
    if(LookupPrivilegeValue(NULL, TEXT("SeLockMemoryPrivilege"), &luid)) {
      TOKEN_PRIVILEGES tp;
      tp.PrivilegeCount = 1;
      tp.Privileges[0].Luid = luid;
      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), 0, 0); //unsigned rc; if(rc = GetLastError()) printf("AdjustTokenPrivileges.rc=%d ", rc);
    }
    CloseHandle(hToken);
  }

  HMODULE hKernel;
  if(hKernel = GetModuleHandle(TEXT("kernel32.dll"))) {
    typedef SIZE_T (WINAPI * GetLargePageMinimumProcT)();

    GetLargePageMinimumProcT largePageMinimumProc = (GetLargePageMinimumProcT)GetProcAddress(hKernel, "GetLargePageMinimum");
    if (largePageMinimumProc != NULL) {
      largePageSize = largePageMinimumProc();
      if ((largePageSize & (largePageSize - 1)) != 0) largePageSize = 0;
    }
  }														  //printf("page=%d \n", g_LargePageSize);	
} 
  #endif

void *vmalloc(size_t size) {
    #if defined(_WIN32)
  vinit();
  if(largePageSize /*&& largePageSize <= (1 << 30)*/ && size >= (1 << 18)) {
    void *rc = VirtualAlloc(0, (size + largePageSize - 1) & (~(largePageSize - 1)), MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
    //if(!rc) printf("VAlloc failed rc=%d ", GetLastError()); else printf("LP=%d/%d ", largePageSize, size); 
	if(rc) return rc; 
  }
  return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
    #else
  return malloc(size);
    #endif
}

void vfree(void *address) {
    #if defined(_WIN32)
  VirtualFree(address, 0, MEM_RELEASE);
    #else
  free(address);
    #endif
}

//--------------------------- lzp preprocessor (lenmin >= 32) ------------------------------------------------------------------------
  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BSWAP32(a) a 
#define BSWAP64(a) a 
  #else
#define BSWAP32(a) bswap32(a)
#define BSWAP64(a) bswap64(a)
  #endif 
  
#ifndef NO_COMP
#define H_BITS      16 										// hash table size 
#define HASH32(_x_) (((_x_) * 123456791) >> (32-H_BITS))
#define emitmatch(_l_,_op_) { unsigned _l = _l_-lenmin+1; *_op_++ = 255; while(_l >= 254) { *_op_++ = 254;  _l -= 254; OVERFLOW(in,inlen,out,op,goto end); } *_op_++ = _l; }
#define emitch(_ch_,_op_)   { *_op_++ = _ch_; if(_ch_ == 255) *_op_++ = 0; OVERFLOW(in,inlen, out, op, goto end); }

size_t lzpenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned lenmin) {	
  unsigned      htab[1<<H_BITS] = {0},cl, cx;
  unsigned char *ip = in, *cp, *op = out;
  if(lenmin < 32) lenmin = 32;
  if(inlen  < lenmin) { memcpy(out, in, inlen); return inlen; }

  cx = ctou32(ip); ctou32(op) = cx; cx = BSWAP32(cx); op += 4; ip += 4;  	//first context					 
  while(ip < in+inlen-lenmin) { 
    unsigned h4 = HASH32(cx);
             cp = in + htab[h4];
       htab[h4] = ip - in;    
    if(ctou64(ip) == ctou64(cp) && ctou64(ip+8) == ctou64(cp+8) && ctou64(ip+16) == ctou64(cp+16) && ctou64(ip+24) == ctou64(cp+24)) { // match
      for(cl = 32;;) {
        if(ip+cl >= in+inlen-32) break;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
        if(ctou64(ip+cl) != ctou64(cp+cl)) break; cl+=8;
      }
      if(cl >= lenmin) {
        for(; ip+cl < in+inlen && ip[cl] == cp[cl]; cl++);
        emitmatch(cl, op);
        ip += cl;				
        cx = BSWAP32(ctou32(ip-4));		        
        continue;
      }
    }
    unsigned ch = *ip++; emitch(ch, op); cx = cx<<8 | ch; // literal
  }
  while(ip < in+inlen) { unsigned c = *ip++; emitch(c, op); }
  end:return op - out;													
}
#endif

size_t lzpdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lenmin) {
  unsigned      htab[1<< H_BITS] = {0};
  unsigned char *ip = in,*op = out;
  unsigned      cx;
  if(lenmin < 32) lenmin = 32;

  cx = ctou32(ip); ctou32(op) = cx; cx = BSWAP32(cx); op += 4; ip += 4;

  while(op < out+outlen) {
    unsigned       h4 = HASH32(cx), c;
    unsigned char *cp = out + htab[h4],*op_;
             htab[h4] = op - out;
    if((c = *ip++) == 255) {
      if(*ip) {
        c = 0; do c += *ip; while(*ip++ == 254);
        for(op_ = op+c+lenmin-1; op < op_; *op++ = *cp++);
        cx = BSWAP32(ctou32(op-4));
        continue;
      } else ip++, c = 255;
    }
    cx = cx << 8 | (*op++ = c);
  }
  return ip - in;
}

#ifndef NO_COMP
// --------------------------------- QLFC - Quantized Local Frequency Coding ------------------------------------------
// QLFC=MTF-Backward: number of different symbols until the next occurrence (number of symbols will be seen before the next one)
// MTF:  number of different symbols since the last occurence  (number of symbols were seen until the current one)
// References: https://ieeexplore.ieee.org/document/1402216
uint8_t *rcqlfc(uint8_t *in, size_t n, uint8_t *out, uint8_t *r2c) {
  unsigned char f[1<<8] = {0};
  uint8_t *ip, *op = out;
  int m;
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
	uint8_t *pb,*q; p = r2c; 
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
#ifndef NO_COMP
//------------- Integer VLC for symbol dictionary coding
#define VB_B2  4  							//6: max. 4276351=41407f   5: 2171071=2120bf   4:1118431=1110df  3:592111=908ef
#define VB_BA2 (255 - (1<<VB_B2))  

#define VB_OFS1 (VB_BA2 - (1<<VB_B2))
#define VB_OFS2 (VB_OFS1 + (1 << (8+VB_B2)))

#define vbput24(_op_, _x_) { \
  if(likely((_x_) < VB_OFS1)){ *_op_++ = (_x_);																	}\
  else if  ((_x_) < VB_OFS2) { ctou16(_op_) = bswap16((VB_OFS1<<8)+((_x_)-VB_OFS1));                _op_  += 2; }\
  else                       { *_op_++ = VB_BA2 + (((_x_) -= VB_OFS2) >> 16); ctou16(_op_) = (_x_); _op_  += 2; }\
}

#define vbget24(_ip_, _x_) do { _x_ = *_ip_++;\
       if(likely(_x_ < VB_OFS1));\
  else if(likely(_x_ < VB_BA2))  { _x_ = ((_x_<<8) + (*_ip_)) + (VB_OFS1 - (VB_OFS1 <<  8)); _ip_++;} \
  else                           { _x_ = ctou16(_ip_) + ((_x_ - VB_BA2 ) << 16) + VB_OFS2; _ip_ += 2;}\
} while(0)

//-----------------------------------------------------------
#pragma pack(1) 
typedef struct { unsigned c, cnt; unsigned short id; } _PACKED sym_t;
#pragma pack() 

static ALWAYS_INLINE unsigned cid(unsigned c) {
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

#define SC(_x_) cid(_x_)

#define CMPA(_a_,_b_,_t_) ((*(_t_ *)(_a_) > *(_t_ *)(_b_)) - (*(_t_ *)(_a_) < *(_t_ *)(_b_)))
#define CMPD(_a_,_b_,_t_) ((*(_t_ *)(_a_) < *(_t_ *)(_b_)) - (*(_t_ *)(_a_) > *(_t_ *)(_b_)))

#define CMPSA(_a_,_b_, _t_, e) (((((_t_ *)_a_)->e) > (((_t_ *)_b_)->e)) - ((((_t_ *)_a_)->e) < (((_t_ *)_b_)->e)))
#define CMPSD(_a_,_b_, _t_, e) (((((_t_ *)_a_)->e) < (((_t_ *)_b_)->e)) - ((((_t_ *)_a_)->e) > (((_t_ *)_b_)->e)))

#define CMPSFA(_a_,_b_, _t_, _e_) (( SC(((_t_ *)_a_)->_e_) > SC(((_t_ *)_b_)->_e_)) - ( SC(((_t_ *)_a_)->_e_) < SC(((_t_ *)_b_)->_e_)))

static inline int cmpua(const void *a, const void *b) { return CMPA(a,b,uint32_t); }
static inline int cmpud(const void *a, const void *b) { return CMPD(a,b,uint32_t); }

static inline int cmpsna(const void *a, const void *b) { return CMPSA(a,b,sym_t,cnt); }
static inline int cmpsnd(const void *a, const void *b) { return CMPSD(a,b,sym_t,cnt); }
static inline int cmpsca(const void *a, const void *b) { return CMPSA(a,b,sym_t,c  ); }

static inline int cmpsiaca(const void *a, const void *b) { int c; if(c = CMPSA(a,b,sym_t,id ))       return c; return CMPSA(a,b,sym_t,c); }
static inline int cmpsiand(const void *a, const void *b) { int c; if(c = CMPSA(a,b,sym_t,id ))       return c; return CMPSD(a,b,sym_t,cnt); }
static inline int cmpsndca(const void *a, const void *b) { int c; if(c = CMPSD(a,b,sym_t,cnt))       return c; return CMPSA(a,b,sym_t,c  ); }
static inline int cmpscand(const void *a, const void *b) { int c; if(c = CMPSFA(a,b,sym_t,c)) return c; return CMPSD(a,b,sym_t,cnt); }

#define UTF8LENMASK(_c_, _m_, _l_) \
       if ((_c_ & 0xe0) == 0xc0) { _l_ = 2; _m_ = 0x1f; }\
  else if ((_c_ & 0xf0) == 0xe0) { _l_ = 3; _m_ = 0x0f; }\
  else if ((_c_ & 0xf8) == 0xf0) { _l_ = 4; _m_ = 0x07; }\
  else if ((_c_ & 0xfc) == 0xf8) { _l_ = 5; _m_ = 0x03; }\
  else if ((_c_ & 0xfe) == 0xfc) { _l_ = 6; _m_ = 0x01; }\
  else _l_ = 0; /*invalid*/

#define UTF8GET(_ip_, _m_, _l_, _c_) do {\
  _c_ &= (_m_);\
  for(int _ui = 1; _ui < (_l_); _ui++) {\
    if(((_ip_)[_ui] & 0xc0) != 0x80) { (_l_) = 0; break;}\
    _c_ = (_c_) << 6 | ((_ip_)[_ui] & 0x3f);\
  }\
} while(0)

#define UCGET(_ip_, _c_, _l_) {\
  if((_c_ = *_ip_) < 128) _ip_++;\
  else { \
    unsigned _mask;\
    UTF8LENMASK(_c_, _mask, _l_);\
	UTF8GET(_ip_, _mask, _l_, _c_); _ip_ += _l_;\
  }\
}

#define FPUSH(in,_n_, nmax, _u_, _l_, _hash_) do { in[_n_].c = _u_; in[_n_].id = _l_; in[_n_++].cnt = 1; _hash_[chash(_u_)] = _n_; } while(0);

#define HASH32(_x_) (((_x_) * 123456791))
#define HBITS 26
#define chash(_c_) (HASH32(_c_) >> (32-HBITS))
#define CGET( _a_, _hash_, _c_, _i_) { unsigned _h = chash(_c_); while(       _a_[_i_ = _hash_[_h]-1].c != _c_) _h = (_h+1)&((1<<HBITS)-1); }
#define CFIND(_a_, _hash_, _c_, _i_) { unsigned _h = chash(_c_); while((_i_=_hash_[_h]) && _a_[_i_-1].c != _c_) _h = (_h+1)&((1<<HBITS)-1); _i_--; }
#define REHASH( _a_, _n_, _hash_) { memset(_hash_, 0, (1<<HBITS)*sizeof(_hash_[0])); for(int _i = 0; _i < _n_; _i++)                 _hash_[chash(_a_[_i].c)] = _i+1; }
#define REHASHV(_a_, _n_, _hash_) { memset(_hash_, 0, (1<<HBITS)*sizeof(_hash_[0])); for(int _i = 0; _i < _n_; _i++) if(_a_[_i].cnt) _hash_[chash(_a_[_i].c)] = _i+1; }

static unsigned char *symsput(sym_t *f, unsigned fn, unsigned char *out, unsigned flag) {	
  unsigned char *op = out+1, *p; 
  out[0] = flag;

  ctou16(op) = fn; op += 2; p = op; op += 1+2+2;                    
  int i=0, v=0,u;
         while(i < fn) { if(f[i].c >     0xff) break;               *op++      = f[i++].c; }                                         *p = i; 
  v = 0; while(i < fn) { if(f[i].c >   0xffff) break;               ctou16(op) = bswap16(f[i++].c);              op += 2; } ctou16(p+1) = i; 
  v = 0; while(i < fn) { if(f[i].c > 0xffffff) break; u = f[i++].c; *op++      = u>>16; ctou16(op) = bswap16(u); op += 2; } ctou16(p+3) = i; 
  v = 0; while(i < fn) {                                            ctou32(op) = bswap32(f[i++].c);              op += 4; }
  return op;
}

unsigned utf8enc(unsigned char *in, size_t inlen, unsigned char *out, unsigned flag) { 
  unsigned char *ip, *op = out;
  sym_t    *f     = calloc(1<<17, sizeof(f[0])); 
  unsigned *fhash = calloc((1<<HBITS), sizeof(fhash[0])), fn = 0, nmax = 1<<17, xprep16 = flag & BWT_PREP16, itmax = (flag>>10) & 0xf, xsort = (flag>>14) & 0x3, cnt;
  
  for(ip = in; ip < in+inlen;) {												// build the symbol dictionary
    unsigned c, ci, l;											   	
    UCGET(ip, c, l); 
	if(!l) { op = out+inlen; goto e; } 											// invalid utf8
	CFIND(f, fhash, c, ci); 
	if(ci != -1) f[ci].cnt++; 
	else { 
	  FPUSH(f,fn,nmax, c, l, fhash);
	  if(fn > 0xffff) { op=out+inlen; goto e; }								    // max. number of symbols is 64k
	}
  }
  
  switch(xsort) {
    case 0: qsort(f, fn, sizeof(sym_t), cmpscand); break;						// sort by code group + count (bwt mode)
    case 1: qsort(f, fn, sizeof(sym_t), cmpsnd); break;  						// sort by count   
  }   
  
  unsigned cnt8 = 0; cnt = 0; 
  for(int i=0; i < fn; i++) { if(f[i].c <= 0xff) cnt8+=f[i].cnt; cnt+=f[i].cnt; } 
  double r = (double)cnt8*100.0 / (double)cnt; 								
  if(r > 50.0) return inlen;                    								// enough saving for converting to 16-bits?
  REHASH(f, fn, fhash);		

  op = out; 														
  if(itmax <= 1 || xprep16) op = symsput(f, fn, op, xprep16?1:0);				// output the dictionary 				
  unsigned hdlen = op - out; 
  if(hdlen & 1) *op++ = 0; 														// offset to data must be even
  
  if(itmax>1 || xprep16) { 														  
    if((op-out)+cnt*2 >= (inlen*255)/256-8) { op = out+inlen; goto e;}; 		// check overflow in case of 16 bits
	for(ip = in; ip < in+inlen;) { 
	  unsigned c,l,ci; 
	  UCGET(ip, c, l); 
	  CGET(f, fhash, c, ci); 
	  ctou16(op) = bswap16(ci); op += 2; 
	}
  } else  																	
    for(ip = in; ip < in+inlen;) { 
      unsigned c,l,ci; 
      UCGET(ip, c, l); 
	  CGET(f, fhash, c, ci); 
	  vbput24(op, ci); 															OVERFLOW(in,inlen,out, op, goto e);  
	}

  e:if(f) free(f); 
  if(fhash) free(fhash);
  return op - out;
}
#endif

//--------------------------- decode ---------------------
static ALWAYS_INLINE size_t utf8len(uint32_t cp) {
  if(cp <=     0x7f) return 1;
  if(cp <=    0x7FF) return 2;
  if(cp <=   0xFFFF) return 3;
  if(cp <= 0x1FFFFF) return 4;
  return 0;
}

#define ctoutf8(op, _c_) { unsigned c = _c_;\
  switch (utf8len(c)) {\
    case 0:	break;			/*invalid*/\
	case 1:	*op++ = c; break;\
	case 2:	ctou16(op) = 0x80c0 | (c & 0x3f)<<8 | c>>6; op+=2; break;\
	case 3:	ctou16(op) = 0x80e0 | ((c & 0xfc0) >> 6) << 8 | c >> 12; op+=2; *op++ = 0x80 | (c & 0x3f); break;\
	case 4: ctou32(op) = 0x808080f0u | (c & 0x3f)<<24 | (c & 0xfc0)<<10 | (c & 0x3f000) >> 4 | c >> 18; op+=4; break;\
  }\
}

unsigned symsget(unsigned *sym, unsigned char **_in, unsigned *flag) {	
  unsigned char *in = *_in, *ip = in; 
  unsigned m0,m1,m2,fn,i=0; *flag = *ip++; fn = ctou16(ip); ip+=2;
  m0 = *ip++; m1 = ctou16(ip); ip += 2; m2 = ctou16(ip); ip += 2;
  
  while(i < m0) sym[i++] =                                            *ip++;                       					
  while(i < m1) sym[i++] = bswap16(ctou16(ip)),                       ip+=2;
  while(i < m2) sym[i++] = (unsigned)ip[0]<<16|bswap16(ctou16(ip+1)), ip+=3; 		
  while(i < fn) sym[i++] = bswap32(ctou32(ip)),                       ip+=4;        			

  if((ip-in) &1 ) ip++;  // align to even offset
  *_in = ip;
  return fn;
}

unsigned utf8dec(unsigned char *in, size_t outlen, unsigned char *out) {
  unsigned char *op = out, *ip = in; 
  unsigned flag,sym[1<<16], fn = symsget(sym, &ip, &flag); 
  
  if(flag & 1) 
    for(; op < out+outlen; ip += 2) { 
      uint16_t u = bswap16(ctou16(ip)); 
	  ctoutf8(op, sym[u]); 						
    }	
  else 
	while(op < out+outlen) { 
      unsigned u; 
	  vbget24(ip, u); 
	  ctoutf8(op, sym[u]); 
	}
  return 0;
}

#define CSIZE (256 + 8)
typedef unsigned cnt_t;

#define HISTEND(_c_,_cn_,_cnt_) { int _i,_j;\
  memset(_cnt_, 0, 256*sizeof(_cnt_[0]));\
  for(_i=0; _i < 256; _i++)\
    for(_j=0; _j < _cn_;_j++) _cnt_[_i] += _c_[_j][_i];\
}	

#define HISTEND8(_c_,_cnt_) HISTEND(_c_,8,_cnt_)
#define HISTEND4(_c_,_cnt_) HISTEND(_c_,4,_cnt_)

//--------------------  64 bits ---------------------------------------------------
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
void histcalc8(unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt) {
  #define IC 4
  cnt_t c[8][CSIZE] = {0}, i; 
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
}

#define UZ 16 // Load size 2x 64 bits = 2*8 bytes
#define CR64(u,v,_o_,_c_) if(likely(u!=v)) { CU64(u,_o_,1); CU64(v,_o_,1); } else if((u^(v<<8)) < (1<<8)) c[_c_][(unsigned char)u]+=UZ; else CU64(u, _o_,2)
#define I2R64(_i_,_o_) { uint64_t u1 = ctou64(ip+UZ+_i_*UZ*2+ 0), v1 = ctou64(ip+UZ+_i_*UZ*2+ 8); CR64(u0,v0,_o_,_i_);\
                                  u0 = ctou64(ip+UZ+_i_*UZ*2+16); v0 = ctou64(ip+UZ+_i_*UZ*2+24); CR64(u1,v1,_o_,_i_);\
}

void histrcalc8(unsigned char *__restrict in, unsigned inlen, unsigned *__restrict cnt) {
  #define IC 4
  cnt_t c[8][CSIZE] = {0},i; 
  unsigned char *ip = in,*in_; 
  
  if(inlen >= UZ+N64) {
    uint64_t u0 = ctou64(ip), v0 = ctou64(ip+8);
    for(; ip <= in+inlen-(UZ+N64); ip += N64) { 
	  I2R64(0,IC); I2R64(1,IC); 
																PREFETCH(ip+512, 0); 
	}
  }
  while(ip != in+inlen) c[0][*ip++]++; 
  HISTEND8(c, cnt);
}
