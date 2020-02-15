#include "../conf.h"
#include <stdlib.h>

  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BSWAP16(a) a 
  #else
#define BSWAP16(a) bswap16(a)
  #endif  

//------------------------------------- Fastet Histogram : https://github.com/powturbo/TurboHist -------------------------------------------
#define cnt_t unsigned
#define CSIZE (256 + 8)

#define CU32(u,_i_) {\
  c[_i_+0][(unsigned char) u     ]++;\
  c[_i_+1][(unsigned char)(u>> 8)]++;\
  c[_i_+2][(unsigned char)(u>>16)]++;\
  c[_i_+3][ 	           u>>24 ]++;\
}

#define OV 8
#define INC4_32(_i_) { { unsigned u = ux, v = vx; ux = ctou32(ip+_i_+OV+0); vx = ctou32(ip+_i_+OV+ 4); CU32(u,0); CU32(v,0); }\
                       { unsigned u = ux, v = vx; ux = ctou32(ip+_i_+OV+8); vx = ctou32(ip+_i_+OV+12); CU32(u,0); CU32(v,0); }\
}

static unsigned cntcalc32(const unsigned char *in, unsigned inlen, cnt_t *cnt) {
  cnt_t c[4][CSIZE] = {0},i; 

  unsigned char *ip = in; 
  if(inlen >= 64) {
    unsigned ux = ctou32(ip), vx = ctou32(ip+4);
    for(; ip != in+(inlen&~(64-1))-64; ip += 64) { INC4_32(0); INC4_32(16); INC4_32(32); INC4_32(48); __builtin_prefetch(ip+512, 0); }
  }
  while(ip != in+inlen) 
    c[0][*ip++]++; 

  for(i = 0; i < 256; i++)
    cnt[i] = c[0][i]+c[1][i]+c[2][i]+c[3][i];
  unsigned a = 256; while(a > 1 && !cnt[a-1]) a--; 
  return a;
}

// Reference: obwt_unbwt_biPSIv2 optimized by powturbo (12% faster)
// openbwt: https://encode.su/threads/104-libBWT?p=22903&viewfull=1#post22903

// bi-gram based PSI array + binary search algorithm v2 (t=NUM_TOPBITS). space: 4(n+1)+4*257+4*256*256+4*(2**t) bytes. time: O(n log 256)?
#define NUM_TOPBITS 17

int obwt_unbwt_biPSIv2(const unsigned char *__restrict T, unsigned char *__restrict U, unsigned *__restrict PSI, int n, int pidx) {					
  unsigned C[257]  ={0};
  unsigned D[1<<16]={0}, *LD;
  unsigned B2S[1U << NUM_TOPBITS];
  unsigned u;
  int      i, t, v, w, x;
  int      c, d, e, mask, lastc;
  int      lowshift, highshift;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0) || (pidx < 0) ||
     (n < pidx) || ((0 < n) && (pidx == 0))) { return -1; }
  if(n <= 1) { if(T != U) { U[0] = T[0]; } return 0; }

  for(i = 0; 0 < (n >> i); ++i) { }
  highshift = (NUM_TOPBITS <= i) ? i - NUM_TOPBITS : 0;
  lowshift  = (16 < i) ? (32 - i) : 16;
  mask      = (1u << lowshift) - 1;

  // Compute the cumulative frequency of uni-grams   													
  cntcalc32(T, n, C);  								 
  for(c = 0, i = 1; c < 256; ++c) { t = C[c]; C[c] = i; i += t; }  C[c] = i;  //R[0] = 0; PREFIXSUM(R, 256);  //for(c=0; c < 257; c++) printf("%d,%d ", R[c], C[c]); exit(0);

  // Count the frequency of bi-grams 
  for(c = 0; c < 256; ++c) {
    LD = D + (c << 8);
    for(i = C[c]; i < C[c + 1]; ++i)
      if(i != pidx) {
        d = T[i - (pidx < i)];
        ++LD[d];
      }
  }

  // Compute the cumulative frequency of bi-grams
  lastc = T[0];
  for(c = 0, i = 1, w = 0; c < 256; ++c) {
    if(c == lastc) { i += 1; }
    LD = D + c;
    for(d = 0; d < 256; ++d) {
      t = LD[d << 8];
      if(0 < t) {
        e = (c << 8) | d;
        for(v = i >> highshift; v < w; ++v) {
          B2S[v] = ((unsigned short)B2S[v]) | (((unsigned )e) << 16);
        }
        for(w = ((i + t - 1) >> highshift) + 1; v < w; ++v) {
          B2S[v] = ((unsigned )e) | (((unsigned )e) << 16);
        }
      }
      LD[d << 8] = i; i += t;
    }
  }
  for(v = 0; v < w; ++v) {
    c = (B2S[v] >> 16) ^ ((unsigned short)B2S[v]);
    for(x = 0; c != 0; ++x, c >>= 1) { }
    B2S[v] = (unsigned)((((unsigned short)B2S[v]) & (0xffffU << x)) | (x << 16));
  }

  // Compute bi-PSI array 
  for(i = 0; i < n; ++i) {
    d = T[i];
    t = C[d]++;
    if(t != pidx) {
      c = T[t - (pidx < t)];
      t = D[(d << 8) | c]++;
      u = i + (pidx <= i);
      PSI[t] = (u << lowshift) | (((c << 8) | d) & mask);
    }
  }

  // Inverse BW-transform. 
  for(c = 0; c < 256; ++c) {
    for(d = 0; d < c; ++d) {
      t = D[(d << 8) | c];
      D[(d << 8) | c] = D[(c << 8) | d];
      D[(c << 8) | d] = t;
    }
  }
  
  for(i = 0, t = pidx; i < n-1; i += 2) {  
    unsigned c    = B2S[t >> highshift],
             half = ((1u << (c >> 16)) - 1)>>1;
             c    = (unsigned short)c;
    #define ST { c += (half + 1)&(-(D[c + half] <= t)); if(mask >= half) break; half >>= 1; }
    for(;;) { ST;ST;ST;ST;ST;ST;ST;ST;ST;ST;ST;ST;ST;ST; }  		//unsigned   c = (unsigned short)B2S[t >> highshift]; while(D[c] <= t) c++;
    ctou16(&U[i]) = BSWAP16(c | (PSI[t] & mask));
    t             = PSI[t] >> lowshift;					
  }
  if(i == n-1) { U[n - 1] = (unsigned char)lastc; }
  return 0;
}

