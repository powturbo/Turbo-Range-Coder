/**
    Copyright (C) powturbo 2013-2020
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
// TurboRC: Range Coder 

#include <stdio.h>
#include <string.h>
  #ifdef _MSC_VER
#include "vs/getopt.h"
#include "conf.h"
  #else
#include <getopt.h> 
  #endif

#include "turborc.h"
#include "time_.h"
  #ifdef EXTRC
#include "xext_.c"
  #endif

#define MAGIC     0x00004352 
#define CODEC_MAX 32
static int verbose,lenmin=64;
enum { E_FOP=1, E_FCR, E_FRD, E_FWR, E_MEM, E_CORR, E_MAG, E_CODEC, E_FSAME };

static char *errs[] = {"", "open error", "create error", "read error", "write error", "malloc failed", "file corrupted", "no TurboRc file", "no codec", "input and output files are same" };

int memcheck(unsigned char *in, unsigned n, unsigned char *cpy) { 
  int i;
  for(i = 0; i < n; i++)
    if(in[i] != cpy[i]) { 
      printf("ERROR in[%d]=%x dec[%d]=%x\n", i, in[i], i, cpy[i]);
      return i+1; 
    }
  return 0;
}

void pr(unsigned l, unsigned n) { double r = (double)l*100.0/n; if(r>0.1) printf("%10u %6.2f%% ", l, r);else printf("%10u %7.3f%%", l, r); fflush(stdout); }

int xtrunc; 
unsigned bench(unsigned char *in, unsigned n, unsigned char *out, unsigned char *cpy, int id) {  
  unsigned l = 0, cdfnum = 0x100;
  cdf_t cdf[0x100+1]; 
  if(xtrunc) 
    for(int i = 0; i < n; i++) in[i] &= 0xf;
    #ifndef _MSC_VER
  memrcpy(cpy,in,n); 
    #endif
  if(id >= 30 && id <= 39) { unsigned i; for(cdfnum = i= 0; i < n; i++) if(in[i]>cdfnum) cdfnum=in[i]; cdfnum++; cdfini(in, n, cdf, 0x100); /*printf("cdfnum=%d ", cdfnum);*/ }
  switch(id) {
    case  1: TMBENCH("",l=rcsenc(     in, n, out),n); pr(l,n); TMBENCH2("11-rcs      o0  simple                  ",l==n?memcpy(cpy,out,n):rcsdec(     out, n, cpy), n); break;
    case  2: TMBENCH("",l=rcxsenc(    in, n, out),n); pr(l,n); TMBENCH2("12-rcxs     o8b simple sliding context  ",l==n?memcpy(cpy,out,n):rcxsdec(    out, n, cpy), n); break;
    case  3: TMBENCH("",l=rcssenc(    in, n, out),n); pr(l,n); TMBENCH2("13-rcss     o0  strong                  ",l==n?memcpy(cpy,out,n):rcssdec(    out, n, cpy), n); break;
    case  4: TMBENCH("",l=rcxssenc(   in, n, out),n); pr(l,n); TMBENCH2("14-rcxss    o8b strong sliding context  ",l==n?memcpy(cpy,out,n):rcxssdec(   out, n, cpy), n); break;
      #ifdef _NZ
    case  5: TMBENCH("",l=rcnzenc(    in, n, out),n); pr(l,n); TMBENCH2("15-rcnz     o0  nanozip                 ",l==n?memcpy(cpy,out,n):rcnzdec(    out, n, cpy), n); break;
    case  6: TMBENCH("",l=rcxnzenc(   in, n, out),n); pr(l,n); TMBENCH2("16-rcxnz    o8b nanozip sliding context ",l==n?memcpy(cpy,out,n):rcxnzdec(   out, n, cpy), n); break;
      #endif
      #ifdef _SH
    case  7: TMBENCH("",l=rcshenc(    in, n, out),n); pr(l,n); TMBENCH2("17-rcsh     o0  shelwien                ",l==n?memcpy(cpy,out,n):rcshdec(    out, n, cpy), n); break;
    case  8: TMBENCH("",l=rcxshenc(   in, n, out),n); pr(l,n); TMBENCH2("18-rcxsh    o8b shelwien sliding context",l==n?memcpy(cpy,out,n):rcxshdec(   out, n, cpy), n); break;
      #endif
    case 11: TMBENCH("",l=rcrlesenc(  in, n, out),n); pr(l,n); TMBENCH2("21-rcrles   RLE o0  simple              ",l==n?memcpy(cpy,out,n):rcrlesdec(  out, n, cpy), n); break;
    case 12: TMBENCH("",l=rcrlexsenc( in, n, out),n); pr(l,n); TMBENCH2("22-rcrlxes  RLE o8b simple              ",l==n?memcpy(cpy,out,n):rcrlexsdec( out, n, cpy), n); break;
    case 13: TMBENCH("",l=rcrlessenc( in, n, out),n); pr(l,n); TMBENCH2("23-rcrless  RLE o0  simple              ",l==n?memcpy(cpy,out,n):rcrlessdec( out, n, cpy), n); break;
    case 14: TMBENCH("",l=rcrlexssenc(in, n, out),n); pr(l,n); TMBENCH2("24-rcrlexss RLE o8b strong              ",l==n?memcpy(cpy,out,n):rcrlexssdec(out, n, cpy), n); break;
    case 15: TMBENCH("",l=rcqlfcsenc( in, n, out),n); pr(l,n); TMBENCH2("25-rcqlfcs  QLFC simple                 ",l==n?memcpy(cpy,out,n):rcqlfcsdec( out, n, cpy), n); break;
      #ifdef _BWT
    case 16: TMBENCH("",l=rcbwtsenc(  in, n, out,9,0,lenmin),n); pr(l,n); TMBENCH2("26-bwt                                  ",l==n?memcpy(cpy,out,n):rcbwtsdec(  out, n, cpy,9,0), n); break;
      #endif
    case 17: TMBENCH("",l=rcqlfcssenc(in, n, out),n); pr(l,n); TMBENCH2("27-rcqlfcss QLFC strong                 ",l==n?memcpy(cpy,out,n):rcqlfcssdec(out, n, cpy), n); break;
      #ifdef _NZ
    case 18: TMBENCH("",l=rcqlfcnzenc(in, n, out),n); pr(l,n); TMBENCH2("28-rcqlfcnz QLFC nanozip                ",l==n?memcpy(cpy,out,n):rcqlfcnzdec(out, n, cpy), n); break;
      #endif
    case 20: TMBENCH("",l=rcgsenc8(   in, n, out),n); pr(l,n); TMBENCH2("30-rcs   o0 gamma8                      ",l==n?memcpy(cpy,out,n):rcgsdec8(   out, n, cpy), n); break;
    case 21: TMBENCH("",l=rcgsenc16(  in, n, out),n); pr(l,n); TMBENCH2("31-rcs   o0 gamma16                     ",l==n?memcpy(cpy,out,n):rcgsdec16(  out, n, cpy), n); break;
    case 22: TMBENCH("",l=rcgsenc32(  in, n, out),n); pr(l,n); TMBENCH2("32-rcs   o0 gamma32                     ",l==n?memcpy(cpy,out,n):rcgsdec32(  out, n, cpy), n); break;
    case 23: TMBENCH("",l=rcgssenc8(  in, n, out),n); pr(l,n); TMBENCH2("33-rcss  o0 gamma8                      ",l==n?memcpy(cpy,out,n):rcgssdec8(  out, n, cpy), n); break;
    case 24: TMBENCH("",l=rcgssenc16( in, n, out),n); pr(l,n); TMBENCH2("34-rcss  o0 gamma16                     ",l==n?memcpy(cpy,out,n):rcgssdec16( out, n, cpy), n); break;
    case 25: TMBENCH("",l=rcgssenc32( in, n, out),n); pr(l,n); TMBENCH2("35-rcss  o0 gamma32                     ",l==n?memcpy(cpy,out,n):rcgssdec32( out, n, cpy), n); break;

    case 30: if(cdfnum<=16) { TMBENCH("",l=rc4senc(in, n, out),n);  pr(l,n); TMBENCH2("40-rc4s  bitwise adaptive o0 simple     ",l==n?memcpy(cpy,out,n):rc4sdec(  out, n, cpy), n); } break;           // Adaptive
    case 31: if(cdfnum<=16) { TMBENCH("",l=rc4csenc(in, n, out),n); pr(l,n); TMBENCH2("41-rc4cs bitwise static   o0 simple     ",l==n?memcpy(cpy,out,n):rc4csdec( out, n, cpy), n); } break;            // Static
    case 32: TMBENCH("",l=rccdfsenc( in, n, out, cdf, cdfnum),n);   pr(l,n); TMBENCH2("42-rccdfsb CDF static search            ",l==n?memcpy(cpy,out,n):(cdfnum<=16?rccdfsldec( out, n, cpy, cdf, cdfnum):rccdfsbdec( out, n, cpy, cdf, cdfnum)), n); break; // static
    case 33: TMBENCH("",l=rccdfsenc( in, n, out, cdf, cdfnum),n);   pr(l,n); TMBENCH2("43-rccdfsv CDF static division          ",l==n?memcpy(cpy,out,n):(cdfnum<=16?rccdfsvldec(out, n, cpy, cdf, cdfnum):rccdfsvbdec(out, n, cpy, cdf, cdfnum)), n); break;
    case 34: TMBENCH("",l=rccdfsmenc(in, n, out, cdf, cdfnum),n);   pr(l,n); TMBENCH2("44-rccdfst CDF static division lut      ",l==n?memcpy(cpy,out,n):(cdfnum<=16?rccdfsmldec(out, n, cpy, cdf, cdfnum):rccdfsmbdec(out, n, cpy, cdf, cdfnum)), n); break;
    case 35: TMBENCH("",l=rccdfs2enc(in, n, out, cdf, cdfnum),n);   pr(l,n); TMBENCH2("45-rccdfsb CDF static search interleaved",l==n?memcpy(cpy,out,n):(cdfnum<=16?rccdfsl2dec(out, n, cpy, cdf, cdfnum):rccdfsb2dec(out, n, cpy, cdf, cdfnum)), n); break; // static
    case 38: if(l=cdfnum<=16) { TMBENCH("",l=rccdf4enc( in, n, out),n); pr(l,n); TMBENCH2("48-rccdf CDF nibble adaptive            ",l==n?memcpy(cpy,out,n):rccdf4dec( out, n, cpy), n); }
             else {             TMBENCH("",l=rccdfenc(  in, n, out),n); pr(l,n); TMBENCH2("48-rccdf CDF byte   adaptive            ",l==n?memcpy(cpy,out,n):rccdfdec(  out, n, cpy), n); } break;
    case 39: if(l=cdfnum<=16) { TMBENCH("",l=rccdf4ienc(in, n, out),n); pr(l,n); TMBENCH2("49-rccdf CDF nibble adaptive interleaved",l==n?memcpy(cpy,out,n):rccdf4idec(out, n, cpy), n); }
             else {             TMBENCH("",l=rccdfienc( in, n, out),n); pr(l,n); TMBENCH2("49-rccdf CDF byte   adaptive interleaved",l==n?memcpy(cpy,out,n):rccdfidec( out, n, cpy), n); } break;
    case 50: TMBENCH("",l=lzpenc(  in, n, out,lenmin),n);               pr(l,n); TMBENCH2("50-lzp                                  ",l==n?memcpy(cpy,out,n):lzpdec(  out, n, cpy,lenmin), n); break;

    #define ID_LAST 29
      #ifdef EXTRC
    #include "xext.c"
      #endif
    #define ID_MEMCPY 19
    case ID_MEMCPY: TMBENCH( "", memcpy(out,in,n) ,n); pr(n,n); TMBENCH2("memcpy", memcpy(cpy,out,n), n);  l=n; break;
    default: return 0;
  }
  if(l) { memcheck(in,n,cpy); }
  return l;
}

