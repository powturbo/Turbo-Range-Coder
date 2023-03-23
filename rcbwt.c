// Turbo Range Coder bwt: templates include
#include "include_/conf.h"
#include "include_/turborc.h"
#include "include_/rcutil.h"
#include "include_/bec.h"

#include "rcutil_.h"
#include "mb_vint.h"  

//------------------------------------------- bwt ----------------------------------------------------------  
  #ifdef _BWTDIV      
#include "libdivsufsort/include/divsufsort.h"
#include "libdivsufsort/include/unbwt.h"
  #else
#include "libsais/src/libsais.h"
typedef int32_t saidx_t;	  
  #endif
#define LZPREV(a) a////
#define OUT       out //in //
static int bwtx, forcelzp;  
static unsigned calcmod(size_t len) { return 1<<__bsr32(len); }
#define SR 16

  #ifndef NCOMP
static unsigned lenmins[33] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  40,40,40,40, 40,40,40,40, 40,40,40,40, 96,128,144,144,144 }; //144,128,144

size_t rcbwtenc(unsigned char *in, size_t inlen, unsigned char *out, unsigned lev, unsigned thnum, unsigned _lenmin) { 
  size_t        iplen  = inlen;
  unsigned      lenmin = _lenmin & 0x3ff, xbwt16 = (_lenmin & BWT_BWT16)?0x80:0, verbose = _lenmin & BWT_VERBOSE; 
  unsigned char *op    = out, *bwt   = vmalloc(inlen+1024), *ip = in; if(!bwt) { op = out+inlen; goto e; }  // inlen + space for bwt indexes idxns
  if(lenmin==1) lenmin = lenmins[bsr32(inlen)];									
																				if(verbose) { printf("level=%u bits=%u ", lev, bsr32(inlen));fflush(stdout); } 
  if(lenmin) {  																if(verbose) { printf("lenmin=%u ", lenmin);fflush(stdout); } 
    ip = bwt;
	switch(lenmin) {
	  //case 2  : iplen = fastaenc(in, inlen, ip);         						if(verbose) { printf("GenTR %u->%u ", inlen, iplen); fflush(stdout); } break;
      default : iplen = utf8enc(in, inlen, ip, _lenmin); 						if(verbose) { if(iplen == inlen) printf("NoUTF8 "); else printf("UTF8:%u->%u ", inlen, iplen); fflush(stdout); } break;					// try utf8 preprocessing
	}
	if(lenmin < 15 || iplen != inlen && iplen != -1) 
	  lenmin = lenmin<15?32-lenmin:31;   				                        // lenmin = 31 for utf8enc success
	else {
      lenmin = ((lenmin>480?480:lenmin)+7)/8; 								    // lenmin 0-30, 31:utf8enc
      ip     = bwt;    	  														LZPREV(if(lev==9) { memcpy(out, in, inlen); memrev(out, inlen); } );
      iplen  = lzpenc(lev==9?OUT:in, inlen, ip, lenmin*8, lev > 8?1:0); 								
	  if(iplen == inlen || iplen+(inlen>>7)+256 > inlen && !forcelzp) { /*Not enough saving*/		if(verbose) { printf("NoLzp r=%.2f ", (double)iplen*100.0/inlen);fflush(stdout); }
        ip = in; iplen = inlen; lenmin = 0;		
      } else { 																	if(verbose) { printf("Lzp r=%.2f ",   (double)iplen*100.0/inlen);fflush(stdout); } 
																				LZPREV(if(lev==9) memrev(ip, iplen));
	  }
	}
  } 
  *op++ = lenmin|xbwt16; 									
  if(lenmin) ctou32(op) = iplen, op += 4; 
    #ifdef _BWTDIV
  *op++ = 0;
  saidx_t   *sa   = (saidx_t *)vmalloc((iplen+2)*sizeof(sa[0])); 
  if(!sa) { op = out+inlen; goto e; }
  *(saidx_t *)op  = divbwt(ip, bwt, sa, iplen); 
              op += sizeof(sa[0]);
    #else
  unsigned idxs[256], iplen_ = xbwt16?(iplen/2):iplen, 
           mod = calcmod(iplen_/SR), idxsn = (iplen_-1)/mod + 1;                //printf("bwt idxs=%d ", idxsn); //idxsn = (idxsn/SR)*SR; 
  *op++ = idxsn - 1; 
  saidx_t *sa = (saidx_t *)vmalloc((iplen_+2)*sizeof(sa[0])); 
  if(!sa) { op = out+inlen; goto e; }	                                        if(verbose) { printf("xbwt16=%u ", xbwt16>0);fflush(stdout); } 
	  #ifdef _LIBSAIS16	                                                                        
  if(xbwt16) { 																	if(verbose) { printf("-"); fflush(stdout); } 
    unsigned rc = libsais16_bwt_aux(ip, bwt, sa, iplen_, 0, 0, mod, idxs); 		if(verbose) { printf("+"); fflush(stdout); } 
    if(iplen & 1) bwt[iplen-1] = ip[iplen-1]; 
  }  else 
	  #endif    
  {
	libsais_bwt_aux(ip, bwt, sa, iplen,  0, 0, mod, idxs);        //libsais_bwt(ip, bwt, sa, iplen, fs);//if(ip == in) { memcpy(bwt, ip, iplen); ip = bwt; } memrev(ip, iplen);	ip[iplen] = 0;	
  }
  memcpy(op, idxs, idxsn*sizeof(idxs[0]));
  op   +=          idxsn*sizeof(idxs[0]);    
    #endif
  vfree(sa);
  switch(lev) {
    case  0: memcpy(op, bwt, iplen); op += iplen; vfree(bwt); if(op-out == inlen) op++; return op - out; break;
	case  2: op += xbwt16?becenc16(bwt, iplen, op):becenc8(bwt, iplen, op); break;
	case  3: op += xbwt16?rcrlesenc16(  bwt, iplen, op):     rcrlesenc(bwt, iplen, op);        break;
 	case  4: op += xbwt16?rcrlessenc16( bwt, iplen, op, 4,7):rcrlessenc( bwt, iplen, op, 4,7); break;
	case  5: op += xbwt16?rcrle1senc16( bwt, iplen, op):     rcrle1senc( bwt, iplen, op);      break;
	case  6: op += xbwt16?rcrle1ssenc16(bwt, iplen, op, 3,7):rcrle1ssenc(bwt, iplen, op, 3,7); break;
    case  7: op +=        rcqlfcsenc(   bwt, iplen, op);        break;
	case  9: op +=        rcmrrssenc(   bwt, iplen, op, 0, 0);  break; // prm1,prm2 in mbc.h fixed
    case  8: 
    default: op +=        rcqlfcssenc(  bwt, iplen, op, 4, 7);
  }																								OVERFLOW(in,inlen,out, op, goto e); 
  e: if(bwt) vfree(bwt);														
  return op - out;
}
  #endif

  #ifndef NDECOMP  