static void usage(char *pgm) {
  fprintf(stderr, "\nTurboRC 20.02 Copyright (c) 2018-2020 Powturbo %s\n", __DATE__);
  fprintf(stderr, "\n Usage: %s <options> <infile1> <outfile>\n", pgm);
  fprintf(stderr, "<options>\n");
  fprintf(stderr, " -#      #: compression codec\n", CODEC_MAX+10);
  fprintf(stderr, "         Range Coder: 11/13=order0, 12/14=Order1 (simple/strong)\n");
  fprintf(stderr, "         RLE+rc     : 21/23=order0, 22/24=Order1 (simple/strong)\n");
  fprintf(stderr, "         QLFC+rc    : 25 (simple)\n");
    #ifdef _BWT
  fprintf(stderr, "         BWT+rc     : 26 (simple)\n");
    #endif
  fprintf(stderr, "         Gamma+rc   : 30/33=gamma8, 31/34=gamma16, 32/34=gamma32 (simple/strong)\n");
  fprintf(stderr, " -b#     #: block size in log2 (10..31 ,default %d)\n", 31);
  fprintf(stderr, " -d      decompress\n");
  fprintf(stderr, " -v      verbose\n");
  fprintf(stderr, " -o      write on standard output\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Ex.: turborc -11 -f file.jpg file.jpg.rc\n");
  fprintf(stderr, "     turborc -d file.jpg.rc file1.jpg\n");
  fprintf(stderr, "---------- Benchmark ---------------------\n");
  fprintf(stderr, " -k      benchmark\n");
  fprintf(stderr, " -e#      # = function ids separated by ',' or ranges '#-#' \n", ID_MEMCPY);
  fprintf(stderr, "          # = 0 Benchmark all functions\n");
  fprintf(stderr, " -i#/-j#  # = Minimum  de/compression iterations per run (default=auto)\n");
  fprintf(stderr, " -I#/-J#  # = Number of de/compression runs (default=3)\n");
  //fprintf(stderr, " -f      force overwrite of output file\n");
  fprintf(stderr, "Ex.:   turborc -e0 file\n");
  fprintf(stderr, "Ex.:   turborc -e11,12,20-22 file\n");
  exit(1);
} 