size_t rcbwtdec(unsigned char *in, size_t outlen, unsigned char *out, unsigned lev, unsigned thnum) {
  unsigned char *ip    = in;                     
  unsigned      lenmin = *ip++, xbwt16 = lenmin&0x80; lenmin &=0x1f;
  size_t        oplen  = outlen, rc;
  
  if(lenmin) oplen = ctou32(ip),ip += 4;    
  
    #ifdef _BWTDIV
  ip++;
  saidx_t       bwtidx = *(saidx_t *)ip; ip += sizeof(saidx_t);
    #else
  unsigned idxs[256];
  int oplen_ = xbwt16?oplen/2:oplen, mod = calcmod(oplen_/SR), idxsn = (oplen_-1)/mod + 1;  //idxsn = (idxsn/SR)*SR; 
  ip++; // idxsn
  memcpy(idxs, ip, idxsn*sizeof(idxs[0])); ip += idxsn*sizeof(idxs[0]);
    #endif
  unsigned char *_bwt = vmalloc(oplen), *op = out, *bwt = _bwt;  if(!_bwt) die("malloc failed\n"); 
    {      if(lenmin) { bwt = out; op = _bwt; } }  
  switch(lev) {
    case  0: memcpy(bwt,    ip, oplen+bwtx);      break;
	case  2: xbwt16?becdec16(ip, oplen+bwtx, bwt):becdec8(ip, oplen+bwtx, bwt); break;
	case  3: xbwt16?rcrlesdec16(  ip, oplen+bwtx, bwt):      rcrlesdec(  ip, oplen+bwtx, bwt); break;
	case  4: xbwt16?rcrlessdec16( ip, oplen+bwtx, bwt, 2, 6):rcrlessdec( ip, oplen+bwtx, bwt, 2, 6); break;
	case  5: xbwt16?rcrle1sdec16( ip, oplen+bwtx, bwt):      rcrle1sdec( ip, oplen+bwtx, bwt); break;
	case  6: xbwt16?rcrle1ssdec16(ip, oplen+bwtx, bwt, 2, 6):rcrle1ssdec(ip, oplen+bwtx, bwt, 2, 6); break;
	case  7:        rcqlfcsdec(   ip, oplen+bwtx, bwt); break;
	case  9:        rcmrrssdec(   ip, oplen+bwtx, bwt, 0, 0); break; 
	case  8:        
    default:        rcqlfcssdec(  ip, oplen+bwtx, bwt, 4, 7);
  }
  saidx_t *sa = (saidx_t *)vmalloc((oplen+2)*sizeof(sa[0])); if(!sa) { vfree(bwt); die("malloc failed\n"); }  
    #ifdef _BWTDIV
  rc = obwt_unbwt_biPSIv2(bwt, op, sa, oplen, bwtidx);
    #else
	  #ifdef _LIBSAIS16 
  if(xbwt16) { rc = libsais16_unbwt_aux(bwt, op, sa, oplen_, 0, mod, idxs); if(oplen & 1) op[oplen-1] = bwt[oplen-1]; }
  else
	  #endif
   rc = libsais_unbwt_aux(bwt, op, sa, oplen, 0, mod, idxs); 	//libsais_unbwt(bwt, op, sa, oplen, idxs[0]);  //#bwtinv(bwt, oplen, op, NULL, idxs, idxsn); memrev(op, oplen);  //op[256]=0; printf("%s ", op);
    #endif
  vfree(sa);

  if(lenmin) {
	switch(lenmin) {
	  case 31: utf8dec(op, outlen, out);  break;
	  //case 30: fastadec(op, outlen, out); break;
	  default: 																    LZPREV(if(lev==9) memrev(op, oplen)); 
	    lzpdec(op, outlen, out, lenmin*8, lev > 8?1:0);                         LZPREV(if(lev==9) memrev(out, outlen));
	}
  }
  vfree(_bwt);
  return rc;
}
  #endif