int main(int argc, char* argv[]) { //mbrtest();
  int   xstdout = 0, xstdin = 0, decomp = 0, codec = 0, blog = 30, dobench = 0, cmp = 1;
  int   c, digit_optind = 0;
  char *scmd = NULL;

  for(;;) {
    int this_option_optind = optind ? optind : 1, optind = 0;
    static struct option long_options[] = {
      { "help",     0, 0, 'h'},
      { 0,          0, 0, 0}
    }; 
    if((c = getopt_long(argc, argv, "1:2:3:4:5:6:7:8:9:b:cde:hI:J:kl:otv", long_options, &optind)) == -1) break;
    switch(c) {
      case 0:
        printf("Option %s", long_options[optind].name);
        if(optarg) printf (" with arg %s", optarg);  printf ("\n");
        break;
      case 'c': cmp++;                  break;
      case 'd': decomp++; break;
      case 't': xtrunc++; break;
      case 'o': xstdout++; break;
      case 'v': verbose++; break;
      case 'b': blog = atoi(optarg); if(blog<15) blog=15; if(blog > 31) blog=31; break;
      case 'e': scmd = optarg; dobench++; break;
      case 'I': if((tm_Rep  = atoi(optarg))<=0) tm_rep =tm_Rep=1; break;
      case 'J': if((tm_Rep2 = atoi(optarg))<=0) tm_rep =tm_Rep2=1; break;
      case 'l': lenmin = atoi(optarg); if(lenmin && lenmin < 16) lenmin = 16; if(lenmin > 256) lenmin = 256; break;
      case '1':case '2':case '3': case '4':case '5':case '6':case '7':case '8':case '9': {
        unsigned l = atoi(optarg); decomp = 0;
        if(l >= 0 && l <= 9) {
          codec = (c-'0')*10 + l - 10;                                  //printf("codec=%d\n", codec);fflush(stdout);
          if(codec>=0 && codec<=23)
            break; 
        }
      }
      case 'h':
      default: 
        usage(argv[0]);
        exit(0); 
    }
  }

  #define ERR(e) do { rc = e; /*printf("line=%d ", __LINE__);*/ goto err; } while(0)
  unsigned bsize = 1u << blog;
  int  rc = 0, inlen;
  char *in = NULL, *out = NULL, *cpy = NULL; 

  if(dobench) { //---------------------------------- Benchmark -----------------------------------------------------
    char _scmd[33];
    int  fno; 
    sprintf(_scmd, "11-%d", ID_MEMCPY+10);                          if(verbose>1) printf("BENCHMARK ARGS=%d,%d,%d\n", fno, optind, argc);
    printf("   E MB/s    size     ratio%%   D MB/s   function\n");  

    for(fno = optind; fno < argc; fno++) {
      uint64_t filen;
      int      n,i;    
      char     *finame = argv[fno];                                     
      FILE     *fi = fopen(finame, "rb");                           if(!fi ) { perror(finame); continue; }   if(verbose>1) printf("'%s'\n", finame);

      fseek(fi, 0, SEEK_END); 
      filen = ftell(fi); 
      fseek(fi, 0, SEEK_SET);
    
      unsigned b = (filen < bsize)?filen:bsize; 					if(verbose>1) printf("bsize=%u ", bsize);
      //n = filen; 

      in  = malloc(b+64);     if(!in)  ERR(E_MEM);
      out = malloc(b+64);     if(!out) ERR(E_MEM);
      cpy = malloc(b+64);     if(!cpy) ERR(E_MEM);

      while((n = fread(in, 1, b, fi)) > 0) {						if(verbose>1) printf("read=%u ", n);
        char *p = scmd && scmd[0] != '0'?scmd:_scmd;
        do { 
          int id = strtoul(p, &p, 10)-10,idx = id, i;
          if(id >= 0) {    
            while(isspace(*p)) p++; if(*p == '-') { if((idx = strtoul(p+1, &p, 10)-10) < id) idx = id; if(idx > ID_LAST) idx = ID_LAST; } //printf("ID=%d,%d ", id, idx);
            for(i = id; i <= idx; i++) 
              if(bench(in, n, out, cpy, i)) printf("\t%s\n", finame);            
          }        
        } while(*p++);
      }
      fclose(fi);        fi  = NULL;
      if(in)  free(in);  in  = NULL; 
      if(out) free(out); out = NULL; 
      if(cpy) free(cpy); cpy = NULL;      
    }
    exit(0);   
  }

  //---------------------------------- File Compression/Decompression -----------------------------------------------------
  if(argc <= optind) xstdin++;
  unsigned l;
  unsigned long long filen=0,folen=0;
  char               *finame = xstdin ?"stdin":argv[optind], *foname, _foname[1027];  if(verbose>1) printf("'%s'\n", finame);

  if(xstdout) foname = "stdout";
  else {
    foname = argv[optind+1];                                                                 
    if(!decomp) {
      int len = strlen(foname), xext = len>3 && !strncasecmp(&foname[len-3], ".rc", 3);
      if(!xext && len < sizeof(_foname)-3) {
        strcpy(_foname, foname);
        strcat(_foname, ".rc");
        foname = _foname;
      }
    }
  }
  if(!strcasecmp(finame,foname)) { printf("'%s','%s' \n", finame, foname); ERR(E_FSAME); }

  FILE *fi = xstdin ?stdin :fopen(finame, "rb"); if(!fi) { perror(finame); return 1; }
  FILE *fo = xstdout?stdout:fopen(foname, "wb"); if(!fo) { perror(finame); return 1; }
  if(!decomp) {
    l = blog<<24 | codec << 16 | MAGIC; 
    if(fwrite(&l, 1, 4, fo) != 4) ERR(E_FWR);             folen = 4;      // blocksize
    in  = malloc(bsize);
    out = malloc(bsize); if(!in || !out) ERR(E_MEM); 

    while((inlen = fread(in, 1, bsize, fi)) > 0) {        filen += inlen;
      switch(codec) {
        case  0: l = inlen; memcpy(out, in, inlen); break;
        case  1: l = rcsenc(     in, inlen, out); break;
        case  2: l = rcxsenc(    in, inlen, out); break;
        case  3: l = rcssenc(    in, inlen, out); break;
        case  4: l = rcxssenc(   in, inlen, out); break;
        case 11: l = rcrlesenc(  in, inlen, out); break;
        case 12: l = rcrlexsenc( in, inlen, out); break;
        case 13: l = rcrlessenc( in, inlen, out); break;
        case 14: l = rcrlexssenc(in, inlen, out); break;
        case 15: l = rcqlfcsenc( in, inlen, out); break;
          #ifdef _BWT
        case 16: l = rcbwtsenc(  in, inlen, out, 9, 0, lenmin); break;
          #endif
        case 20: l = rcgsenc8(   in, inlen, out); break;
        case 21: l = rcgsenc16(  in, inlen, out); break;
        case 22: l = rcgsenc32(  in, inlen, out); break;
        case 23: l = rcgssenc8(  in, inlen, out); break;
        case 24: l = rcgssenc16( in, inlen, out); break;
        case 25: l = rcgssenc32( in, inlen, out); break;
        default: ERR(E_CODEC); 
      }
      //if(l >= inlen) { l = inlen; memcpy(out, in, l); }                                                             
      l = l<<1 | (inlen < bsize?1:0);                               // last block?
      if(fwrite(&l, 1, 4, fo) != 4) ERR(E_FWR);  l>>=1;   folen += 4;
      if(inlen < bsize) 
       if(fwrite(&inlen, 1, 4, fo) != 4) ERR(E_FWR); else folen +=4;
      if(fwrite(out, 1, l, fo) != l) ERR(E_FWR);          folen += l;
                                    
    }                                                             if(verbose) printf("compress: '%s'  %d->%d\n", finame, filen, folen);                                                                    
  } else { // Decompress
    if(fread(&l, 1, 4, fi) != 4) ERR(E_FRD);                  filen = 4;
    if((l&0xffffu) != MAGIC) ERR(E_MAG);
    if((blog = l>>24) < 16 || blog>31) ERR(E_CORR);
    if((codec = (char)(l>>16)) > CODEC_MAX) ERR(E_CODEC);
    bsize = 1 << blog;
    in  = malloc(bsize); 
    out = malloc(bsize); if(!in || !out) ERR(E_MEM); 

    if(!(in = realloc(in, bsize))) ERR(E_MEM);  
    for(;;) {
      if(fread(&l, 1, 4, fi) != 4) break;                     filen += 4;   // compressed block length
      if((l>>1) > bsize) ERR(E_CORR);
      if((l&1))
        if(fread(&bsize, 1, 4, fi) != 4) ERR(rc = E_FRD);else filen += 4; // read last block length
      l >>= 1;                   
      if(fread(in, 1, l, fi) != l) ERR(E_FRD);                filen += l; // read block
      if(l == bsize) { memcpy(out,in, l); }
      else switch(codec) { 
        case  0: memcpy(out, in, bsize);      break;
        case  1: rcsdec(     in, bsize, out); break;
        case  2: rcxsdec(    in, bsize, out); break;
        case  3: rcssdec(    in, bsize, out); break;
        case  4: rcxssdec(   in, bsize, out); break;
        case 11: rcrlesdec(  in, bsize, out); break;
        case 12: rcrlexsdec( in, bsize, out); break;
        case 13: rcrlessdec( in, bsize, out); break;
        case 14: rcrlexssdec(in, bsize, out); break;
        case 15: rcqlfcsdec( in, inlen, out); break;
          #ifdef _BWT
        case 16: rcbwtsdec(  in, bsize, out, 9, 0); break;
          #endif
        case 20: rcgsdec8(   in, bsize, out); break;
        case 21: rcgsdec16(  in, bsize, out); break;
        case 22: rcgsdec32(  in, bsize, out); break;
        case 23: rcgssdec8(  in, bsize, out); break;
        case 24: rcgssdec16( in, bsize, out); break;
        case 25: rcgssdec32( in, bsize, out); break;
        default: ERR(E_CODEC); 
      }
      if(fwrite(out, 1, bsize, fo) != bsize) ERR(E_FWR);     folen += bsize;                                  
    }                                                        if(verbose) printf("decompress:'%s' %d->%d\n", foname, filen, folen);
  }                                                              
  if(fi) fclose(fi);
  if(fo) fclose(fo);
  err: if(rc) { fprintf(stderr,"%s\n", errs[rc]); fflush(stderr); }
  if(in) free(in);
  if(out) free(out);
  if(cpy) free(cpy);
  return rc;
}

